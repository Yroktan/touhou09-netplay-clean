USAGE='Usage: python tools/build.py [exe]. Build settings: project.toml and config/source-flags.json.'

























































import collections, csv, glob, importlib.util, json, os, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

import coff, project

ROOT = project.ROOT
BUILD, DLL_CONFIG, EXE_CONFIG = (project.config[k] for k in ("build", "dll", "exe"))
OBJ = "build/obj"
OBJ_EXE = EXE_CONFIG["obj_dir"]
SJIS = "build/src"
DLL = project.DLL
EXE = project.EXE
EXE_MAP = project.EXE_MAP
ORIGINAL = project.ORIGINAL
CFLAGS = BUILD["cflags"]


EXE_CFLAGS = CFLAGS + EXE_CONFIG["cflags"]
EXE_DEFINES = ["/D" + d for d in EXE_CONFIG["defines"]]
GL = "/GL" in EXE_CFLAGS
PCH = EXE_CONFIG["pch"]
PCH_STUB = "pch"
FEATURES = EXE_CONFIG["features"]

FEATURE_SOURCES = {f: {os.path.normpath(p) for p in paths}
                   for f, paths in EXE_CONFIG["feature_sources"].items()}
GLUE_PREFIX = "dll_"
DLL_ONLY, EXE_ONLY = "dll", "exe"
TIMEOUT = BUILD["timeout"]


JOBS = int(os.environ.get("JOBS") or max(1, (os.cpu_count() or 2) // 2))

NATIVE = os.name == "nt"


def host(path):

    if NATIVE and path[:3].upper() == "C:\\":
        local = os.path.join(ROOT, ".wine", "drive_c", path[3:])
        if os.path.exists(local):
            return local
    return path


ENV = dict(
    os.environ,
    WINEPREFIX=os.environ.get("WINEPREFIX", os.path.join(ROOT, ".wine")),
    WINEDEBUG=os.environ.get("WINEDEBUG", "-all"),
    INCLUDE=";".join(map(host, BUILD["include"])),
    LIB=";".join(map(host, BUILD["lib"])),
)

_hook = None


def hook(name):

    global _hook
    if _hook is None:
        _hook = False
        if BUILD["hook"]:
            path = os.path.join(ROOT, BUILD["hook"])
            spec = importlib.util.spec_from_file_location("build_hook", path)
            _hook = importlib.util.module_from_spec(spec)
            sys.modules[spec.name] = _hook
            spec.loader.exec_module(_hook)
    return getattr(_hook, name, None) if _hook else None


def win(path):
    return path.replace("/", "\\")


def run_tool(name: str, *args: str) -> tuple[int, str]:


    cmd = ([] if NATIVE else ["wine"]) + [host(BUILD["bin"] + "\\" + name + ".exe"), "/nologo", *args]
    try:
        r = subprocess.run(cmd, env=ENV, timeout=TIMEOUT, capture_output=True, text=True, errors="replace")
    except subprocess.TimeoutExpired:
        return 1, f"{name} timed out (fatal error?): {' '.join(args)}"
    out = "\n".join(l for l in r.stdout.splitlines() if l.strip() and not l.endswith(".cpp"))


    out = "\n".join(part for part in (out, r.stderr.strip()) if part)
    return r.returncode, out


def keep_wineserver(seconds=30):



    if not NATIVE:
        subprocess.run(["wineserver", f"-p{seconds}"], env=ENV)


def tool(name, *args):
    code, out = run_tool(name, *args)
    if out:
        print(out)
    if code:
        sys.exit(code)


def read_symbols():
    with open("config/symbols.csv") as f:
        return {s: int(r["address"], 16) for r in csv.DictReader(f) for s in r["symbol"].split()}


def to_sjis(text):






    out = bytearray()
    for ch in text:
        b = ch.encode("cp932")
        out += b
        if len(b) == 2 and b[1] == 0x5C:
            out += b"\\"
    return bytes(out)


def features():

    names = sorted({f.strip() for f in os.environ.get("FEAT", "").split(",") if f.strip()})
    unknown = [f for f in names if f not in FEATURES]
    if unknown:
        sys.exit(f"unknown FEAT {', '.join(unknown)}; known: {', '.join(FEATURES) or 'none'}")
    return names


def compile_all(target=DLL, exe_flags=False, feats=()):











    glue = []
    if not exe_flags:
        obj_dir, base, defines = OBJ, CFLAGS, []
        glue = ["/D%s=1" % FEATURES[f] for f in feats]
        feats = ()
    else:
        obj_dir = "_".join([OBJ, *feats]) if feats else OBJ_EXE
        base = EXE_CFLAGS
        defines = EXE_DEFINES + ["/D%s=1" % FEATURES[f] for f in feats]
        extra = hook("exe_flags")
        if extra:
            defines += extra(list(feats))
    os.makedirs(obj_dir, exist_ok=True)
    for src in glob.glob("src/**/*.*", recursive=True):
        copy = os.path.join(SJIS, os.path.relpath(src, "src"))
        if not os.path.exists(copy) or os.path.getmtime(copy) < os.path.getmtime(src):
            os.makedirs(os.path.dirname(copy), exist_ok=True)
            with open(src, encoding="utf-8") as f:
                text = f.read()
            with open(copy, "wb") as f:
                f.write(to_sjis(text))
    more = hook("headers")
    headers = max((os.path.getmtime(h) for h in glob.glob("src/**/*.h", recursive=True)
                   + (list(more()) if more else []) if os.path.exists(h)), default=0)
    record = os.path.join(obj_dir, "flags.json")
    try:
        with open(record) as f:
            built = json.load(f)
    except (OSError, ValueError):
        built = {}
    objs, stale = [], []
    pch = exe_flags and not GL and PCH
    if pch:
        pch_obj, pch_flags = precompile(obj_dir, base + defines, headers, built)
        objs.append(pch_obj)
        newest = os.path.getmtime(pch_obj)
    for src, obj in sources(target, obj_dir, feats):
        objs.append(obj)
        flags = file_flags(src, base) + defines
        if os.path.basename(obj).startswith(GLUE_PREFIX):
            flags += glue
        newer = max(os.path.getmtime(src), headers)
        if pch and uses_pch(src, base):
            flags += pch_flags
            newer = max(newer, newest)
        if not (os.path.exists(obj) and os.path.getmtime(obj) > newer and built.get(obj) == flags):
            stale.append((src, obj, flags))

    def compile_one(job):
        src, obj, flags = job
        return run_tool("cl", "/c", *flags, "/I" + win(SJIS), "/Fo" + win(obj),
                        win(os.path.join(SJIS, os.path.relpath(src, "src"))))

    failed = 0
    with ThreadPoolExecutor(JOBS) as pool:

        for (src, obj, flags), (code, out) in zip(stale, pool.map(compile_one, stale)):
            print("cl", src)
            if out:
                print(out)
            if code:
                built.pop(obj, None)
                failed = failed or code
            else:
                built[obj] = flags
    with open(record, "w") as f:
        json.dump(built, f, indent=1, sort_keys=True)
    if failed:
        sys.exit(failed)
    return objs


def precompile(obj_dir, flags, headers, built):



    stub, obj = (os.path.join(obj_dir, PCH_STUB + ext) for ext in (".cpp", ".obj"))
    pch = os.path.join(obj_dir, project.NAME + ".pch")
    text = f'#include "{PCH}"\n'
    if not os.path.exists(stub) or open(stub).read() != text:
        with open(stub, "w") as f:
            f.write(text)
    flags = [*flags, "/Yc" + PCH, "/Fp" + win(pch)]
    if not (os.path.exists(obj) and os.path.exists(pch) and built.get(obj) == flags
            and min(map(os.path.getmtime, (obj, pch))) > max(headers, os.path.getmtime(stub))):
        print("cl", stub)
        code, out = run_tool("cl", "/c", *flags, "/I" + win(SJIS), "/Fo" + win(obj), win(stub))
        if out:
            print(out)
        if code:
            built.pop(obj, None)
            sys.exit(code)
        built[obj] = flags
    return obj, ["/FI" + PCH, "/Yu" + PCH, "/Fp" + win(pch)]


def uses_pch(src, base):



    opt = lambda flags: {f for f in flags if f.startswith("/O")}
    return opt(file_flags(src, base)) == opt(base)


def sources(target=DLL, obj_dir=OBJ, feats=()):


    skip = EXE_ONLY if target == DLL else DLL_ONLY
    unused = set().union(*(paths for f, paths in FEATURE_SOURCES.items() if f not in feats))
    for src in sorted(glob.glob(os.path.join(ROOT, "src/**/*.cpp"), recursive=True)):
        rel = os.path.relpath(src, os.path.join(ROOT, "src"))
        if rel.split(os.sep)[0] == skip or os.path.normpath(os.path.relpath(src, ROOT)) in unused:
            continue
        yield os.path.relpath(src, ROOT), os.path.join(obj_dir, rel.replace(os.sep, "_")[:-4] + ".obj")


def file_flags(src, base=CFLAGS):


    key=os.path.relpath(os.path.abspath(src),ROOT).replace(os.sep,"/")
    with open(os.path.join(ROOT,"config/source-flags.json"),encoding="utf-8") as source:
        changes=json.load(source).get(key,[])
    flags = [f for f in base if "-" + f not in changes]
    return flags + [c for c in changes if not c.startswith("-")]


def write_hooks(objs, symbols):






    defined, code, undefined, called, glue = set(), set(), set(), set(), set()
    for path in objs:
        obj = coff.Object(path)
        for s in obj.externals():
            if s.section <= 0:
                undefined.add(s.name)
                if os.path.basename(path).startswith(GLUE_PREFIX):
                    glue.add(s.name)
                continue
            defined.add(s.name)
            if obj.sections[s.section - 1].flags & coff.CODE:
                code.add(s.name)
        for sec in obj.sections:
            called.update(obj.symbols[i].name for _, i, t in sec.relocs if t == coff.REL32)


    hooks = sorted({symbols[n]: n for n in sorted(code & symbols.keys())}.items())


    external = {n: a for n, a in symbols.items()
                if n in undefined - defined and not (n.startswith("__imp_") and n in glue)}

    table, table_relocs = bytearray(), []
    for addr, name in hooks:
        table_relocs.append((len(table) + 4, name, coff.DIR32))
        table += struct.pack("<II", addr, 0)
    table += bytes(8)
    thunks, entry = bytearray(), {"_g_Hooks": (1, 0)}
    for name, addr in external.items():
        if name in called:
            entry[name] = (2, len(thunks))
            thunks += struct.pack("<BIB", 0x68, addr, 0xC3).ljust(8, b"\xcc")
    absolute = {n: a for n, a in external.items() if n not in entry}


    if "__except_list" in undefined:
        absolute["__except_list"] = 0

    path = os.path.join(OBJ, "hooks.obj")
    coff.write_object(path, [(".rdata", coff.RDATA, bytes(table), table_relocs),
                             (".text", coff.TEXT, bytes(thunks), [])],
                      entry, [n for _, n in hooks], absolute)
    print(f"{len(hooks)} hooked, {len(external)} used from the original")
    return path


def link_order(objs, symbols, how):












    if how == "source":
        return list(objs)
    if how not in ("median", "first"):
        sys.exit(f"exe.link_order is median, first or source, not {how}")
    code = {}
    for path in objs:
        obj = coff.Object(path)
        code[path] = {s.name for s in obj.externals()
                      if s.section > 0 and obj.sections[s.section - 1].flags & coff.CODE}
    count = collections.Counter(n for names in code.values() for n in names)

    def key(path):
        addrs = sorted(symbols[n] for n in code[path] if count[n] == 1 and n in symbols)
        if not addrs:
            return 1 << 32
        return addrs[len(addrs) // 2] if how == "median" else addrs[0]
    return sorted(objs, key=key)


def link_exe(objs, exe_objs=None, feats=()):




    import rsrc
    exe_objs = exe_objs or objs
    res = os.path.join(os.path.dirname(exe_objs[0]), f"{project.NAME}.res")
    resources = hook("exe_resources")
    image = (resources(list(feats)) if resources else None) or ORIGINAL
    rsrc.write_res(res, rsrc.resources(image))
    linked = dict(zip(objs, exe_objs))
    ordered = [linked[o] for o in link_order(objs, read_symbols(), EXE_CONFIG["link_order"])]
    prepare = hook("exe_objects")
    if prepare:
        ordered = prepare(ordered, list(feats))
    tool("link", *EXE_CONFIG["ldflags"], "/MAP:" + win(EXE_MAP), "/OUT:" + win(EXE),
         *map(win, ordered), win(res), *EXE_CONFIG["libs"])
    copy_header_fields(ORIGINAL, EXE)
    finish = hook("exe_finished")
    if finish:
        finish(list(feats))
    print("linked", EXE)


def copy_header_fields(src, dst):


    with open(src, "rb") as f:
        orig = f.read(0x400)
    pe = struct.unpack_from("<I", orig, 0x3C)[0]
    with open(dst, "r+b") as f:
        f.seek(0x3C)
        dpe = struct.unpack("<I", f.read(4))[0]
        f.seek(dpe + 8)
        f.write(orig[pe + 8:pe + 12])


def main():
    os.chdir(ROOT)
    if sys.argv[1:] == ["exe"]:
        feats = features()
        if feats:
            print("features:", ", ".join(feats))
        if GL:

            link_exe(compile_all(EXE, feats=feats), compile_all(EXE, True, feats), feats)
        else:
            link_exe(compile_all(EXE, True, feats), feats=feats)
        return
    if sys.argv[1:]:
        sys.exit(USAGE)


    feats = features()
    if feats:
        print("DLL features:", ", ".join(feats))
    objs = link_order(compile_all(feats=feats), read_symbols(), EXE_CONFIG["link_order"])
    hooks = write_hooks(objs, read_symbols())
    tool("link", *DLL_CONFIG["ldflags"], "/MAP:" + win(DLL[:-4] + ".map"), "/OUT:" + win(DLL),
         *map(win, objs), win(hooks), *DLL_CONFIG["libs"])
    print("linked", DLL)


if __name__ == "__main__":
    main()
