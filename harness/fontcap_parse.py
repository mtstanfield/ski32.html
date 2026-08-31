#!/usr/bin/env python3
"""fontcap_parse.py — turn the wine font capture into the shim + web assets.

Reads (default dir: harness/fontcap/, override as argv[1]):
  fontcap.txt  "fontcap 1" / face / sheet W H / cell M CW CH / metrics
               (17 TEXTMETRICS ints) / "extent CODE ADV" lines
  fontcap.bmp  24bpp sheet: an 8-column grid of CW(=21) x CH(=12) slots,
               one per ASCII code 32..126 row-major, each char drawn at its
               slot's pen origin (margin M in from the slot's left).

Per-char crop = the STRIP: columns [pen-1 .. pen+7] (9 px: the glyph's
subpixel-AA fringe reaches 1px LEFT of the pen — verified against the
reference panel, where a following 'm' tints the previous char's last
column B=253 — and the ink ends at/inside the advance + 1), rows 0..CH-1
(full text row height). The wide 21px slots (harness/fontcap/fontcap.c)
guarantee no neighbor ink in the strip's guard columns; the parser asserts
this per char, so a future re-capture that violates the layout fails loud.

The shim draws each char's strip at (x + STRIP_L) in left-to-right order
with GDI's subpixel-AA blend (dst_c = dst_c * cap_c / 255), so a later
glyph's strip overwrites the previous glyph's overhanging fringe exactly
like the reference's X11 rendering.

Writes:
  web/assets/font.json  per-char {"adv", "px": base64(RGB rows)} + metrics
  shim/font.inc         the same data as static const C arrays (ShimFont g_font),
                        included by shim/text.c only.

Run from the repo root:  harness/.venv/bin/python harness/fontcap_parse.py
"""
import base64
import json
import sys
from pathlib import Path

from PIL import Image

MAGIC = "fontcap 1"
FIRST, LAST = 32, 126
STRIP_L = -1        # strip's first column, relative to the pen
STRIP_R = 7         # strip's last column, inclusive
STRIP_W = STRIP_R - STRIP_L + 1
ROOT = Path(__file__).resolve().parent.parent


def parse_txt(path):
    meta = {"face": "", "sheet": None, "cell": None, "metrics": None, "adv": {}}
    for line in path.read_text().splitlines():
        t = line.split()
        if not t:
            continue
        if t[0] == "fontcap":
            assert t == MAGIC.split(), f"bad magic: {line!r}"
        elif t[0] == "face":
            # "face <name...> lfHeight lfWidth lfWeight" (name may be empty)
            assert len(t) >= 4, f"bad face line: {line!r}"
            meta["face"] = " ".join(t[1:-3])
            meta["lf"] = [int(x) for x in t[-3:]]
        elif t[0] == "sheet":
            meta["sheet"] = (int(t[1]), int(t[2]))
        elif t[0] == "cell":
            meta["cell"] = (int(t[1]), int(t[2]), int(t[3]))  # MARGIN, CW, CH
        elif t[0] == "metrics":
            m = [int(x) for x in t[1:]]
            assert len(m) == 17, f"want 17 metrics, got {len(m)}: {line!r}"
            meta["metrics"] = m
        elif t[0] == "extent":
            meta["adv"][int(t[1])] = int(t[2])
        else:
            raise AssertionError(f"unknown line: {line!r}")
    assert meta["sheet"] and meta["cell"] and meta["metrics"], "incomplete txt"
    assert len(meta["adv"]) == LAST - FIRST + 1, f"want 95 extents, got {len(meta['adv'])}"
    for code in range(FIRST, LAST + 1):
        assert code in meta["adv"], f"missing extent for {code}"
    return meta


def main():
    capdir = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "harness" / "fontcap"
    meta = parse_txt(capdir / "fontcap.txt")
    margin, cellw, cellh = meta["cell"]
    W, H = meta["sheet"]
    ncol = W // cellw
    nrow = H // cellh
    assert W % cellw == 0 and H % cellh == 0, f"sheet {W}x{H} !% cell"
    assert ncol * nrow >= LAST - FIRST + 1, "grid too small"

    sheet = Image.open(capdir / "fontcap.bmp").convert("RGB")
    assert sheet.size == (W, H), f"bmp {sheet.size} != sheet {W}x{H}"
    px = sheet.load()

    chars = {}
    for k, code in enumerate(range(FIRST, LAST + 1)):
        x0 = (k % ncol) * cellw + margin   # pen origin
        y0 = (k // ncol) * cellh
        # guard checks: the slot columns outside the strip must be pure
        # white, else a neighbor (or an oversized glyph) contaminates it
        for gx in [x0 - 2] + list(range(x0 + STRIP_R + 1, x0 + cellw - margin)):
            for gy in range(y0, y0 + cellh):
                if 0 <= gx < W and px[gx, gy] != (255, 255, 255):
                    raise AssertionError(
                        f"0x{code:02x} '{chr(code)}': guard col {gx - x0:+d} "
                        f"not white at row {gy - y0}: {px[gx, gy]}")
        rows = []
        for y in range(cellh):
            row = bytearray()
            for x in range(STRIP_W):
                row += bytes(px[x0 + STRIP_L + x, y0 + y])
            rows.append(bytes(row))
        blob = b"".join(rows)
        assert len(blob) == STRIP_W * cellh * 3
        chars[str(code)] = {"adv": meta["adv"][code], "px": base64.b64encode(blob).decode("ascii")}

    m = meta["metrics"]  # TEXTMETRICS field order (see fontcap.c)
    json_out = {
        "face": meta["face"],
        "lfHeight": meta["lf"][0], "lfWidth": meta["lf"][1], "lfWeight": meta["lf"][2],
        "tmHeight": m[0], "tmAscent": m[1], "tmDescent": m[2],
        "tmInternalLeading": m[3], "tmExternalLeading": m[4],
        "tmAveCharWidth": m[5], "tmMaxCharWidth": m[6], "tmWeight": m[7],
        "tmOverhang": m[8], "tmDigitizedAspectX": m[9],
        "tmFirstChar": m[10], "tmLastChar": m[11],
        "tmDefaultChar": m[12], "tmBreakChar": m[13],
        "tmItalic": m[14], "tmUnderlined": m[15], "tmStruckOut": m[16],
        "cell": {"margin": margin, "cellw": cellw, "cellh": cellh, "cols": ncol,
                 "strip": {"l": STRIP_L, "r": STRIP_R, "w": STRIP_W}},
        "chars": chars,
    }
    jpath = ROOT / "web" / "assets" / "font.json"
    jpath.parent.mkdir(parents=True, exist_ok=True)
    jpath.write_text(json.dumps(json_out, separators=(",", ":")) + "\n")

    # ---- shim/font.inc -------------------------------------------------
    L = []
    L.append("/* shim/font.inc — GENERATED by harness/fontcap_parse.py (Task 19).")
    L.append(" *")
    L.append(" * Pixel-exact capture of the reference run's text font: the 12px")
    L.append(" * monospace TTF that wine 9.0's GetStockObject(10) resolves to and")
    L.append(" * wproc_status_create SelectObject's into the status DC (evidence")
    L.append(" * chain: harness/fontcap/fontcap.c header). Re-capture and")
    L.append(" * regenerate; do not edit by hand.")
    L.append(" *")
    L.append(" * Per char: a STRIP_W x STRIP_H RGB strip, columns")
    L.append(" * [FONT_STRIP_L .. FONT_STRIP_L+STRIP_W-1] relative to the pen")
    L.append(" * origin, row 0 = text top (baseline at tmAscent). The game")
    L.append(" * draws left-to-right with the subpixel blend dst*cap/255, so a")
    L.append(" * later glyph's left fringe (col FONT_STRIP_L) lands in the")
    L.append(" * previous glyph's last column, exactly like the reference.")
    L.append(" *")
    L.append(" * Included by shim/text.c only.")
    L.append(" */")
    L.append("#ifndef SHIM_FONT_INC")
    L.append("#define SHIM_FONT_INC")
    L.append("")
    L.append(f"#define FONT_FIRST   {FIRST}")
    L.append(f"#define FONT_LAST    {LAST}")
    L.append(f"#define FONT_NCHARS  {LAST - FIRST + 1}")
    L.append(f"#define FONT_STRIP_L {STRIP_L}")
    L.append(f"#define FONT_STRIP_W {STRIP_W}")
    L.append(f"#define FONT_STRIP_H {cellh}")
    L.append("")
    L.append("typedef struct ShimFont {")
    L.append("    TEXTMETRICS tm;                        /* captured, see font.json */")
    L.append("    int strip_l, strip_w, strip_h;        /* pen-relative strip geometry */")
    L.append("    int adv[FONT_NCHARS];                 /* per-char advance, px */")
    L.append("    const unsigned char *const *px;       /* [FONT_NCHARS] strips: STRIP_W*STRIP_H*3 RGB, top-down */")
    L.append("} ShimFont;")
    L.append("")
    arrays = []
    for code in range(FIRST, LAST + 1):
        blob = base64.b64decode(chars[str(code)]["px"])
        name = f"g_font_px{code:02x}"
        arrays.append(name)
        L.append(f"static const unsigned char {name}[FONT_STRIP_W * FONT_STRIP_H * 3] = {{")
        for y in range(cellh):
            row = blob[y * STRIP_W * 3:(y + 1) * STRIP_W * 3]
            vals = ", ".join(f"0x{b:02x}" for b in row)
            L.append(f"    {vals}, /* row {y:2d} 0x{code:02x} */")
        L.append("};")
        L.append("")
    L.append("static const unsigned char *const g_font_px_tab[FONT_NCHARS] = {")
    for i in range(0, len(arrays), 4):
        L.append("    " + ", ".join(arrays[i:i + 4]) + ",")
    L.append("};")
    L.append("")
    L.append("static const ShimFont g_font = {")
    L.append("    { " + ", ".join(str(v) for v in m) + " },")
    L.append(f"    {STRIP_L}, {STRIP_W}, {cellh},")
    L.append("    { " + ", ".join(str(meta["adv"][c]) for c in range(FIRST, LAST + 1)) + " },")
    L.append("    g_font_px_tab")
    L.append("};")
    L.append("")
    L.append("#endif /* SHIM_FONT_INC */")
    (ROOT / "shim" / "font.inc").write_text("\n".join(L) + "\n")

    print(f"font.json: {jpath.stat().st_size} bytes, {len(chars)} chars, "
          f"strip {STRIP_W}x{cellh} @ L={STRIP_L}, tmHeight {m[0]}")
    print(f"font.inc:  {(ROOT / 'shim' / 'font.inc').stat().st_size} bytes")


if __name__ == "__main__":
    main()
