#!/usr/bin/env python3
"""Extract all RT_BITMAP sprites + RT_STRING tables from ski32.exe.
Leaf offsets in this PE follow an inconsistent convention, so each candidate
is validated (BITMAPINFOHEADER size 40, sane dims/bpp, exact payload size).

Payload convention found in this PE (differs from a stock BMP):
  [40B BITMAPINFOHEADER][palette: N entries x 4B RGBQUAD][rows, 4B-aligned][<=8B tail]
- biClrUsed is 16 for most 4bpp entries (0 on some); palette is 16 RGBQUAD
  entries either way (the standard VGA 16-color palette).
- Heights are positive => rows are stored bottom-up.
"""
import json, struct, sys, pathlib
from PIL import Image

data = pathlib.Path("original/ski32.exe").read_bytes()
OUT = pathlib.Path("web/assets/sprites"); OUT.mkdir(parents=True, exist_ok=True)
base, RVA_BASE = 0xd000, 0x40d000
HDR = struct.Struct("<IIHHHH"); ENT = struct.Struct("<II")

def read_dir(off):
    _, _, _, _, nn, ni = HDR.unpack_from(data, base + off)
    out, tab = [], base + off + 16
    for _ in range(nn + ni):
        nid, off2 = ENT.unpack_from(data, tab); tab += 8
        if nid & 0x80000000:  # named entry: low 31 bits = offset into .rsrc
            n = struct.unpack_from("<H", data, base + (nid & 0x7FFFFFFF))[0]
            nid = data[base + (nid & 0x7FFFFFFF) + 2:
                        base + (nid & 0x7FFFFFFF) + 2 + n].decode("utf-16-le")
        out.append((nid, off2 & 0x7FFFFFFF, bool(off2 & 0x80000000)))
    return out

def leaf_data(l_off):
    rva, size, cp, res = struct.unpack_from("<IIII", data, base + l_off)
    for off in (rva, base + rva, rva - RVA_BASE + base):
        yield off, size

def valid_bitmap(off, size):
    """Return (w, h, bpp, pal_n, entry_w, tail) or None. h>0 => bottom-up."""
    if not (0 < off < len(data)) or off + size > len(data): return None
    hs, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", data, off)
    if hs != 40 or comp != 0 or planes != 1 or bpp not in (1, 4, 8): return None
    if not (1 <= w <= 512 and 1 <= abs(h) <= 512): return None
    clrused, _ = struct.unpack_from("<II", data, off + 32)
    rowsz = ((w * bpp + 31) // 32) * 4
    core = rowsz * abs(h)
    for tail in (0, 4, 8):
        pal_bytes = size - 40 - core - tail
        if pal_bytes <= 0: continue
        for ew in (4, 3):
            if pal_bytes % ew: continue
            n = pal_bytes // ew
            ok = (clrused and n == clrused) or (not clrused and 1 <= n <= 2 ** bpp)
            if ok: return (w, h, bpp, n, ew, tail)
    return None

entries = []  # (kind, id, off, size)
for tid, t_off, t_isdir in read_dir(0):
    if not t_isdir: continue
    if isinstance(tid, str):
        for _, l_off, _ in read_dir(t_off):
            for off, size in leaf_data(l_off):
                entries.append(("icon", tid, off, size)); break
    elif tid == 2:
        for nid, n_off, _ in read_dir(t_off):
            for _, l_off, _ in read_dir(n_off):
                for off, size in leaf_data(l_off):
                    if valid_bitmap(off, size):
                        entries.append(("bmp", nid, off, size)); break
    elif tid == 6:
        for nid, n_off, _ in read_dir(t_off):
            for _, l_off, _ in read_dir(n_off):
                rva, size, _, _ = struct.unpack_from("<IIII", data, base + l_off)
                for off in (rva, base + rva, rva - RVA_BASE + base):
                    if 0 < off < len(data) - size:
                        entries.append(("str", nid, off, size)); break

res = {"bitmaps": {}, "strings": {}}
for kind, nid, off, size in entries:
    if kind == "str":
        buf = data[off:off + size]
        # not a stock STRINGTABLE: entries are [u8 len][u8 pad][2*len UTF-16LE],
        # terminated by a zero-length entry followed by all-zero padding
        i, s = 0, []
        while i + 2 <= len(buf):
            l = buf[i]
            if l == 0 and all(b == 0 for b in buf[i:]):
                break
            s.append(buf[i + 2:i + 2 + 2 * l].decode("utf-16-le"))
            i += 2 + 2 * l
        res["strings"][nid] = s
        print(f"string group {nid}: {s}")
    elif kind == "bmp":
        w, h, bpp, pal_n, ew, tail = valid_bitmap(off, size)
        pal = [data[off + 40 + k * ew: off + 40 + k * ew + 3] for k in range(pal_n)]
        rowsz = ((w * bpp + 31) // 32) * 4
        raw = data[off + 40 + pal_n * ew: off + 40 + pal_n * ew + rowsz * h]
        img = Image.new("RGB", (w, h)); px = img.load()
        for y in range(h):  # rows stored bottom-up
            row = (h - 1 - y) * rowsz
            for x in range(w):
                if bpp == 8: idx = raw[row + x]
                else:
                    byte = raw[row + (x * bpp) // 8]
                    shift = 8 - bpp - (x * bpp) % 8
                    idx = (byte >> shift) & ((1 << bpp) - 1)
                # Windows RGBQUADs are stored (b,g,r,0) in the file; PIL RGB
                # wants (r,g,b) — swap. (The original renders via GDI which
                # reads the palette natively; this matches the on-screen
                # colors — verified 2026-08-29 against a live Wine run where
                # the chair lifts are red.)
                p3 = pal[idx]
                px[x, y] = (p3[2], p3[1], p3[0])
        p = OUT / f"bmp_{nid:03d}.png"
        img.save(p)
        res["bitmaps"][nid] = {"w": w, "h": h, "bpp": bpp, "file": str(p)}
        print(f"bitmap {nid}: {w}x{h} {bpp}bpp -> {p}")

pathlib.Path("web/assets/resources.json").write_text(json.dumps(res, indent=1))
print(f"OK: {len(res['bitmaps'])} bitmaps, {len(res['strings'])} string groups")
sys.exit(0 if len(res["bitmaps"]) == 89 else 1)
