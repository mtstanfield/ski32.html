#!/usr/bin/env python3
"""Aligned differential driver for the ORIGINAL ski32 (root, /proc/mem).

Starts immediately after the child PID is known. Detects the window
(xdotool). During the menu the game calls rand() exactly ONCE per tick
(the spawn picker), so distinct c16c values == ticks: count c16c changes
from the start and send KP_1 after exactly N ticks (same reference as the
rebuild's Nth in-binary dump line). Captures one state sample per c698
value to <out> (dedup by player y in analysis).

Usage: sudo python3 tools/orig_tick.py <pid> <N> <out> <secs>
"""
import sys, struct, time, os, subprocess

pid = sys.argv[1]
N = int(sys.argv[2])
out = sys.argv[3]
secs = float(sys.argv[4])

mem = open(f"/proc/{pid}/mem", "rb")
def rd(a, n):
    mem.seek(a)
    return mem.read(n)
def u32(a):
    return struct.unpack("<I", rd(a, 4))[0]
def i16(a):
    return struct.unpack("<h", rd(a, 2))[0]
def u16(a):
    return struct.unpack("<H", rd(a, 2))[0]

env = dict(os.environ, DISPLAY=":99")
def find_window():
    try:
        r = subprocess.run(["xdotool", "search", "--name", "SkiFree"],
                           capture_output=True, text=True, env=env, timeout=2)
        w = r.stdout.split()
        return w[0] if w else None
    except Exception:
        return None

wid = None
last_r = None
last_t = None
count = 0
sent = False
end = time.time() + secs
f = open(out, "w")
while time.time() < end:
    try:
        if wid is None:
            wid = find_window()
        c16c = u32(0x40c16c)
        c698 = u32(0x40c698)
        # count menu ticks as distinct c16c values (one rand() call per tick)
        if not sent and c16c != last_r and last_r is not None:
            count += 1
            if count >= N and wid is not None:
                subprocess.run(["xdotool", "key", "--window", wid, "KP_1"], env=env)
                sent = True
        last_r = c16c
        # capture one sample per c698 value
        if last_t is None or c698 != last_t:
            last_t = c698
            pl = u32(0x40c72c)
            if pl and 0x300000 <= pl < 0x7ff00000:
                px = i16(pl + 0x40); py = i16(pl + 0x42); st = i16(pl + 0x46)
                sp = i16(pl + 0x48); md = u16(pl + 0x44); fr = u16(pl + 0x1c)
                fl = u32(pl + 0x4c)
            else:
                px = py = st = sp = -1; md = fr = fl = 0
            n = 0; e = u32(0x40c618)
            while e and n < 400:
                n += 1; e = u32(e)
            cy = u16(0x40c5f2); cx = u16(0x40c640)
            f.write(f"py={py} t={c698} r={c16c:08x} px={px} st={st} sp={sp} "
                    f"md={md} fl={fl:08x} fr={fr} n={n} cy={cy} cx={cx}\n")
            f.flush()
    except Exception:
        break
    time.sleep(0.004)
f.close()
print(f"wid={wid} menu_ticks={count} sent_kp1={sent}", flush=True)
