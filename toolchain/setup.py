"""Prepare the private Microsoft x86 build tools; no SSH access is needed."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import zipfile

ROOT=Path(__file__).resolve().parent
LIBRARIES=('d3dx8.lib','dxerr8.lib','d3d8.lib','dinput8.lib','dsound.lib','dxguid.lib')


def main():
    parser=argparse.ArgumentParser(__doc__)
    parser.add_argument('--dx8-installer',type=Path,help='Original dx81sdk_full.exe archive; extracted without executing it')
    parser.add_argument('--accept-license',action='store_true',help='Accept Microsoft compiler/SDK license terms')
    parser.add_argument('--check',action='store_true',help='Only inspect dependencies; download nothing')
    args=parser.parse_args()
    compiler=next((ROOT/'msvc').glob('VC/Tools/MSVC/14.29.*/bin/Hostx64/x86/cl.exe'),None)
    sdk=next((ROOT/'msvc').glob('Windows Kits/10/Include/10.0.19041.0/um/Windows.h'),None)
    if (compiler is None or sdk is None) and not args.check:
        env=dict(os.environ,TH09_MINIMAL_SDK='1')
        command=[sys.executable,str(ROOT/'vsdownload.py'),'--major','16','--msvc-version','14.29',
                 '--sdk-version','10.0.19041','--architecture','x86','--host-arch','x64',
                 '--with-default','no','--with-msvc','yes','--with-sdk','yes','--skip-recommended',
                 '--cache',str(ROOT/'cache'),'--dest',str(ROOT/'msvc')]
        if args.accept_license:command.append('--accept-license')
        subprocess.run(command,env=env,check=True)
    if args.dx8_installer and not args.check:
        installer=args.dx8_installer.resolve()
        expected='61b5733209205e942f37431ee40da712e1f50e6a'
        if hashlib.sha1(installer.read_bytes()).hexdigest()!=expected:
            raise RuntimeError('The DirectX 8.1 SDK archive hash does not match')
        (ROOT/'dx8-headers').mkdir(exist_ok=True)
        (ROOT/'dx8-lib').mkdir(exist_ok=True)
        with zipfile.ZipFile(installer) as archive:
            for name in archive.namelist():
                normalized=name.replace(chr(92),'/')
                filename=normalized.rsplit('/',1)[-1].lower()
                if '/include/' in normalized.lower() and filename.endswith(('.h','.inl')) and filename!='basetsd.h':
                    (ROOT/'dx8-headers'/filename).write_bytes(archive.read(name))
                elif filename in LIBRARIES:
                    (ROOT/'dx8-lib'/filename).write_bytes(archive.read(name))
    required=[ROOT/'dx8-headers/d3d8.h',ROOT/'dx8-headers/d3dx8.h']+[ROOT/'dx8-lib'/name for name in LIBRARIES]
    missing=[str(path) for path in required if not path.is_file()]
    if not next((ROOT/'msvc').glob('VC/Tools/MSVC/14.29.*/bin/Hostx64/x86/cl.exe'),None):missing.append('MSVC v142 x86 compiler')
    if not next((ROOT/'msvc').glob('Windows Kits/10/Include/10.0.19041.0/um/Windows.h'),None):missing.append('Windows SDK 10.0.19041')
    if missing:
        raise RuntimeError('Missing dependencies:\n'+'\n'.join(missing))
    print('PASS: compiler, Windows SDK, DirectX headers and static libraries are present')


if __name__=='__main__':main()
