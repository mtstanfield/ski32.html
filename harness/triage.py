#!/usr/bin/env python3
"""Classify functions CRT vs GAME.

GAME signal: decompiled C references game-owned data:
  .rdata  0x40a190-0x40a4ff  class names (SkiMain/SkiStatus/button) + game const tables
  .rsrc   0x40d000-0x41c8bf  resources (not referenced directly from code; pattern kept)
  .data   0x40c030-0x40c13f  game strings (entpack.ini, nosound, WAVE, INI keys, fmt strings)
  .data   0x40c5d0-0x40c6ff  game state block part 1 (WAVE pairs, rects, entity pool, ...)
  .data   0x40c700-0x40c794  game state block part 2 (player, gates, sndPlaySoundA ptr, nosound flag)
  .data   0x40c940-0x40c968  timer proc ptr + style-score accumulators

ADJUSTMENT from the original pattern DAT_0040([a-fc-d])([0-9a-f]{2}) which matched all of
0x40a000-0x40ffff and produced a degenerate 116 GAME / 47 CRT (71%) split: it counted
CRT-owned data as game signal — the CRT heap (c5c0, c96c-c9a0), ctype table (c178-c182),
locale state (c900-c93c, c384+), env strings (c798-c7dc), SEH unwind tables (a560, a860,
a8a8) and CRT init arrays (c000-c020). The pattern below matches only the ranges verified
as game-owned by reading the decompiled C and the Ghidra data-reference dump
(196 unique DAT addresses, each cross-referenced against its referencing functions).

The heuristic still misses pure-computation game functions that touch no DAT_ globals
(only named string symbols like s_V__hack_ski32_ski2_c_0040c090 or no data at all);
those 27 are reclassified to GAME by hand review — see decompile/NOTES.md.
"""
import json, re, pathlib
d = pathlib.Path("decompile/ghidra")
fm = json.loads((d / "function-map.json").read_text())
pat = re.compile(r"DAT_0040((?:a[1-4][0-9a-f]{2}|d[0-9a-f]{3}|c0[3-9][0-9a-f]|c0[a-c][0-9a-f]|c0d8|c0e[04c]|c0f[48c]|c1[0-3][0-9a-f]|c5[df][0-9a-f]|c6[0-9a-f]{2}|c7[0-8][0-9a-f]|c79[0-4]|c9[4-5][0-9a-f]|c96[0-8]))")
for f in fm:
    c = (d / f["file"]).read_text()
    hits = {m[:3] for m in pat.findall(c)}
    f["class"] = "GAME" if hits else "CRT"
    f["dat_refs"] = sorted(hits)[:12]
(d / "function-map.json").write_text(json.dumps(fm, indent=1))
g = [f for f in fm if f["class"] == "GAME"]
print(f"GAME: {len(g)}  CRT: {len(fm) - len(g)}")
for f in g:
    print(f"  0x{f['addr']:08x} {f['name']} size={f['size']} refs={f['dat_refs']}")
