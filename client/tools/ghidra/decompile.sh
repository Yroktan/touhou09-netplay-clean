#!/bin/sh



set -e
cd "$(dirname "$0")/../.."
project() { python3 tools/project.py "$@"; }
GHIDRA=$(project ghidra)
NAME=$(project get ghidra.project)
PROGRAM=$(basename "$(project get game.original)")
"$GHIDRA/support/analyzeHeadless" build/ghidra "$NAME" \
	-process "$PROGRAM" -noanalysis -readOnly \
	-scriptPath tools/ghidra \
	-postScript Decompile.java "$PWD/build/decomp" "$@" \
	> build/ghidra/decompile.log 2>&1
grep REPORT build/ghidra/decompile.log
