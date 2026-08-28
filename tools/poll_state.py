#!/usr/bin/env python3
"""Compact per-tick poller for a live wine ski32.exe process.

Reads /proc/PID/mem in a tight loop (no respawn) so it captures every 40 ms
tick. Emits one compact line per sample to stdout (redirect to a file).
Field layout matches the rebuild's in-binary dump (ski_dbg_tickline) so the
two streams can be aligned by player y and diffed.

Usage: poll_state.py <win32-child-pid> [seconds]
  win32-child-pid: the 32-bit ski32.exe process (NOT the wine64 parent).
  seconds: stop after this many seconds (default 240).
"""
import sys
import struct
import time

pid = int(sys.argv[1])
secs = float(sys.argv[2]) if len(sys.argv) > 2 else 240.0
mem = open(f"/proc/{pid}/mem", "rb")


def rd(a, n):
    mem.seek(a)
    d = mem.read(n)
    if len(d) != n:
        raise SystemExit("read failed")
    return d


def u16(a):
    return struct.unpack("<H", rd(a, 2))[0]


def i16(a):
    return struct.unpack("<h", rd(a, 2))[0]


def u32(a):
    return struct.unpack("<I", rd(a, 4))[0]


POOL_LO = 0x00300000
POOL_HI = 0x7FF00000
end = time.time() + secs
last_t = None
while time.time() < end:
    try:
        c698 = u32(0x40c698)
        c16c = u32(0x40c16c)
        pl = u32(0x40c72c)
        if pl and POOL_LO <= pl < POOL_HI:
            px, py = i16(pl + 0x40), i16(pl + 0x42)
            st, sp = i16(pl + 0x46), i16(pl + 0x48)
            md, fr = u16(pl + 0x44), u16(pl + 0x1c)
            fl = u32(pl + 0x4c)
        else:
            px = py = st = sp = -1
            md = fr = fl = 0
        # entity count via next-chain walk (4-byte next per node)
        n = 0
        e = u32(0x40c618)
        while e and n < 400:
            n += 1
            e = u32(e)
        cy = u16(0x40c5f2)
        cx = u16(0x40c640)
        # emit one line per tick: only when c698 advances >= 40 since last emit
        if last_t is None or c698 - last_t >= 40:
            print(
                f"py={py} t={c698} r={c16c:08x} px={px} st={st} sp={sp} "
                f"md={md} fl={fl:08x} fr={fr} n={n} cy={cy} cx={cx}",
                flush=True,
            )
            last_t = c698
    except SystemExit:
        break
    except Exception:
        break
    time.sleep(0.004)
