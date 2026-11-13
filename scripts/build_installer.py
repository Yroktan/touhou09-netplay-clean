import hashlib
import ctypes
import json
import os
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]


def array(name,data):
    rows=[','.join('0x%02x'%value for value in data[index:index+24]) for index in range(0,len(data),24)]
    return 'static const unsigned char '+name+'[]={\n'+',\n'.join(rows)+'\n};\n'


def embed_manifest(exe):
    kernel=ctypes.WinDLL('kernel32',use_last_error=True)
    kernel.BeginUpdateResourceW.argtypes=[ctypes.c_wchar_p,ctypes.c_int]
    kernel.BeginUpdateResourceW.restype=ctypes.c_void_p
    kernel.UpdateResourceW.argtypes=[ctypes.c_void_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ushort,ctypes.c_void_p,ctypes.c_ulong]
    kernel.EndUpdateResourceW.argtypes=[ctypes.c_void_p,ctypes.c_int]
    handle=kernel.BeginUpdateResourceW(str(exe),False)
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    data=(ROOT/'installer/setup.manifest').read_bytes()
    buffer=ctypes.create_string_buffer(data)
    if not kernel.UpdateResourceW(handle,24,1,0,buffer,len(data)):
        error=ctypes.get_last_error()
        kernel.EndUpdateResourceW(handle,True)
        raise ctypes.WinError(error)
    if not kernel.EndUpdateResourceW(handle,False):
        raise ctypes.WinError(ctypes.get_last_error())


def main():
    tools=ROOT/'toolchain/msvc'
    compiler=next(tools.glob('VC/Tools/MSVC/*/bin/Hostx64/x86/cl.exe'))
    header=next(tools.glob('Windows Kits/10/Include/*/um/Windows.h'))
    sdk=header.parents[3]
    version=header.parents[1].name
    vc=compiler.parents[3]
    build=ROOT/'installer/build'
    build.mkdir(exist_ok=True)
    client=(ROOT/'game/th09-netplay.exe').read_bytes()
    license=(ROOT/'LICENSE').read_bytes()
    (build/'payload.h').write_text(array('kClientExe',client)+array('kLicense',license),encoding='ascii')
    includes=[ROOT/'client/src',build,vc/'include']+[sdk/'Include'/version/name for name in ('ucrt','shared','um','winrt')]
    libraries=[vc/'lib/x86',sdk/'Lib'/version/'ucrt/x86',sdk/'Lib'/version/'um/x86']
    env=dict(os.environ,INCLUDE=';'.join(map(str,includes)),LIB=';'.join(map(str,libraries)))
    env['PATH']=str(compiler.parent)+';'+env.get('PATH','')
    exe=build/'Touhou09-Netplay-Setup.exe'
    command=[str(compiler),'/nologo','/O2','/EHsc','/MT','/W4','/DUNICODE','/D_UNICODE','/D_CRT_SECURE_NO_WARNINGS',
             'installer/setup.cpp','client/src/NetConfig.cpp','/Fo'+str(build)+'\\','/Fe'+str(exe),
             '/link','/SUBSYSTEM:WINDOWS','/MACHINE:X86','/MANIFEST:NO','user32.lib','gdi32.lib','shell32.lib','ole32.lib','advapi32.lib','normaliz.lib']
    subprocess.run(command,cwd=ROOT,env=env,check=True)
    embed_manifest(exe)
    report=dict(client_sha256=hashlib.sha256(client).hexdigest(),installer_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),size=exe.stat().st_size)
    (build/'installer.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print('Built standalone installer:',exe)


if __name__=='__main__':
    main()
