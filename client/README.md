# th09

This tree is the native Netplay fork. See [the mod README](../README.md) for
installation, both multiplayer modes and the modern Windows build. The
matching/decompilation figures below describe the upstream reconstruction,
not the modified multiplayer executable.

A from-scratch matching decompilation of 東方花映塚 ~ Phantasmagoria of
Flower View (Touhou 9), built and verified on Linux through Wine.

The goal is C++ source that compiles, with the original toolchain, into a
binary that is byte-for-byte identical to the original executable.

This repository contains no game code or data. You need your own copy of
the game, see [`orig/README.md`](orig/README.md).

## Clean room

This is an independent, clean-room implementation. It is written from
the binary alone: no code, names, struct layouts or function lists are
taken from other decompilations or ports of Touhou games. Names come from
our own analysis or, for library code, from the libraries themselves.

## Status

1105 of the 1119 identified game functions match (98.7%), 262 KB of 297 KB
(88.1%). The remaining 14 are decompiled but not matching yet;
some have the original size, while others still differ in size.
`just compare` lists them as DIFF,
the progress image shows them as not matching, and `hooks.allow_diff`
in `project.toml` lists them, the only functions a commit may leave
differing. `just farm` searches source variants of them for a match.
All currently identified game functions are decompiled. The inventory now
includes five constructors recovered from the global startup entries.

The standalone build is 97.92% identical across the whole file
(14,273 differing bytes). Reaching 100% also requires resolving executable
layout, library code, data and header differences; matching all game
functions alone does not establish whole-file identity. These figures come
from the existing build artifacts; rebuild before using them to verify a
source change.

The remaining function matches, ordered by original function size, are:

| Function | Original bytes | Compiled bytes |
| --- | ---: | ---: |
| `ScreenEffect::OnUpdateShakeRamp` | 336 | 334 |
| `MainMenu::UpdateState13` | 485 | 483 |
| `ReplayManager::AddedCallbackMode1` | 540 | 540 |
| `Player::CheckHitAreas` | 862 | 830 |
| `Player::UpdateShotInput` | 1210 | 1197 |
| `ReplayManager::SaveReplay` | 1238 | 1247 |
| `MainMenu::DrawState11` | 1280 | 1280 |
| `App::UpdateNetPlay` | 1633 | 1609 |
| `GameManager::LoadingThread` | 1689 | 1682 |
| `Player::HandleInput` | 1835 | 1835 |
| `DrawEffectType29` | 2220 | 2220 |
| `Stage::DrawLayer` | 3274 | 3274 |
| `EnemyManager::OnUpdate` | 3881 | 3856 |
| `EclManager::RunEnemy` | 14792 | 14792 |

Start with the smaller functions, comparing each change with
`just compare SYMBOL`, then run the full comparison to catch regressions
in other functions from the same translation unit. Remove a function from
`hooks.allow_diff` only after it matches. Use `just compare-exe` to track
whole-file differences separately. Every section, including the
resources, has its original address and virtual size, and every
compiled contribution has an original location: like the original
linker's ICF, the layout folds identical COMDATs, including EH funclets
that refer to their own function information. The Rich header/toolchain
record still differs. All 216,727 tracked library code bytes match, including
relocation operands; 46 KB of executable code remains outside the function
inventory in `symbols.csv`. All initialized bytes in `.rdata`, `.data`, and
`.data1` match the original too.

Following th08's completion work, the layout reconstruction preserves entire
ordinary SDK/CRT data sections, recovers constant and callback locations
through table references, and restores the global initialization order.
Unreferenced storage still present in the original is defined in source.
The build audits 80 complete tables, their original addresses, and
626 pointer identities.

Ghidra finds 2042 functions. Unlike th06–th08, the game code is
optimized (`/O2`) but keeps its frame pointer (`/Oy-`), and functions
default to `__fastcall` (`/Gr`), as th08's. The game code runs from
`0x401000` to `0x44cb5c`: 1119 identified functions, about 297 KB, counting the
callbacks only reached through function tables, which Ghidra misses.
The last part, from `0x440c80`, is one file per character
(`src/pl00.cpp` to `pl15.cpp`). D3DX8's tables follow, then D3DX8 and
the CRT from `0x47b08c`.

```sh
just analyze   # writes build/functions_150a.csv
```

![Progress](docs/progress.svg)

`just progress` renders it, and `just hooks` makes every commit redraw
it and include it.

## Target

1.50a, released 2005-10-07 as a prerelease that adds netplay, the last
official update:

- 32-bit PE, image base `0x400000`, entry point `0x47d45f`, `.text` is
  0x8cada bytes and starts at `0x401000`, like th06 and th07
- `.data` is 0x441fc bytes in memory but only 0x7e00 on disk. A
  `.data1` section of 0x8e0 bytes follows it.
- Imports `d3d8`, `DSOUND`, `DINPUT8`, `WINMM`, `GDI32`, `USER32`,
  `KERNEL32`, `ADVAPI32` (registry reads) and `ole32`. Only
  `Direct3DCreate8` is imported from D3D, so D3DX8 is statically linked.

## Toolchain

Read from the Rich header (`tools/peinfo.py`), as identified in th10:

| Tool | Build | Objects | Probably |
| --- | --- | --- | --- |
| C++ compiler 13.10 | 3077 | 88 | Game code |
| C compiler 13.10 | 3077 | 138 | MSVC CRT |
| MASM 7.10 | 3077 | 40 | MSVC CRT |
| C++ compiler 13.00 | 9178 | 67 | Static libraries, probably D3DX8 |
| C compiler 13.00 | 9178 | 2 | Static libraries |
| C compiler 13.10 | 4035, 2179 | 2, 2 | Platform SDK libraries |
| C++ compiler 13.10 | 4035 | 2 | Platform SDK libraries |
| Import library 7.10 | 4035, 2179, 2067 | 7, 10, 2 | Import libraries |
| Linker 7.10 | 3077 | | |

Unlike th06–th08, this is Visual C++ .NET 2003 (MSVC 7.1, `cl`
13.10.3077), with the Platform SDK for Windows XP SP2 (import libraries
build 4035) and still the DirectX 8 SDK: D3DX8 is a 13.00 build like
th08's. No object was compiled with `/GL`, unlike th10.

The build uses the original's RTM compiler and linker (`cl`
13.10.3077, `link` 7.10.3077) from `msvc710/bin`, with SP1's headers;
th10's SP1 (`cl` 13.10.6030) generates the same code for every
function. `project.toml` expects, in the Wine prefix:

```
.wine/drive_c/msvc710/  bin/, lib/ (VS .NET 2003 RTM: compiler, CRT)
.wine/drive_c/msvc71/   include/ (VS .NET 2003 SP1)
.wine/drive_c/psdk/     Include/, Lib/ (Platform SDK for XP SP2)
.wine/drive_c/dx8/      include/, lib/ (DirectX 8 SDK)
```

The game code is compiled with `/O2 /Ob0 /Oy- /Gr /GX`: optimized,
frame pointers kept, nothing inlined, and functions `__fastcall` unless
declared otherwise. Methods stay `__thiscall`; functions with only float
arguments look like `__stdcall` ones, since fastcall passes floats on
the stack. D3DX's and math.h's inline helpers, which declare no
convention, follow `/Gr` too, and under `/Ob0` they are emitted out of
line (`D3DXMatrixIdentity` at `0x401300` takes its pointer in `ecx`).
There is no `/GL` (the Rich header lists no LTCG objects), but `/O2`
still gives a file's `static` functions custom register conventions:
`0x401090` takes bytes in `al` and `cl`. `just compare` only sees
external symbols, so such functions can't be compared yet.

## Workflow

Everything runs headless, so it works over SSH. `just` lists the recipes.
`tools/` is a submodule: clone with `--recursive`, or run
`git submodule update --init tools` after cloning.

1. Analyze the binary with Ghidra in headless mode (`just analyze`).
   New functions are added to `config/symbols.csv` with placeholder names
   (`Func43be50`); library functions keep the names Ghidra's function ID
   gives them.
2. Write C++ for a function in `src/`, in the style of 2005-era code.
   `just decompile [ADDR...]` writes Ghidra's pseudocode to
   `build/decomp/<address>.c` to read next to the disassembly.
3. Record the function and every original function and global it
   uses in [`config/symbols.csv`](config/symbols.csv): address, size, our
   name, and the mangled name, which the link error for an unresolved
   external gives.
4. Compare (`just compare`): builds with `cl.exe` under Wine and
   checks each function against the original byte for byte. Calls and
   global addresses are resolved through `symbols.csv` and compared too;
   only references to unnamed data such as string literals are masked.
   `just compare SYMBOL` shows a disassembly diff.
5. Play (`just run`): runs the original game with the new code hooked
   in, headless under Xvfb, to check that the code behaves correctly as
   well as matching. Steps drive it and take screenshots, e.g.
   `just run wait:12 key:Escape key:z wait:2 shot:menu` writes
   `build/shots/menu.png`.
6. Standalone (`just exe`): links the same objects, without the hook
   DLL, into `build/th09.exe`, a game that needs nothing from the
   original executable. It needs the game's data and global objects in
   `src/exe/`. `just exe STEPS` runs it like `just run`. Build it with
   `FEAT=orig_checksum` to play the original's replays: the archive's
   version records and the replays expect the 1.50a exe's size and
   checksum, which a rebuilt exe only has once it is byte-identical.

### Hooking

The decompiled code runs inside the original game. `tools/patch_exe.py` adds
one import, `th09.dll`, to a copy of the executable. The DLL is
built from the same objects that `just compare` checks. On load it
overwrites each decompiled function in the original with a jump to the
new code.

Calls from new code to functions that are not decompiled yet go through
generated thunks (`push address; ret`) to the original. Globals are
absolute symbols at their original addresses. Both are generated from
`symbols.csv`. The DLL has no C runtime of its own: CRT functions belong
in `symbols.csv` too, so the game and the new code share one heap.

## Layout

```
orig/     your game files (not tracked) and their checksums
src/      decompiled source; src/dll/ is the hook DLL's own code,
          src/exe/ the game's data and globals for the standalone exe
config/   symbols.csv: address, size, name and mangled symbol
tools/    build, compare, exe patcher, symbol seeding, Ghidra scripts:
          the decomp-tools repository shared with th06 to th08, as a
          git submodule (just tools-init, tools-pull, tools-push)
project.toml  th09's settings for tools/
build/    outputs; build/game/ is a runnable copy of the patched game
```

## Requirements

- Wine (32-bit), Python 3, [just](https://github.com/casey/just),
  binutils (`objdump`), 7-Zip
- For `just run`: Xvfb, xdotool, ImageMagick, 32-bit Mesa OpenGL
  (`libgl1-mesa-dri:i386`, `libglx-mesa0:i386`) and the `ja_JP.UTF-8`
  locale
- The toolchain in the Wine prefix at `.wine/`, see above (bring your
  own copies)
- Ghidra (for analysis, optional)

## Legal

Touhou Project and 東方花映塚 are by ZUN / Team Shanghai Alice. This is an
unofficial fan project. It contains no assets or code from the original
game, and it is not affiliated with or endorsed by ZUN or Team Shanghai
Alice.

## License

The source in this repository is licensed under the GNU General Public
License, version 3 only (`GPL-3.0-only`), see [`LICENSE`](LICENSE). The
license covers our code, not the original game or its data.
