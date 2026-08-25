#!/usr/bin/env python3
"""Apply the seed-freeze hex patch to a COPY of ski32.exe.
This PE maps sections 1:1 (VMA - ImageBase == file offset for .text/.rdata/.data/.rsrc:
.text 0x401000->0x1000, .rdata 0x40a000->0xa000, .data 0x40c000->0xc000, .rsrc 0x40d000->0xd000).
Usage: seed_patch.py OUT_PATH
"""
import json, pathlib, sys
cfg = json.loads(pathlib.Path("harness/seed.json").read_text())
src = pathlib.Path("original/ski32.exe").read_bytes()
o = cfg["original_patch"]
off = int(o["addr"], 16) - 0x400000
before = bytes.fromhex(o["bytes_before"].replace(" ", ""))
after = bytes.fromhex(o["bytes_after"].replace(" ", ""))
assert len(before) == len(after), "patch length mismatch"
assert src[off:off + len(before)] == before, \
    f"byte mismatch at {off:#x}: got {src[off:off+len(before)].hex()}"
pathlib.Path(sys.argv[1]).write_bytes(src[:off] + after + src[off + len(before):])
print(f"patched {sys.argv[1]} at file offset {off:#x}")
