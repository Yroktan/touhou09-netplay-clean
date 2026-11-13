import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import zipfile

ROOT=Path(__file__).resolve().parents[1]


def excluded(path):
    path=Path(path)
    return (bool(set(path.parts)&{'tests','reports','verification'})
            or path.name.startswith('test_') or path.stem.endswith('_test')
            or path.name in {'Verify.cmd','smoke.py','verify.py','final_report.py',
                             'test_network.py','parity.py','parity_probe.cpp'}
            or path.as_posix()=='client/docs/parity.md')


def validate(directory,private_values=()):
    directory=Path(directory)
    manifest=json.loads((directory/'SHA256SUMS.json').read_text(encoding='utf-8'))
    actual={path.relative_to(directory).as_posix() for path in directory.rglob('*')
            if path.is_file() and '.git' not in path.relative_to(directory).parts}
    if actual!=set(manifest)|{'SHA256SUMS.json'}:
        raise ValueError('Release manifest does not match its files')
    credentials=re.compile(rb'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{30,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----)')
    for name,checksum in manifest.items():
        path=directory/name
        if excluded(name):
            raise ValueError('Development artifact in release: '+name)
        data=path.read_bytes()
        if hashlib.sha256(data).hexdigest()!=checksum:
            raise ValueError('Checksum mismatch: '+name)
        if path.name.startswith('.env') and path.name!='.env.example':
            raise ValueError('Local environment file in release')
        if path.name=='.env.example' and re.search(rb'(?m)^[ \t]*TH09_RELAY_HOST[ \t]*=[ \t]*\S',data):
            raise ValueError('Environment template contains an endpoint')
        if path.stem.lower() in {'credentials','secrets','passwords','known_hosts','id_rsa','id_ed25519'} or path.name=='netplay.ini' or path.suffix.lower() in {'.dat','.cfg','.rpy','.lib','.obj','.pdb','.key','.pem','.pyc'}:
            raise ValueError('Private artifact in release: '+name)
        if credentials.search(data):
            raise ValueError('Credential-shaped data in release: '+name)
        for value in private_values:
            if len(value)>=8 and any(encoded in data for encoded in (value.encode('utf-8'),value.encode('utf-16-le'))):
                raise ValueError('Private configuration data in release: '+name)
    for name in ('mod/th09-netplay.exe','mod/Touhou09-Netplay-Setup.exe'):
        if name not in manifest or not (directory/name).read_bytes().startswith(b'MZ'):
            raise ValueError('Missing Windows binary: '+name)
    return len(actual)


def validate_archive(directory):
    directory=Path(directory)
    files={path.relative_to(directory).as_posix():hashlib.sha256(path.read_bytes()).hexdigest()
           for path in directory.rglob('*') if path.is_file() and '.git' not in path.relative_to(directory).parts}
    expected={directory.name+'/'+name:checksum for name,checksum in files.items()}
    with zipfile.ZipFile(Path(str(directory)+'.zip')) as archive:
        if len(archive.namelist())!=len(expected) or set(archive.namelist())!=set(expected):
            raise ValueError('Archive contents do not match the release')
        for name,checksum in expected.items():
            if hashlib.sha256(archive.read(name)).hexdigest()!=checksum:
                raise ValueError('Archive checksum mismatch: '+name)


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--private-values',type=Path)
    args=parser.parse_args()
    target=args.output.resolve()
    archive_path=Path(str(target)+'.zip')
    if target==ROOT or ROOT in target.parents or target.exists() or archive_path.exists():
        raise ValueError('Use a new output directory outside the development project')
    target.mkdir(parents=True)
    def copy(source,relative):
        destination=target/relative
        destination.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(source,destination)
    for name in ('README.md','LICENSE','NOTICE.txt','.env.example','.gitignore','.gitattributes','Build.cmd','Install.cmd'):
        copy(ROOT/name,name)
    allowed={'.py','.cpp','.h','.c','.java','.sh','.ps1','.cmd','.md','.txt','.toml','.json','.csv','.svg','.sha1','.manifest','.just'}
    special={'LICENSE','justfile','.gitignore','.gitmodules','.env.example','commit-msg','pre-commit'}
    for folder in ('client','server','docs','scripts','installer','deploy'):
        for source in sorted((ROOT/folder).rglob('*')):
            if not source.is_file():
                continue
            relative=source.relative_to(ROOT)
            if excluded(relative):
                continue
            if any(part in {'.git','build','__pycache__','.wine'} for part in relative.parts):
                continue
            if relative.parts[:2]==('client','orig') and source.suffix!='.sha1' and source.name!='README.md':
                continue
            if source.name.startswith('.env') and source.name!='.env.example':
                continue
            if source.suffix.lower() not in allowed and source.name not in special:
                continue
            if b'\0' in source.read_bytes():
                continue
            copy(source,relative)
    for name in ('setup.py','finish_toolchain.py','vsdownload.py','LICENSE.vsdownload.txt'):
        copy(ROOT/'toolchain'/name,Path('toolchain')/name)
    client=ROOT/'game/th09-netplay.exe'
    setup=ROOT/'installer/build/Touhou09-Netplay-Setup.exe'
    copy(client if client.is_file() else ROOT/'mod/th09-netplay.exe','mod/th09-netplay.exe')
    copy(setup if setup.is_file() else ROOT/'mod/Touhou09-Netplay-Setup.exe','mod/Touhou09-Netplay-Setup.exe')
    manifest={path.relative_to(target).as_posix():hashlib.sha256(path.read_bytes()).hexdigest() for path in sorted(target.rglob('*')) if path.is_file()}
    (target/'SHA256SUMS.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    values=json.loads(args.private_values.read_text(encoding='utf-8')) if args.private_values else ()
    count=validate(target,values)
    with zipfile.ZipFile(archive_path,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
        for path in sorted(target.rglob('*')):
            if path.is_file():
                archive.write(path,Path(target.name)/path.relative_to(target))
    validate_archive(target)
    print('PASS: clean package prepared,',count,'files')
    print('Ready:',target)


if __name__=='__main__':
    main()
