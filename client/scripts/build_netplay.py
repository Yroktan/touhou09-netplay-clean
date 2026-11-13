
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tomllib

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build' / 'netplay'

def native(path):
    local = ROOT / '.wine' / 'drive_c' / path[3:]
    return local if local.exists() else Path(path)

def main():
    cfg = tomllib.loads((ROOT / 'project.toml').read_text(encoding='utf-8'))
    build = cfg['build']
    binary = native(build['bin'])
    include = [native(p) for p in build['include']]
    libraries = [native(p) for p in build['lib']]
    missing = []
    for name in ('cl.exe', 'link.exe'):
        if not (binary / name).is_file():
            missing.append(str(binary / name))
    for name in ('windows.h', 'd3dx8.h', 'stdio.h'):
        if not any((d / name).is_file() for d in include):
            missing.append(name + ' in configured include paths')
    for name in cfg['exe']['libs']:
        if not any((d / name).is_file() for d in libraries):
            missing.append(name + ' in configured library paths')
    OUT.mkdir(parents=True, exist_ok=True)
    report = dict(compiler=str(binary), missing=missing, features=['netplay', 'orig_checksum'])
    (OUT / 'toolchain-check.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    if missing:
        print('GAME BUILD BLOCKED: original VC 7.1 compiler / SDK files are missing:')
        for item in missing:
            print('  ' + item)
        print('Expected directories are in th09/project.toml; local .wine/drive_c paths also work.')
        return 2
    env = dict(os.environ, FEAT='netplay,orig_checksum', PYTHONUTF8='1')
    result = subprocess.run([sys.executable, '-B', 'tools/build.py', 'exe'], cwd=ROOT, env=env)
    if result.returncode:
        return result.returncode
    runtime = ROOT.parent / 'game'
    runtime.mkdir(exist_ok=True)
    for name in ('th09.dat', 'thbgm.dat'):
        shutil.copy2(ROOT / 'orig' / 'th09_150a' / name, runtime / name)

    config = runtime / 'th09.cfg'
    if not config.exists():
        shutil.copy2(ROOT / 'orig' / 'th09_150a' / 'th09.cfg', config)
    shutil.copy2(ROOT / 'build' / 'th09.exe', runtime / 'th09-netplay.exe')
    print('Built game/th09-netplay.exe. In-game validation is still required.')
    return 0

if __name__ == '__main__':
    sys.exit(main())
