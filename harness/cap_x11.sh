#!/usr/bin/env bash
# Manual X11 capture stream of the original main window (cap_%07d_main.png).
# Wall-clock cadence (~20 fps) — for visual/manual comparison only. The
# deterministic tick-locked stream comes from the T14 injected stub
# (frame_%06d_main.ppm at the tick boundary), NOT from this script.
# Requires an already-running Xvfb ($DISPLAY, default :99) and a running
# ski32.exe instance. Usage: bash harness/cap_x11.sh OUT_DIR [SECONDS]
set -e
OUT=${1:?usage: cap_x11.sh OUT_DIR [SECONDS]}
DUR=${2:-15}
export DISPLAY=${SKI_DISPLAY:-:99}
mkdir -p "$OUT"
WID=""
for i in $(seq 1 30); do
  for w in $(xdotool search --name "SkiFree" 2>/dev/null || true); do
    geo=$(xdotool getwindowgeometry --shell "$w" 2>/dev/null || true)
    # main window: 760x734 client under bare Xvfb (evidence/m0-geometry.txt)
    if echo "$geo" | grep -q "WIDTH=760" && echo "$geo" | grep -q "HEIGHT=734"; then
      WID=$w; break
    fi
  done
  [ -n "$WID" ] && break
  sleep 0.5
done
[ -z "$WID" ] && { echo "main window 760x734 not found" >&2; exit 1; }
echo "capturing window $WID -> $OUT (up to ${DUR}s)" >&2
END=$(( $(date +%s) + DUR ))
n=0
while [ "$(date +%s)" -lt "$END" ]; do
  import -window "$WID" "$OUT/cap_$(printf '%07d' "$n")_main.png" 2>/dev/null || true
  n=$((n + 1))
  sleep 0.05
done
echo "wrote $n frames to $OUT" >&2
