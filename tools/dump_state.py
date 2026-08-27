#!/usr/bin/env python3
"""Read the 32-bit ski state out of a live (hung) wine64 process via /proc/PID/mem.
Usage: dump_state.py <wine64-pid>
"""
import sys, struct

pid = int(sys.argv[1])
mem = open(f"/proc/{pid}/mem", "rb")

def rd(a, n):
    mem.seek(a)
    d = mem.read(n)
    if len(d) != n:
        raise SystemExit(f"read failed at {a:#x}")
    return d

def u16(a): return struct.unpack("<H", rd(a, 2))[0]
def i16(a): return struct.unpack("<h", rd(a, 2))[0]
def u32(a): return struct.unpack("<I", rd(a, 4))[0]

print(f"tick={u32(0x40c698)} cam_y={u16(0x40c5f2)} view={u16(0x40c5fc)} "
      f"c684={u16(0x40c684)} c68c={u16(0x40c68c)} area={u32(0x40c6fc)} "
      f"desc_n={u32(0x40c702)} c5d8={i16(0x40c5d8)} c714={i16(0x40c714)}")
print(f"player c64c={u32(0x40c64c):#x} c72c={u32(0x40c72c):#x} "
      f"freelist c744={u32(0x40c744):#x}")

POOL = 0x40c648
NSLOT = 100

def ent_line(e, idx):
    return (f"  #{idx:3d} {e:#08x} next={u32(e):#08x} desc={u32(e+0xc):#08x} "
            f"col={u16(e+0x10)} type={u16(e+0x18)} fr={u16(e+0x1c):#04x} "
            f"x={i16(e+0x40)} y={i16(e+0x42)} mode={u16(e+0x44)} "
            f"steer={i16(e+0x46)} speed={i16(e+0x48)} fl={u32(e+0x4c):#08x}")

print("== entity list (c618) ==")
e = u32(0x40c618)
seen = set()
n = 0
while e:
    if e in seen:
        print(f"  CYCLE! node {e:#x} repeats at count {n}")
        break
    if not (POOL <= e < POOL + NSLOT * 80):
        print(f"  !!! {ent_line(e, (e-POOL)//80 if POOL<=e<POOL+NSLOT*80 else -1)} NOT IN POOL")
        n += 1
        if n > 40: break
        e = u32(e)
        continue
    seen.add(e)
    print(ent_line(e, (e - POOL) // 80))
    e = u32(e)
    n += 1
    if n > 300:
        print("  ... >300 nodes (cycle?)")
        break
print(f"  total nodes: {n}")

print("== freelist (c744) ==")
f = u32(0x40c744)
fn = 0
while f:
    if not (POOL <= f < POOL + NSLOT * 80) or fn > NSLOT:
        print(f"  !!! freelist corrupt at {f:#x} (count {fn})")
        break
    f = u32(f)
    fn += 1
print(f"  free slots: {fn} (expect {NSLOT - n})")

def desc_line(d, i):
    return (f"  d[{i}] {d:#08x} ent={u32(d):#08x} col={u16(d+8)} type={u16(d+0xc)} "
            f"fr={u16(d+0x10):#04x} x={i16(d+0x14)} y={i16(d+0x16)} z={i16(d+0x18)} "
            f"vx={i16(d+0x1a)} vy={i16(d+0x1c)} fd={i16(d+0x1e)} ts={u32(d+0x20)}")

for nm, a in [("c630", 0x40c630), ("c5e0", 0x40c5e0), ("c658", 0x40c658),
              ("c738", 0x40c738), ("c720", 0x40c720)]:
    first, end, cur = u32(a), u32(a + 4), u32(a + 8)
    print(f"== {nm}: first={first:#x} end={end:#x} cursor={cur:#x} "
          f"({(end - first) // 36 if first else 0} descs) ==")
    d, i = first, 0
    while d < end and i < 60:
        print(desc_line(d, i))
        d += 36
        i += 1
    if nm == "c630" and first and first != cur:
        print(f"  cursor desc: " + desc_line(cur, "cur").strip())
