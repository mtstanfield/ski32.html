#!/usr/bin/env python3
"""T14: build the instrumented original ski32.exe.

Applies, to a COPY of original/ski32.exe (file offset == VA - 0x400000):
  1. the T8 seed-freeze patch (from harness/seed.json)
  2. the T14 tick hook: jmp at 0x1000 -> stub at 0x96ac, stub bytes,
     and the .text VirtualSize grow (so the raw stub tail is loaded).
     The stub is self-contained: it VirtualAlloc's its own page at the
     first tick; the only .data footprint is the 8-byte home slots at
     0x40c284/0x40c288 (verified dead block; must be 0 in file).
  3. pause-auto activation bypass: 0x5a17 "je" -> 2 x nop, so
     game_pause_auto resumes on !c770 (minimize) alone. Under Xvfb the
     wineserver foreground state is racy and a fresh window
     intermittently never gets WM_ACTIVATE(1), leaving the game paused
     forever; this makes the instrumented original activation-free
     (the SKI_HARNESS rebuild carries the matching #if).

Usage: stub_patch.py OUT_PATH
"""
import json, pathlib, struct, sys

cfg = json.loads(pathlib.Path("harness/seed.json").read_text())
src = pathlib.Path("original/ski32.exe").read_bytes()
stub_path = pathlib.Path("harness/stub/orig_stub.bin")
if not stub_path.exists():  # regenerate from the committed source
    import subprocess
    subprocess.run(["nasm", "-f", "bin", "-o", str(stub_path),
                    "harness/stub/orig_stub.asm"], check=True)
stub = stub_path.read_bytes()
assert len(stub) < 0xA000 - 0x96AC, "stub exceeds .text tail"

# ---- 1. seed patch (same contract as seed_patch.py) ----
o = cfg["original_patch"]
off = int(o["addr"], 16) - 0x400000
before = bytes.fromhex(o["bytes_before"].replace(" ", ""))
after = bytes.fromhex(o["bytes_after"].replace(" ", ""))
assert len(before) == len(after)
assert src[off:off + len(before)] == before, f"seed patch mismatch at {off:#x}"
src = src[:off] + after + src[off + len(before):]

# ---- 2a. jmp patch at ski_tick entry (0x401000) ----
JMP_AT, STUB_AT, BODY_AT = 0x1000, 0x96AC, 0x1006
assert src[JMP_AT:JMP_AT + 6] == bytes.fromhex("ff15c0a04000"), \
    f"patch site changed: {src[JMP_AT:JMP_AT+6].hex()}"
rel32 = (0x400000 + STUB_AT) - ((0x400000 + JMP_AT) + 5)
assert -0x80000000 <= rel32 < 0x80000000
src = src[:JMP_AT] + b"\xe9" + struct.pack("<i", rel32) + b"\x90" + src[JMP_AT + 6:]

# ---- 2a'. pause-auto activation bypass (VA 0x405a17 -> file 0x5a17) ----
# game_pause_auto (0x405a10): mov c694 -> test -> JE 0x405a31 (pause) ->
#   mov c770 -> test -> jne pause -> c67c=1, resume.
# Patch the c694 JE (74 18) to 2 x nop: resume becomes !c770 (minimize)
# only. Under Xvfb the wineserver foreground state is racy and a fresh
# window intermittently NEVER receives WM_ACTIVATE(1) -> the unpatched
# game would sit paused forever (0 frames) on ~half the launches.
# Both diff sides go activation-free (rebuild: #if SKI_HARNESS in
# ski_pause_auto). Deactivate no longer pauses; MINIMIZE still does.
cur = src[0x5A17:0x5A19]
assert cur == bytes.fromhex("7418"), f"pause je not 74 18: {cur.hex()}"
src = src[:0x5A17] + b"\x90\x90" + src[0x5A19:]

# ---- 2b. stub code (.text raw tail, must be zeroed) ----
tail = src[STUB_AT:0xA000]
assert tail == bytes(len(tail)), "stub region not zeroed"
src = src[:STUB_AT] + stub + b"\x90" * (0xA000 - STUB_AT - len(stub)) + src[0xA000:]

# ---- 2b'. grow .text VirtualSize so the raw tail is actually loaded ----
# .text: rptr 0x1000, vsz 0x86ac, rsz 0x9000. The loader commits vsz and
# loads min(vsz, rsz) bytes: raw bytes beyond vsz (the stub zone) are
# zeroed in memory unless vsz is raised to rsz. Section header:
# secbase = pe(0xd0) + 4 + 20 + optsize(0xe0) = 0x174.
pe, optsize = 0xD0, struct.unpack_from("<H", src, 0xD4 + 16)[0]
secbase = pe + 4 + 20 + optsize
tvsize, tvaddr, trsize, trptr = struct.unpack_from("<IIII", src, secbase + 8)
assert (tvsize, tvaddr, trsize, trptr) == (0x86AC, 0x1000, 0x9000, 0x1000), \
    f".text section header changed: {(tvsize, tvaddr, trsize, trptr)}"
src = src[:secbase + 8] + struct.pack("<I", trsize) + src[secbase + 12:]

# ---- 2c. home slots (.data 0x40c284..0x40c28c, 8 B, must be zero in file) ----
# The stub keeps its VirtualAlloc'd page base here (two redundant copies).
# Evidence (see orig_stub.asm HOME1/HOME2 comment): the 0x40c200 .data page
# has zero direct .text references; 0x40c284..0x40c383 (256 B) verified
# all-zero at runtime including mid-descent; neighbors 0x40c280/0x40c384
# are live. (A home at 0x40ca00 failed: inside the game's runtime-built
# letter tables 0x40c9e1..0x40ca25. A fixed VirtualAlloc base failed:
# 0x21000000 is MEM_RESERVE wine arena in the game process.)
assert src[0xC284:0xC28C] == bytes(8), \
    f"home slots 0xc284..0xc28c not zero: {src[0xC284:0xC28C].hex()}"

out = pathlib.Path(sys.argv[1])
out.write_bytes(src)
print(f"wrote {out} ({len(src)} bytes); stub {len(stub)} B at 0x{STUB_AT:x}, "
      f"jmp rel32={rel32:#x}; pause je bypassed @0x5a17; homes @0x40c284/0x40c288")
