#!/bin/sh



set -e
cd "$(dirname "$0")/../.."
project() { python3 tools/project.py "$@"; }
GHIDRA=$(project ghidra)
NAME=$(project get ghidra.project)
ORIGINAL=$(project get game.original)
FUNCTIONS=$(project get ghidra.functions)
mkdir -p build/ghidra
"$GHIDRA/support/analyzeHeadless" build/ghidra "$NAME" \
	-import "$ORIGINAL" -overwrite \
	-scriptPath tools/ghidra \
	-postScript ExportFunctions.java "$PWD/$FUNCTIONS" \
	> build/ghidra/analyze.log 2>&1
grep REPORT build/ghidra/analyze.log
