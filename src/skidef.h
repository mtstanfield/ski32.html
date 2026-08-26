/* Reconstructed SkiFree 1.04 (32-bit) — shared definitions.
 * All constants below are transcribed from original/ski32.exe (v1.04,
 * decompiled with Ghidra; see decompile/ghidra/ and decompile/NOTES.md).
 */
#ifndef SKIDEF_H
#define SKIDEF_H

#include <windows.h>
#include <stdint.h>
#include "seed_values.h" /* Task 8: SKI_SEED_CONSTANT */

/* Timer (game_resume 0x404ad0 / game_pause 0x4057c0):
 * SetTimer(hwnd, 0x29a, c678 & 0xffff, c940) — callback timer. */
#define SKI_TIMER_ID 0x29a /* 666 */
#define SKI_TIMER_MS 0x28  /* c678 initial period: 40 ms */

/* String-table ids (runtime-verified LoadStringA map, ids 1-17; see
 * decompile/NOTES.md "RT_STRING table" and src/resources.rc). */
#define STR_TITLE    1  /* "SkiFree" */
#define STR_PAUSED   2  /* "Ski Paused ... Press F3 to continue" */
#define STR_TIME     3  /* "Time:"  */
#define STR_DIST     4  /* "Dist:"  */
#define STR_SPEED    5  /* "Speed:" */
#define STR_STYLE    6  /* "Style:" */
#define STR_TIME0    7  /* "00:00:00.00" (panel sizing) */
#define STR_DIST0    8  /* " 0000m"    (panel sizing) */
#define STR_SPEED0   9  /* " 0000m/s"  (panel sizing) */
#define STR_SCORE0   10 /* "0000000"   (panel sizing) */
#define STR_FMT_TIME 11 /* "%2u:%2.2u:%2.2u.%2.2u" */
#define STR_FMT_DIST 12 /* "%5.2dm" */
#define STR_FMT_SPEED 13 /* "%5.2dm/s" */
#define STR_FMT_SCORE 14 /* "%7ld" */
#define STR_HIGHSCORE 15 /* "High Scores" */
#define STR_SUFFIX_YOU 16 /* " <-- that's you!" */
#define STR_SUFFIX_TRY 17 /* " <-- try again!" */

/* Entity struct field offsets (80-byte entities, 1/16 m units;
 * disasm-verified against ski32.exe — the decompiler misreads several of
 * these as +0x10/+0x11/+0x12; the disassembly is authoritative). */
#define ENT_NEXT     0x00 /* active-list next (ptr) */
#define ENT_COL      0x10 /* sprite column (u16; set by set_col 0x402180) */
/* +0x12..+0x13 are never accessed (padding) — verified by full .text scan. */
#define ENT_COLPTR   0x14 /* sprite column table entry ptr (c5f8 + col*0x10) */
#define ENT_TYPE     0x18 /* type (u32 slot; low16 used, 0-0x11) */
#define ENT_FRAME    0x1c /* frame (u32, 0-0x3f) */
#define ENT_X        0x40 /* x (u16, 1/16 m; teleport 0x402390) */
#define ENT_Y        0x42 /* y (u16, 1/16 m) */
#define ENT_MODE     0x44 /* mode (u16) */
#define ENT_STEER    0x46 /* steer (u16 signed, -8..8) */
#define ENT_SPEED    0x48 /* speed (u16 signed, 1/16 m per tick) */
#define ENT_CROUCH   0x4a /* 2 = crouch (Insert/Numpad0), 4 = jump (mouse) */
#define ENT_FLAG     0x4c /* u32 flags: 1 in-list, 2 group, 4 rect-cached,
                             8 dead, 0x10 in-group, 0x20 col/pos changed */

/* Steer tables (0x40a258, stride 8B: {L[frame] u32, R[frame] u32}).
 * Left key (VK_LEFT/Numpad4): newframe = L[frame]; if newframe == 7 the
 * steer is also applied (steer = max(steer-8, -8)).
 * Right key (VK_RIGHT/Numpad6): newframe = R[frame]; if newframe == 8 the
 * steer is also applied (steer = min(steer+8, 8)).
 * Frames > 0x15 are asserted (fatal, lines 0xf63/0xf6b). */
typedef struct { uint32_t left; uint32_t right; } ski_steer_pair_t;
extern const ski_steer_pair_t ski_steer_table[22]; /* frames 0..0x15 (+6 spare, as stored) */

/* Sprite column table entry (c5f8, 90 x 16B; built by ski_load_bitmaps
 * 0x405ab0 from the 89 RT_BITMAPs, ids 1..0x59; index 0 unused). */
typedef struct {
    HDC    img_dc; /* +0x00 */
    HDC    mask_dc; /* +0x04 */
    uint16_t yoff;  /* +0x08 strip offset within the column */
    uint16_t width; /* +0x0a */
    uint16_t height; /* +0x0c */
    uint16_t area;   /* +0x0e width*height */
} ski_col_entry_t; /* 16 bytes */

#endif /* SKIDEF_H */
