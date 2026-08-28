#!/usr/bin/env python3
"""Aligned differential driver for the ORIGINAL ski32 (root, /proc/mem).

Starts immediately after the child PID is known. Detects the window
(xdotool). Two fire modes for KP_1:
  - <arg2> = 0xHEX: fire when c16c == target EXACTLY (the deterministic
    seed orbit value; immune to observation-start offset — preferred).
  - <arg2> = N: count distinct c16c changes from the first observation
    and fire at the Nth (menu calls rand() once per tick, so changes ~=
    ticks; only valid if observation starts at process start).
Captures one state sample per c698 value to <out> (dedup by player y in
analysis), each with the full in-list entity dump (same fingerprint fields
as the rebuild's ski_dbg_state_dump SKI_DBG_FULL) so the two sides diff
cleanly. Set SKI_F2_SECS=<s> to send F2 <s> seconds after KP_1 (restart
parity check).

Usage: python3 tools/orig_tick.py <pid> <0xTARGET|N> <out> <secs>
"""
import sys, struct, time, os, subprocess

pid = sys.argv[1]
_arg = sys.argv[2]
if not _arg.isdigit():
    target = int(_arg, 16)
    N = None
else:
    N = int(_arg)
    target = None
out = sys.argv[3]
secs = float(sys.argv[4])
f2_after = float(os.environ.get("SKI_F2_SECS", "0") or 0)

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
f2_sent = False
kp1_time = None
end = time.time() + secs
f = open(out, "w")
while time.time() < end:
    try:
        if wid is None:
            wid = find_window()
        c16c = u32(0x40c16c)
        c698 = u32(0x40c698)
        # fire KP_1 at the exact c16c orbit value (preferred) or at the Nth
        # distinct-c16c change (menu calls rand() once per tick)
        if not sent and wid is not None:
            if target is not None:
                fire = (c16c == target)
            else:
                if last_r is not None and c16c != last_r:
                    count += 1
                fire = (count >= N)
            if fire:
                subprocess.run(["xdotool", "key", "--window", wid, "KP_1"], env=env)
                sent = True
                kp1_time = time.time()
        last_r = c16c
        # optional F2 (restart parity check) f2_after seconds after KP_1
        if f2_after and sent and not f2_sent and time.time() - kp1_time >= f2_after:
            subprocess.run(["xdotool", "key", "--window", wid, "F2"], env=env)
            f2_sent = True
            f.write(f"# F2 sent at wall+{f2_after}s post-KP_1\n")
            f.flush()
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
            # full in-list entity dump (same fingerprint fields as the rebuild's
            # ski_dbg_state_dump SKI_DBG_FULL) so the two sides diff cleanly.
            idx = 0; e = u32(0x40c618)
            while e and idx < 400:
                gn = u32(e + 0x08); pt = u32(e + 0x04); ds = u32(e + 0x0c)
                col = u16(e + 0x10); ty = u32(e + 0x18); fr2 = u16(e + 0x1c)
                xx = i16(e + 0x40); yy = i16(e + 0x42); md2 = u16(e + 0x44)
                st2 = i16(e + 0x46); sp2 = i16(e + 0x48); fl2 = u32(e + 0x4c)
                r0 = struct.unpack("<i", rd(e + 0x20, 4))[0]
                r1 = struct.unpack("<i", rd(e + 0x24, 4))[0]
                r2 = struct.unpack("<i", rd(e + 0x28, 4))[0]
                r3 = struct.unpack("<i", rd(e + 0x2c, 4))[0]
                f.write(f"  #{idx:3d} {e:08x} gn={gn:08x} pt={pt:08x} "
                        f"desc={ds:08x} col={col} type={ty} fr={fr2:04x} "
                        f"x={xx} y={yy} mode={md2} steer={st2} speed={sp2} "
                        f"fl={fl2:08x} rect=[{r0},{r1},{r2},{r3}]\n")
                e = u32(e); idx += 1
            f.flush()
    except Exception:
        break
    time.sleep(0.004)
f.close()
mode = f"target={target:08x}" if target is not None else f"changes={count}/{N}"
print(f"wid={wid} {mode} sent_kp1={sent} f2_sent={f2_sent}", flush=True)
