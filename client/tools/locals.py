



























import heapq, os, random, re, string, struct, subprocess, sys, tempfile

import build

M = 0xffffffff


def name_hash(name):

    b = name.encode("latin-1")
    h = 0xb170a1bf
    n = len(b) // 4
    for w in struct.unpack_from(f"<{n}I", b):
        h = (h + w) * 0x401 & M
        h ^= h >> 6
    for c in b[4 * n:]:
        h = (h + (c - 256 if c >= 128 else c)) * 0x401 & M
        h ^= h >> 6
    h = (h * 0x19660d + 0x3c6ef35f) & M
    return (h ^ h >> 16) & 0xffff


def bucket(name):
    return name_hash(name) & 15


def order(names):

    return [n for _, _, n in sorted((bucket(n), -i, n) for i, n in enumerate(names))]


def layout(specs):

    info = {}
    for spec in specs:
        name, _, rest = spec.partition(":")
        m = re.fullmatch(r"(\d*)([dg]*)", rest)
        info[name] = (int(m.group(1) or 4), "d" in m.group(2), "g" in m.group(2))
    out, cur, cookie = [], 0, False
    for name in order(list(info)):
        size, double, gs = info[name]
        if gs and not cookie:
            cur, cookie = (cur + 7) // 4 * 4, True
            out.append(("(cookie)", cur))
        if size == 1:
            cur += 1
        else:
            align = 8 if double else 4
            cur = -(-(cur + -(-size // 4) * 4) // align) * align
        out.append((name, cur))
    return out


def compile_orders(sets):

    src = "void use(...);\n"
    for k, names in enumerate(sets):
        body = " ".join(f"int {n};" for n in names)
        body += " " + " ".join(f"{n} = {i + 1};" for i, n in enumerate(names))
        body += " use(" + ", ".join(names) + ");"
        src += f"void probe{k}()\n{{\n    {body}\n}}\n"
    with tempfile.TemporaryDirectory(dir=os.path.join(build.ROOT, "build")) as tmp:
        path = os.path.join(tmp, "probe.cpp")
        obj = os.path.join(tmp, "probe.obj")
        with open(path, "w") as f:
            f.write(src)
        subprocess.run(["wine", build.BUILD["bin"] + r"\cl.exe", "/nologo", "/c", *build.CFLAGS,
                        "/Fo" + build.win(os.path.relpath(obj, build.ROOT)),
                        build.win(os.path.relpath(path, build.ROOT))],
                       env=build.ENV, cwd=build.ROOT, capture_output=True, timeout=build.TIMEOUT, check=True)
        out = subprocess.run(["objdump", "-d", "-M", "intel", obj], capture_output=True, text=True).stdout
    orders = []
    for k, names in enumerate(sets):
        func = re.search(rf"<\?probe{k}@@YAXXZ>:\n(.*?)(?:\n\n|\Z)", out, re.S).group(1)
        slots = {}
        for m in re.finditer(r"mov\s+DWORD PTR \[ebp-0x([0-9a-f]+)\],0x([0-9a-f]+)", func):
            slots[names[int(m.group(2), 16) - 1]] = int(m.group(1), 16)
        orders.append(sorted(names, key=lambda n: slots[n]))
    return orders


def pick(pools, decl=None, count=1):

    n = len(pools)
    pos = {r: i for i, r in enumerate(decl or range(n - 1, -1, -1))}

    tie = [k > 0 and pos[k] < pos[k - 1] for k in range(n)]
    cands = []
    for pool in pools:
        by = [[] for _ in range(16)]
        for cost, name in enumerate(dict.fromkeys(pool)):
            by[bucket(name)].append((cost, name))
        cands.append(by)
    inf = float("inf")

    def first(k, b):
        return 0 if b is None else b if tie[k] else b + 1


    best = [[0] * 17 for _ in range(n + 1)]
    for k in range(n - 1, -1, -1):
        for b in range(17):
            best[k][b] = min((cands[k][c][0][0] + best[k + 1][c]
                              for c in range(first(k, None if b == 16 else b), 16) if cands[k][c]), default=inf)
    if best[0][16] == inf:
        return

    heap = [(best[0][16], 0, ())]
    while heap and count:
        _, cost, picked = heapq.heappop(heap)
        k = len(picked)
        if k == n:
            yield cost, [name for name, _ in picked]
            count -= 1
            continue
        used = {name for name, _ in picked}
        for c in range(first(k, picked[-1][1] if picked else None), 16):
            if best[k + 1][c] == inf:
                continue
            for ccost, name in cands[k][c]:
                if name not in used:
                    heapq.heappush(heap, (cost + ccost + best[k + 1][c], cost + ccost, picked + ((name, c),)))


KEYWORDS = set("""asm auto bool break case catch char class const const_cast continue default
delete do double dynamic_cast else enum explicit export extern false float for friend goto if
inline int long mutable namespace new operator private protected public register
reinterpret_cast return short signed sizeof static static_cast struct switch template this throw
true try typedef typeid typename union unsigned using virtual void volatile wchar_t while use
main""".split())


def random_names(rnd):
    first = string.ascii_letters + "_"
    rest = first + string.digits
    names = set()
    count = rnd.choice([1, 2, 5, 10, 20, 40, 80, 150])
    length = rnd.choice([3, 6, 12, 24])
    while len(names) < count:
        n = rnd.choice(first) + "".join(rnd.choice(rest) for _ in range(rnd.randrange(length)))
        if n not in KEYWORDS and not n.startswith(("__", "probe")):
            names.add(n)
    names = list(names)
    rnd.shuffle(names)
    return names


def check(total, batch=100):
    rnd = random.Random(total)
    agree = 0
    for start in range(0, total, batch):
        sets = [random_names(rnd) for _ in range(min(batch, total - start))]
        for names, compiled in zip(sets, compile_orders(sets)):
            if compiled == order(names):
                agree += 1
            else:
                print("differs:", " ".join(names))
    print(f"{agree}/{total} sets agree")


def main():
    args = sys.argv[1:]
    if args[0] == "--check":
        return check(int(args[1]) if len(args) > 1 else 1000)
    if args[0] == "--compile":
        for i, name in enumerate(compile_orders([args[1:]])[0]):
            print(f"[ebp-{(i + 1) * 4:#x}] {name}")
        return
    if args[0] == "--pick":
        opts = dict(zip(args[2::2], args[3::2]))
        suffixes = [""] + opts["-s"].split(",") if "-s" in opts else [""]
        pools = [[b + s for b in line.split() for s in suffixes]
                 for line in open(args[1]).read().splitlines() if line.strip()]
        decl = [int(x) for x in opts["-d"].split(",")] if "-d" in opts else None
        found = False
        for cost, names in pick(pools, decl, int(opts.get("-n", 1))):
            print(cost, " ".join(names))
            found = True
        if not found:
            print("no set fits; the buckets each slot's pool reaches:")
            for k, pool in enumerate(pools):
                print(k, " ".join(f"{b:x}" for b in sorted({bucket(n) for n in pool})))
        return
    for name, offset in layout(args):
        print(f"[ebp-{offset:#x}] {name}")


if __name__ == "__main__":
    main()
