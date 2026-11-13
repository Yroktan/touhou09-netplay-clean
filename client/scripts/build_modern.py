USAGE='Standalone x86 game using Microsoft v142 tools.'
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT.parent
TOOLS = WORK/'toolchain'
BUILD = ROOT/'build/modern'

def main():
    parser=argparse.ArgumentParser(USAGE)
    parser.add_argument('--original',type=Path,default=ROOT/'orig/th09_150a')
    parser.add_argument('--dx8-headers',type=Path,default=TOOLS/'dx8-headers')
    args=parser.parse_args()
    compiler = next((TOOLS/'msvc').glob('VC/Tools/MSVC/*/bin/Hostx64/x86/cl.exe'), None)
    sdk_header = next((TOOLS/'msvc').glob('kits/10/Include/*/um/Windows.h'), None)
    if sdk_header is None:
        sdk_header = next((TOOLS/'msvc').glob('Windows Kits/10/Include/*/um/Windows.h'), None)
    if compiler is None or sdk_header is None:
        raise RuntimeError('Microsoft toolchain extraction is not finished')
    vc = compiler.parents[3]
    sdk_version = sdk_header.parents[1].name
    sdk = sdk_header.parents[3]
    includes = [args.dx8_headers.resolve(), ROOT/'src', vc/'include'] + [sdk/'Include'/sdk_version/d for d in ('ucrt','shared','um','winrt')]
    libs = [TOOLS/'dx8-lib',vc/'lib/x86',sdk/'Lib'/sdk_version/'ucrt/x86',sdk/'Lib'/sdk_version/'um/x86']
    env = dict(os.environ, INCLUDE=';'.join(map(str,includes)), LIB=';'.join(map(str,libs)))
    env['PATH'] = str(compiler.parent)+';'+env.get('PATH','')
    BUILD.mkdir(parents=True,exist_ok=True)
    flags = ['/nologo','/c','/O2','/Ob0','/Oy-','/Gr','/EHsc','/MT','/GS-',
             '/Zc:threadSafeInit-','/Zc:sizedDealloc-','/source-charset:utf-8','/execution-charset:.932',
             '/D_CRT_SECURE_NO_WARNINGS','/DTH09_EXE=1','/DTH09_NATIVE_NETPLAY=1','/DTH09_MODERN=1']
    original = (args.original/'th09.exe').read_bytes()
    checksum = sum(v[0] for v in struct.iter_unpack('<I',original[:(len(original)//4-1)*4])) & 0xffffffff
    flags+=['/DFEAT_ORIG_CHECKSUM=1','/DTH09_ORIGINAL_EXE_SIZE=%dU'%len(original),
            '/DTH09_ORIGINAL_EXE_CHECKSUM=0x%08xU'%checksum]
    sources = sorted((ROOT/'src').glob('*.cpp')) + sorted((ROOT/'src/exe').glob('*.cpp'))
    header_hash=hashlib.sha256()
    for header in sorted((ROOT/'src').rglob('*.h')):
        header_hash.update(header.read_bytes())
    for header in sorted(args.dx8_headers.iterdir()):
        if(header.suffix.lower() in ('.h','.inl')):header_hash.update(header.read_bytes())
    header_key=header_hash.hexdigest()
    def compile_one(src):
        obj = BUILD/((src.parent.name+'_' if src.parent.name=='exe' else '')+src.stem+'.obj')
        command = [str(compiler)] + flags + ['/Fo'+str(obj), str(src.relative_to(ROOT))]
        key=hashlib.sha256((header_key+'\0'.join(command)).encode()+src.read_bytes()).hexdigest()
        stamp=obj.with_suffix('.sha256')
        if obj.exists() and stamp.exists() and stamp.read_text()==key:
            return obj,0
        run = subprocess.run(command,env=env,cwd=ROOT,capture_output=True)
        text = (run.stdout+run.stderr).decode('utf-8',errors='replace')
        (BUILD/(obj.stem+'.log')).write_text(text,encoding='utf-8')
        print(('OK ' if run.returncode==0 else 'FAIL ')+src.name,flush=True)
        if run.returncode:
            print(text,flush=True)
        else:
            stamp.write_text(key)
        return obj,run.returncode
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(compile_one,sources))
    if any(code for _,code in results):
        return 1
    exe = BUILD/'th09-netplay.exe'
    command = [str(compiler.parent/'link.exe'),'/NOLOGO','/SUBSYSTEM:WINDOWS','/MACHINE:X86','/DYNAMICBASE:NO',
               '/NODEFAULTLIB:libci.lib','/NODEFAULTLIB:libc.lib',
               '/OUT:'+str(exe),'/MAP:'+str(BUILD/'th09-netplay.map')]
    command += [str(obj) for obj,_ in results]
    command += ['dinput8.lib','dsound.lib','d3d8.lib','d3dx8.lib','dxerr8.lib','dxguid.lib','winmm.lib','kernel32.lib',
                'user32.lib','gdi32.lib','advapi32.lib','ole32.lib','ws2_32.lib','legacy_stdio_definitions.lib']
    run = subprocess.run(command,env=env,cwd=ROOT,capture_output=True)
    output = (run.stdout+run.stderr).decode('utf-8',errors='replace')
    (BUILD/'link.log').write_text(output,encoding='utf-8')
    print(output,flush=True)
    if run.returncode:
        return run.returncode
    runtime = WORK/'game'
    runtime.mkdir(exist_ok=True)
    for name in ('th09.dat','thbgm.dat','th09.cfg'):
        if not (runtime/name).exists():
            shutil.copy2(args.original/name,runtime/name)
            if name=='th09.cfg':
                config=bytearray((runtime/name).read_bytes()); config[0xb1]=1
                (runtime/name).write_bytes(config)
    shutil.copy2(exe,runtime/exe.name)
    (BUILD/'build.json').write_text(json.dumps(dict(compiler=str(compiler),exe=str(runtime/exe.name),
        sha256=hashlib.sha256(exe.read_bytes()).hexdigest()),indent=2),encoding='utf-8')
    print('Built:',runtime/exe.name,flush=True)
    return 0

if __name__=='__main__':
    sys.exit(main())
