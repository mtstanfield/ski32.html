#!/usr/bin/env python3
"""Generate res/bmp_NNN.bmp (24-bit, bottom-up) from the T6-extracted
web/assets/sprites/bmp_NNN.png files, for resources.rc / windres.

The PNGs are the palette-corrected RGB extraction (see
harness/extract_resources.py — Windows RGBQUADs are b,g,r in the file).
Run after extract_resources.py; idempotent.
"""
import pathlib
import struct

SPR = pathlib.Path("web/assets/sprites")
OUT = pathlib.Path("res")
OUT.mkdir(exist_ok=True)
n = 0
for p in sorted(SPR.glob("bmp_*.png")):
    im = __import__("PIL.Image", fromlist=["Image"]).open(p).convert("RGB")
    w, h = im.size
    # 24bpp DIB pixels are (B,G,R); PIL RGB tobytes is (R,G,B) — emit BGR.
    px = im.tobytes("raw", "BGR")  # BGR, top-down rows
    stride = (w * 3 + 3) & ~3
    rows = bytearray()
    for y in range(h - 1, -1, -1):  # bottom-up
        row = px[y * w * 3:(y + 1) * w * 3]
        rows += row + b"\x00" * (stride - len(row))
    datasz = stride * h
    fh = b"BM" + struct.pack("<IHHI", 14 + 40 + datasz, 0, 0, 14 + 40)
    ih = struct.pack("<IiiHHIIiiII", 40, w, h, 1, 24, 0, datasz,
                     2835, 2835, 0, 0)  # 72 DPI
    (OUT / p.name.replace(".png", ".bmp")).write_bytes(fh + ih + bytes(rows))
    n += 1
print(f"OK: {n} BMPs in res/")
