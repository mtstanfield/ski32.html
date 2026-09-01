#!/usr/bin/env bash
# run_scenario_wasm.sh — the WASM side of the T21 frame diff (M3 exit).
#
# The rebuild frames must already exist (harness/run_scenario.sh, wine):
#   harness/frames/rebuild_<SCENARIO>/frame_%06d_main.ppm
# This produces the WASM side (harness/frames/wasm_<SCENARIO>/
# frame_%06d_main.png, harness/wasm_capture.mjs over CDP) and runs the
# house diff (tol=0 + the 620,0,760,60 panel mask) with the REBUILD
# PPMs as the stable reference. Frame index k on both sides is the same
# game state (the seal sits at the rebuild's capture point —
# src/ski_core.c ski_harness_frame), so the comparison is shift 0.
# s08: both sides stall at the F3 pause (~301 frames); diff.py compares
# the overlap, which is the whole of the rebuild side.
# Usage: bash harness/run_scenario_wasm.sh SCENARIO
set -u
cd "$(dirname "$0")/.."
S=${1:?usage: run_scenario_wasm.sh SCENARIO}
PY=harness/.venv/bin/python
SCJSON=harness/scenarios/$S.json
[ -f "$SCJSON" ] || { echo "no scenario $SCJSON" >&2; exit 2; }
REB=harness/frames/rebuild_$S
ls "$REB"/frame_000000_main.ppm >/dev/null 2>&1 || {
  echo "no rebuild frames in $REB (run harness/run_scenario.sh first)" >&2;
  exit 2; }

# The mjs serves web/ — the HARNESS=ON diff build is the artifact there
# (web/ski.js is a gitignored scratch slot; build-web, the HARNESS=OFF
# boot build, stays intact).
cmake --build build-wasmdiff -j >/dev/null || exit 2
cp build-wasmdiff/ski.js build-wasmdiff/ski.wasm web/

TICKS=$("$PY" -c "import json,sys; print(json.load(open(sys.argv[1]))['ticks'])" "$SCJSON")
OUT=harness/frames/wasm_$S
rm -rf "$OUT"; mkdir -p "$OUT"
"$PY" harness/gen_input.py "$SCJSON" "$OUT/ski_in.bin" || exit 2
node harness/wasm_capture.mjs "$S" "$OUT" || exit 2

"$PY" harness/diff.py "$REB" "$OUT" \
  --report evidence/t21-wasm-diff.txt \
  --viz /tmp/t21-viz-$S
RC=$?
echo "wasm scenario $S done (diff rc=$RC)"
exit $RC
