USAGE="Reads the game's project.toml, the one place for what differs between\nthe games: names, paths, the code's end, compiler and linker flags,\nlibraries.\n\n    project.py get KEY        print a setting (game.name, ghidra.project...)\n    project.py ghidra         print the Ghidra installation's directory\n    project.py orig           verify the original's checksums, copy it to build/\n    project.py game           set up build/game: game data, patched exe and DLL\n    project.py standalone     copy the standalone exe (and extras) into build/game\n    project.py check-exe      fail unless build/<name>.exe is the original byte for byte\n    project.py clean          remove build outputs (keeps the Ghidra project)\n\nThe file is project.toml in the game repository's root (the directory\nabove tools/), or the one the DECOMP_PROJECT environment variable names.\nOther tools import this module and read `config`, or the shorthands\nbelow. Paths in the file are relative to the repository's root.\n"
















import copy, glob, hashlib, os, shutil, subprocess, sys, tomllib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.dirname(os.path.abspath(__file__))
PATH = os.environ.get("DECOMP_PROJECT") or os.path.join(ROOT, "project.toml")

REQUIRED = object()  


DEFAULTS = {
    "game": {
        "name": REQUIRED,  
        "dir": REQUIRED,  
        "exe": REQUIRED,  
        "original": REQUIRED,  
        "game_end": REQUIRED,  
        "title": "",  
        "progress_exclude": [],  
        "window_title": "",  
        "checksums": "",  
        "standalone_extras": [],  
    },
    "ghidra": {
        "path": "",  
        "project": "",  
        "functions": "",  
    },
    "build": {
        "bin": r"C:\msvc7\bin",
        "include": [r"C:\dx8\include", r"C:\msvc7\include", r"C:\psdk\Include"],
        "lib": [r"C:\dx8\lib", r"C:\msvc7\lib", r"C:\msvc7\PlatformSDK\Lib", r"C:\psdk\Lib"],
        "cflags": REQUIRED,
        "timeout": 60,
        "hook": "",
    },
    "dll": {
        "ldflags": ["/DLL", "/NODEFAULTLIB", "/ENTRY:DllMain", "/BASE:0x30000000"],
        "libs": ["kernel32.lib", "user32.lib", "winmm.lib"],
    },
    "exe": {
        "obj_dir": "build/obj_exe",
        "cflags": [],
        "defines": [],
        "link_order": "median",
        "pch": "",  
        "ldflags": ["/SUBSYSTEM:WINDOWS", "/FIXED", "/NODEFAULTLIB:libc.lib"],
        "libs": ["libcmt.lib"],
        "features": {},
        "feature_sources": {},
    },
    "psdk": {
        "libs": [],
    },
    "symbols": {
        "import_libs": [".wine/drive_c/psdk/Lib", ".wine/drive_c/dx8/lib"],
    },
    "replay": {
        "hook": "",  
    },
    "hooks": {
        "progress": False,  
        "compare": False,  
        "readme": False,  
        "allow_diff": [],  
    },
}

def merge(defaults, values, where=""):

    out = copy.deepcopy(defaults)
    for key, value in values.items():
        name = where + key
        if key not in defaults:
            sys.exit(f"{PATH}: unknown setting {name}")
        if isinstance(value, dict) and isinstance(defaults.get(key), dict) and defaults[key]:
            out[key] = merge(defaults[key], value, name + ".")
        else:
            out[key] = value
    return out


def load(path=PATH):
    try:
        with open(path, "rb") as f:
            values = tomllib.load(f)
    except FileNotFoundError:
        sys.exit(f"{path} is missing: copy one from tools/examples/ and adapt it")
    config = merge(DEFAULTS, values)
    for section, settings in config.items():
        for name, value in settings.items():
            if value is REQUIRED:
                sys.exit(f"{path}: {section}.{name} is required")
    game, ghidra = config["game"], config["ghidra"]
    game["title"] = game["title"] or game["name"]
    game["window_title"] = game["window_title"] or game["title"]
    stem = os.path.splitext(os.path.basename(game["original"]))[0]
    ghidra["project"] = ghidra["project"] or stem
    ghidra["functions"] = ghidra["functions"] or f"build/functions_{ghidra['project']}.csv"
    return config


config = load()
NAME = config["game"]["name"]
ORIGINAL = config["game"]["original"]  
GAME_END = config["game"]["game_end"]  
DLL = f"build/{NAME}.dll"
EXE = f"build/{NAME}.exe"
EXE_MAP = f"build/{NAME}_exe.map"  
GAME = "build/game"  


def get(key):
    value = config
    for part in key.split("."):
        if not isinstance(value, dict) or part not in value:
            sys.exit(f"no setting {key}")
        value = value[part]
    return value


def ghidra_home():


    path = os.environ.get("GHIDRA") or config["ghidra"]["path"]
    if path:
        return os.path.join(ROOT, os.path.expanduser(path))
    found = sorted(glob.glob(os.path.expanduser("~/opt/ghidra_*")))
    if not found:
        sys.exit("no Ghidra: set GHIDRA or ghidra.path in project.toml")
    return found[-1]


def orig():


    sums = config["game"]["checksums"]
    if sums:
        subprocess.run(["sha1sum", "--quiet", "-c", os.path.basename(sums)],
                       cwd=os.path.join(ROOT, os.path.dirname(sums)), check=True)
    os.makedirs(os.path.dirname(ORIGINAL), exist_ok=True)
    shutil.copyfile(os.path.join(config["game"]["dir"], config["game"]["exe"]), ORIGINAL)


def game():


    import patch_exe
    shutil.copytree(config["game"]["dir"], GAME, dirs_exist_ok=True)
    os.remove(os.path.join(GAME, config["game"]["exe"]))
    patch_exe.patch(ORIGINAL, os.path.join(GAME, f"{NAME}.exe"))
    shutil.copyfile(DLL, os.path.join(GAME, os.path.basename(DLL)))


def standalone():


    shutil.copyfile(EXE, os.path.join(GAME, f"{NAME}_standalone.exe"))
    for pattern in config["game"]["standalone_extras"]:
        for path in sorted(glob.glob(pattern)):
            shutil.copy(path, GAME)


def check_exe():


    with open(EXE, "rb") as f:
        built = f.read()
    with open(ORIGINAL, "rb") as f:
        original = f.read()
    if built != original:
        differ = sum(a != b for a, b in zip(built, original)) + abs(len(built) - len(original))
        first = next((i for i, (a, b) in enumerate(zip(built, original)) if a != b),
                     min(len(built), len(original)))
        sys.exit(f"{EXE}: {differ:,} bytes differ from {ORIGINAL}, the first at "
                 f"file offset {first:#x}")
    print(f"{EXE} is {ORIGINAL} byte for byte: {len(built):,} bytes, "
          f"SHA-1 {hashlib.sha1(built).hexdigest()}")


def clean():
    paths = ["build/src", GAME, *glob.glob("build/obj*"), *glob.glob(f"build/{NAME}.*"),
             *glob.glob(f"build/{NAME}_exe.*")]
    for path in paths:
        if os.path.isdir(path):
            shutil.rmtree(path)
        elif os.path.exists(path):
            os.remove(path)


def main():
    os.chdir(ROOT)
    args = sys.argv[1:]
    if args[:1] == ["get"] and len(args) == 2:
        value = get(args[1])
        print(" ".join(map(str, value)) if isinstance(value, list) else value)
    elif args == ["ghidra"]:
        print(ghidra_home())
    elif args == ["check-exe"]:
        check_exe()
    elif len(args) == 1 and args[0] in ("orig", "game", "standalone", "clean"):
        globals()[args[0]]()
    else:
        sys.exit(USAGE)


if __name__ == "__main__":
    main()
