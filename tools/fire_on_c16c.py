#!/usr/bin/env python3
"""Tail /tmp/rebuild_state and send KP_1 the moment the summary c16c matches.

Usage: fire_on_c16c.py <target_c16c_hex> <wid>
Reads only new bytes (O(1) per tick), so it never misses the 40ms window.
"""
import sys, time, subprocess

target = sys.argv[1].lower()
wid = sys.argv[2]
path = "/tmp/rebuild_state"

# wait for the file
t0 = time.time()
while not __import__("os").path.exists(path):
    if time.time() - t0 > 60:
        sys.exit("no rebuild_state")
    time.sleep(0.01)

pos = 0
buf = b""
with open(path, "rb") as f:
    while True:
        chunk = f.read(65536)
        if chunk:
            pos += len(chunk)
            buf += chunk
            # process complete lines
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                if line.startswith(b"py="):
                    for kv in line.split():
                        if kv.startswith(b"r="):
                            val = kv[2:].decode().lower()
                            if val == target:
                                subprocess.run(["xdotool", "key", "--window", wid, "KP_1"])
                                print(f"FIRING KP_1 at c16c={target} (pos={pos})", flush=True)
                                sys.exit(0)
        else:
            time.sleep(0.001)
