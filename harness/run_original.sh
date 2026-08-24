#!/usr/bin/env bash
# Launch ski32.exe under a dedicated Xvfb + Wine; print window IDs; keep X alive.
# Usage: bash harness/run_original.sh   (uses display :99, or $SKI_DISPLAY if set)
set -e
export WINEPREFIX=${WINEPREFIX:-$HOME/.wine-ski}
export DISPLAY=${SKI_DISPLAY:-:99}
Xvfb $DISPLAY -screen 0 1024x768x24 >/tmp/xvfb-ski.log 2>&1 &
XPID=$!
sleep 3
wine original/ski32.exe >/tmp/ski-wine.log 2>&1 &
WPID=$!
trap 'kill $XPID $WPID 2>/dev/null || true' EXIT
for i in $(seq 1 45); do
  MAIN=$(DISPLAY=$DISPLAY xdotool search --name "SkiFree" 2>/dev/null | head -1 || true)
  [ -n "$MAIN" ] && break
  sleep 2
done
[ -z "$MAIN" ] && { echo "WINDOW NOT FOUND (see /tmp/ski-wine.log)" >&2; exit 1; }
echo "MAIN=$MAIN"
echo "STATUS=$(DISPLAY=$DISPLAY xdotool search --name 'SkiFree' | tail -1)"
wait $WPID
