#!/usr/bin/env python3
"""Per-tick yeti gate-desc poller (strict c698-change dedup).

Emits one line-set whenever c698's value changes. The game ticks at
~40ms/tick and wine GetTickCount has ms resolution, so each tick yields
exactly one line-set; same-tick re-samples (c698 unchanged) are skipped.
(Do NOT gate on a real-time delta >= 40ms instead: the wine stub's
jittered clock drops ticks whose real-time delta is < 40ms.)

Reads the instrumented original via /proc/<pid>/mem. The gate desc pool
is c720 = 0x40c720 (linked list of 36-byte descs; first at +0x00, end at
+0x04 of the 0x40c720 slot); only yetis (type 5-8) are printed. Desc
fields: +0x0c type, +0x10 frame, +0x14 x, +0x16 y, +0x18 z, +0x1a vx,
+0x1c vy, +0x1e fdelta, +0x20 timestamp (c698-based).

Same fields as the rebuild's SKI_DBG_YETI block (src/ski_core.c):
  t=<c698> pl=<c72c!=0> Y<idx> type= fr= ts= dt= x= y= z= fd= vx= vy= ent=

Usage: yeti_poll.py <win32-child-pid> [seconds]
"""
import struct
import sys
import time

pid = int(sys.argv[1])
secs = float(sys.argv[2]) if len(sys.argv) > 2 else 300.0
mem = open(f"/proc/{pid}/mem", "rb")


def rd(a, n):
    mem.seek(a)
    d = mem.read(n)
    if len(d) != n:
        raise SystemExit(f"read failed at {a:#x}")
    return d


def u16(a):
    return struct.unpack("<H", rd(a, 2))[0]


def i16(a):
    return struct.unpack("<h", rd(a, 2))[0]


def u32(a):
    return struct.unpack("<I", rd(a, 4))[0]


def c720_range():
    first = u32(0x40c720)
    end = u32(0x40c724)
    if not first or first >= end:
        return []
    out = []
    d = first
    i = 0
    while d < end and i < 64:
        t = u32(d + 0x0C) & 0xFF
        if 5 <= t <= 8:
            out.append((i, t, d))
        d += 36
        i += 1
    return out


end = time.time() + secs
last_t = None
while time.time() < end:
    try:
        c698 = u32(0x40c698)
        if last_t is not None and c698 == last_t:
            time.sleep(0.004)
            continue
        if last_t is None and c698 == 0:
            time.sleep(0.004)
            continue
        last_t = c698
        pl = u32(0x40c72c) != 0
        descs = c720_range()
        for i, t, d in descs:
            fr = u16(d + 0x10)
            ts = u32(d + 0x20)
            x = i16(d + 0x14)
            y = i16(d + 0x16)
            z = i16(d + 0x18)
            vx = i16(d + 0x1A)
            vy = i16(d + 0x1C)
            fd = i16(d + 0x1E)
            ent = u32(d) != 0
            print(
                f"t={c698} pl={1 if pl else 0} Y{i:02d} type={t} "
                f"fr={fr:02x} ts={ts} dt={c698 - ts} x={x} y={y} z={z} "
                f"fd={fd} vx={vx} vy={vy} ent={'y' if ent else 'n'}",
                flush=True,
            )
        time.sleep(0.004)
    except SystemExit:
        break
    except Exception:
        break
