



export WINEPREFIX := justfile_directory() / ".wine"
export WINEDEBUG := env("WINEDEBUG", "-all")



tools_push_url := "git@github.com:Cirno-Labs/decomp-tools.git"



[default]
default:
    @just --list


build:
    python3 tools/build.py


[positional-arguments]
compare *symbols: build _orig
    python3 tools/compare.py "$@"


game: build _orig
    python3 tools/project.py game


[positional-arguments]
run *steps: game
    python3 tools/play.py "$@"


[positional-arguments]
exe *steps: game
    python3 tools/build.py exe
    python3 tools/project.py standalone
    if [ $# -gt 0 ]; then python3 tools/play.py --standalone "$@"; fi


check-exe: exe
    python3 tools/project.py check-exe


[positional-arguments]
compare-exe *args: _orig
    python3 tools/compare_exe.py "$@"


[positional-arguments]
replay *args: _orig
    python3 tools/replay/replay.py check "$@"


psdk-libs zip:
    python3 tools/psdk_libs.py {{ quote(zip) }}


analyze: _orig
    tools/ghidra/analyze.sh
    python3 tools/seed_symbols.py


decompile *addrs:
    tools/ghidra/decompile.sh {{ addrs }}


[positional-arguments]
permute name *args: _orig
    python3 tools/permute.py "$@"


[positional-arguments]
farm *args: _orig
    python3 tools/farm.py "$@"


progress: build _orig
    python3 tools/render_progress.py


hooks:
    git config core.hooksPath tools/hooks
    git -C tools config core.hooksPath hooks


clean:
    python3 tools/project.py clean


_orig:
    python3 tools/project.py orig




tools-init:
    git submodule update --init tools
    git -C tools switch -q -C main "$(git rev-parse :tools)"


tools-pull:
    git -C tools switch -q main
    git -C tools pull -q --ff-only
    git -C tools log -1 --oneline


tools-push:
    git -C tools push {{ tools_push_url }} main
