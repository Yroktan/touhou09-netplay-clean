























import csv, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import project

ROOT = project.ROOT
MAP = os.path.join(ROOT, project.EXE_MAP)
SYMBOLS = os.path.join(ROOT, "config/symbols.csv")
GAME = (0x401000, project.GAME_END)


def read_map():


    out = []
    rx = re.compile(r"^\s*0001:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+f\s+(i\s+)?(\S+)\s*$")
    for line in open(MAP, encoding="latin-1"):
        m = rx.match(line)
        if m:
            out.append((int(m.group(2), 16), m.group(1), bool(m.group(3)), m.group(4)))
    return out


def read_syms():

    d = {}
    for r in csv.DictReader(open(SYMBOLS)):
        a = int(r["address"], 16)
        if GAME[0] <= a < GAME[1]:
            for s in r["symbol"].split():
                d[s] = (a, r["name"])
    return d


def objects():
    syms = read_syms()
    objs = {}
    for addr, sym, inl, obj in read_map():
        if sym in syms:
            objs.setdefault(obj, []).append((addr, sym, inl))
    return objs, syms


def main():
    objs, syms = objects()
    if "--summary" in sys.argv:
        tot = ok = 0
        for obj, fl in sorted(objs.items(), key=lambda kv: min(syms[s][0] for _, s, _ in kv[1])):
            n = len(fl)
            k = sum(1 for a, s, _ in fl if a == syms[s][0])
            ranked = sorted(range(n), key=lambda i: syms[fl[i][1]][0])
            inorder = sum(1 for i, r in enumerate(ranked) if i == r)
            deltas = {fl[i][0] - syms[fl[i][1]][0] for i in range(n)}
            tot += n
            ok += k
            first = min(syms[s][0] for _, s, _ in fl)
            print(f"{obj:24s} {k:3d}/{n:3d} at addr, {inorder:3d} in rank, "
                  f"{len(deltas):2d} deltas  first orig {first:#x}")
        print(f"total {ok}/{tot}")
        return
    want = [a.replace(".obj", "") for a in sys.argv[1:]]
    for obj, fl in objs.items():
        if want and obj.replace(".obj", "") not in want:
            continue
        rank = {s: i for i, (_, s, _) in enumerate(sorted(fl, key=lambda t: syms[t[1]][0]))}
        print(f"== {obj}")
        for i, (addr, sym, inl) in enumerate(fl):
            oa, name = syms[sym]
            mark = "  " if addr == oa else "**"
            print(f"{mark} {addr:#x} orig {oa:#x} {addr - oa:+#7x} {i:3d}->{rank[sym]:3d} "
                  f"{'i' if inl else ' '} {name}")


if __name__ == "__main__":
    main()
