#!/bin/bash
# T13/T14 differential: instrumented ORIGINAL vs harness REBUILD, same Xvfb.
#
# Original : original/ski32_fixed_seed.exe (seed 0x00123456 @0x404971,
#            T14 parity je->nop @0x405a17) driven by tools/orig_tick.py,
#            which polls /proc/mem, fires KP_1 when the menu c16c orbit
#            hits 0x12e83c69 (the reference run's last menu sample — the
#            keydown landed between it and descent tick 1, r=0x69780dfd)
#            and dumps per-tick state+entity fingerprints to /tmp/orig_r2.log.
# Rebuild  : build-native-harness/ski.exe with SKI_DBG_FULL=1 and
#            SKI_ALIGN_C16C=0x69780dfd (autonomous in-process fire at the
#            same boundary, c16c overwritten to the reference keydown-time
#            value) dumping to /tmp/rebuild_state.
#
# Both sides are focus-independent (je->nop / SKI_HARNESS T14 parity), so
# they coexist on :99 and tick regardless of X focus.
#
# Usage: bash harness/run_diff_pair.sh [seconds]   (default 100)
SECS="${1:-100}"
cd "$(dirname "$0")/.."
pkill -f 'ski32_fixed_seed\.exe' 2>/dev/null
pkill -f 'ski\.exe' 2>/dev/null
sleep 1
rm -f /tmp/rebuild_state /tmp/orig_r2.log

# NOTE: direct `wine foo.exe` is broken in this environment
# ("ShellExecuteEx failed: File not found" — prefix launcher corruption);
# launch through wine cmd.
echo "[1/4] launching original"
DISPLAY=:99 WINEPREFIX="$HOME/.wine-ski" WINEDEBUG=-all \
  wine cmd /c "start z:\\$PWD\\original\\ski32_fixed_seed.exe" >/tmp/ski-orig-boot.log 2>&1 &
OPARENT=$!
OPID=""
for i in $(seq 1 30); do
  OPID=$(pgrep -f 'ski32_fixed_seed\.exe' | head -1)
  [ -n "$OPID" ] && break
  sleep 1
done
[ -z "$OPID" ] && { echo "original child not found"; cat /tmp/ski-orig-boot.log; exit 1; }
echo "  orig pid=$OPID"
echo "[2/4] starting original driver (fires KP_1 at c16c=12e83c69)"
DISPLAY=:99 python3 tools/orig_tick.py "$OPID" 12e83c69 /tmp/orig_r2.log "$SECS" >/tmp/orig_tick_out.log 2>&1 &
OT=$!
sleep 4
NS=$(grep -c '^py=' /tmp/orig_r2.log 2>/dev/null || echo 0)
echo "  orig samples after 4s: $NS (must be >0)"
echo "[3/4] launching rebuild (in-process fire at same boundary)"
DISPLAY=:99 SKI_DBG_FULL=1 SKI_ALIGN_C16C=0x69780dfd WINEDEBUG=-all \
  wine cmd /c "start z:\\$PWD\\build-native-harness\\ski.exe" >/tmp/ski-reb-boot.log 2>&1 &
RPARENT=$!
RPID=""
for i in $(seq 1 30); do
  RPID=$(pgrep -f 'build-native-harness[\\/]ski\.exe' | head -1)
  [ -n "$RPID" ] && break
  sleep 1
done
[ -z "$RPID" ] && { echo "rebuild child not found"; cat /tmp/ski-reb-boot.log; exit 1; }
echo "  rebuild pid=$RPID"
sleep 3
NR=$(grep -c '^py=' /tmp/rebuild_state 2>/dev/null || echo 0)
echo "  rebuild samples +3s: $NR (must be >0)"
echo "[4/4] running $SECS s"
wait "$OT" || true
kill "$OPARENT" "$RPARENT" 2>/dev/null
pkill -f 'ski32_fixed_seed\.exe' 2>/dev/null
pkill -f 'build-native-harness[\\/]ski\.exe' 2>/dev/null
echo "done: orig_r2=$(grep -c '^py=' /tmp/orig_r2.log) rebuild=$(grep -c '^py=' /tmp/rebuild_state)"
cat /tmp/orig_tick_out.log
