#!/usr/bin/env bash
# Run one scenario against a ski.exe (rebuild harness build or T14 patched
# original) under the already-running Xvfb, with tick-locked input:
#   1. harness/gen_input.py SCENARIO.json OUT_DIR/ski_in.bin
#   2. launch $BIN via bash from OUT_DIR (bash launch -> Xvfb auto-focus ->
#      boot-paused game auto-resumes; python-Popen launches stay frozen)
#   3. wait for frame_<ticks>_main.ppm (the C tick hook dumps one PPM per
#      game tick; input word k is consumed at game tick k+1)
#   4. kill, count frames, report stall/assert evidence
# Usage: bash harness/run_scenario.sh SCENARIO BIN OUT_DIR
#   SCENARIO  scenario name (harness/scenarios/<SCENARIO>.json) or full path
#   BIN       path to ski.exe
#   OUT_DIR   run dir (ski_in.bin + frame_*.ppm land here)
# Env: SKI_DISPLAY (default :99), SKI_TIMEOUT (seconds; default computed)
set -u
REPO=$(cd "$(dirname "$0")/.." && pwd)
SC=${1:?usage: run_scenario.sh SCENARIO BIN OUT_DIR}
BIN=${2:?missing BIN}
OUT=${3:?missing OUT_DIR}
SCJSON=$SC
[ -f "$SCJSON" ] || SCJSON="$REPO/harness/scenarios/$SC.json"
[ -f "$SCJSON" ] || { echo "no scenario $SCJSON" >&2; exit 2; }
export DISPLAY=${SKI_DISPLAY:-:99}
export WINEPREFIX=${WINEPREFIX:-$HOME/.wine-ski}
PY=$REPO/harness/.venv/bin/python

# wine is launched from OUT_DIR (ski_in.bin is opened from CWD), so BIN
# must be absolute
case "$BIN" in /*) ;; *) BIN="$PWD/$BIN" ;; esac
TICKS=$("$PY" -c "import json,sys; print(json.load(open(sys.argv[1]))['ticks'])" "$SCJSON")
mkdir -p "$OUT"
cd "$OUT"
rm -f frame_*_main.ppm ski_in.bin
"$PY" "$REPO/harness/gen_input.py" "$SCJSON" ski_in.bin >/dev/null || exit 2

# match only wine child processes (cmdline == PE path, ends in ski.exe);
# a bare `pkill -f ski.exe` would match this script's own argv and kill it
SKIPAT='(^|/)[^ ]*ski\.exe$'
pkill -f "$SKIPAT" 2>/dev/null || true
sleep 2

TIMEOUT=${SKI_TIMEOUT:-$(( TICKS * 6 / 10 + 60 ))}  # 60ms/tick budget + 60s
wine "$BIN" >/tmp/ski-scen.log 2>&1 &
WPID=$!

LAST=0; STALL=0; T0=$(date +%s)
while :; do
  NOW=$(date +%s)
  [ $((NOW - T0)) -ge "$TIMEOUT" ] && { echo "TIMEOUT after ${TIMEOUT}s (last frame $LAST/$TICKS)" >&2; break; }
  LATEST=0
  for f in frame_*_main.ppm; do
    [ -e "$f" ] || continue
    idx=${f:6:6}; idx=$((10#$idx))
    [ "$idx" -gt "$LATEST" ] && LATEST=$idx
  done
  if [ "$LATEST" -ge "$TICKS" ]; then echo "done: $LATEST frames" >&2; break; fi
  if [ "$LATEST" -eq "$LAST" ]; then
    STALL=$((STALL + 1))
    if [ "$STALL" -ge 12 ]; then
      if ! kill -0 $WPID 2>/dev/null; then
        echo "PROCESS DIED: game exited at frame $LAST/$TICKS" >&2
      else
        echo "STALL: no new frames for 12s at $LAST/$TICKS (possible assert modal, or tick hook not dumping frames)" >&2
      fi
      break
    fi
  else
    LAST=$LATEST; STALL=0
  fi
  sleep 1
done

kill $WPID 2>/dev/null || true
sleep 1
pkill -f "$SKIPAT" 2>/dev/null || true

N=$(ls frame_*_main.ppm 2>/dev/null | wc -l)
echo "frames: $N/$TICKS in $OUT"
# evidence of a modal assert box: any extra top-level window named like the app
xdotool search --name "ski" 2>/dev/null | while read -r w; do
  echo "window $w: $(xdotool getwindowname $w 2>/dev/null)"
done
grep -iE "assert|exception|abort" /tmp/ski-scen.log || true
[ "$N" -ge "$TICKS" ]
