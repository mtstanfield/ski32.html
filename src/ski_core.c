/* Reconstructed SkiFree 1.04 — globals + game core (Tasks 11/12).
 *
 * The window/message layer lives in src/ski_win.c. This file defines every
 * game global (VAs as in the original PE) and the non-window functions.
 * T10 leaves (string cache, fatal/assert, sound path, bitmap loader, text
 * helpers) are implemented now; T11 fills the game-logic stubs, T12 the
 * render/status stubs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ski_game.h"

/* ================= .data globals ================= */

uint32_t g_c16c;
uint32_t g_c5d8; /* spawn cursor Y */
uint32_t g_c5dc;
uint32_t g_c5f4;
void    *g_c5f8;
uint16_t g_c5fc;
uint32_t g_c600;
uint16_t g_c640;
void    *g_c64c;
uint16_t g_c5f0, g_c5f2;
uint32_t g_c610;
void    *g_c618;
HBITMAP  g_c614;
HWND     g_c624;
HBITMAP  g_c620;
HDC      g_c63c;
uint16_t g_c6a0, g_c6a2;
HDC      g_c6a4;
uint32_t g_c6a8;
uint16_t g_c668, g_c66a;
uint16_t g_c66c, g_c66e;
HGDIOBJ  g_c664;
uint32_t g_c670;
void    *g_c674;
uint32_t g_c678;
uint32_t g_c67c;
int32_t  g_c680, g_c684;
int32_t  g_c688, g_c68c;
uint16_t g_c690, g_c6e8;
uint32_t g_c694;
HBRUSH   g_c69c;
uint32_t g_c698;
HINSTANCE g_c61c;
RECT     g_c6b0;
HWND     g_c6c8;
HDC      g_c6cc;
uint32_t g_c650;
uint32_t g_c6d0;
HBITMAP  g_c6d4;
uint16_t g_c6d8, g_c6da;
HDC      g_c6ec;
uint32_t g_c6fc;
void    *g_c6f8;
uint16_t g_c700, g_c70c;
uint32_t g_c702;
uint16_t g_c704, g_c706;
uint32_t g_c708;
uint32_t g_c714; /* spawn cursor X */
HDC      g_c710;
void    *g_c72c;
HDC      g_c730;
void    *g_c744;
uint16_t g_c74c, g_c74e;
uint32_t g_c748;
void    *g_c758;
uint32_t g_c760;
HBITMAP  g_c75c;
uint32_t g_c770;
RECT     g_c778;
char     g_c788[16]; /* zeroed by default (.data) */
HMODULE  g_c78c;
void    *g_c790;
uint32_t g_c794;
void    *g_c940;
void    *g_c94c, *g_c950;
uint32_t g_c944, g_c948;
uint32_t g_c954, g_c958, g_c95c;
uint16_t g_c960, g_c962, g_c964, g_c968;
HBITMAP  g_c644;
HDC      g_c5ec;
void    *g_c648;
ski_gate_list_t g_c630, g_c5e0, g_c658, g_c738, g_c720;

ski_sound_t ski_sound_1; /* @0x40c6c0 */
ski_sound_t ski_sound_2; /* @0x40c768 */
ski_sound_t ski_sound_3; /* @0x40c5d0 */
ski_sound_t ski_sound_4; /* @0x40c718 */
ski_sound_t ski_sound_5; /* @0x40c750 */
ski_sound_t ski_sound_6; /* @0x40c628 */
ski_sound_t ski_sound_7; /* @0x40c6f0 (resource id 9) */
ski_sound_t ski_sound_8; /* @0x40c6e0 (resource id 7) */
ski_sound_t ski_sound_9; /* @0x40c608 (resource id 8) */

int g_ski_tick = 0; /* harness counter: ski_tick() increments it (see ski_game.h) */

/* ================= T10 leaves ================= */

/* 0x4048c0 — LocalAlloc pools; fatal on failure. */
int ski_init_mem(void)
{
    g_c674 = LocalAlloc(0, 0x50);   /* string cache: 20 pointers */
    g_c5f8 = LocalAlloc(0, 0x5a0);  /* sprite column table: 90 x 16B */
    g_c648 = LocalAlloc(0, 8000);  /* entity pool: 100 x 80B (0x4048c0: 8000) */
    g_c758 = LocalAlloc(0, 0x2400); /* gate pool: 256 x 36B */
    if (g_c674 != NULL && g_c648 != NULL && g_c5f8 != NULL && g_c758 != NULL) {
        memset(g_c674, 0, 0x50);
        return 1;
    }
    ski_fatal_msg("Insufficient local memory.");
    return 0;
}

/* 0x401cf0 — LoadStringA into a LocalAlloc cache keyed by id. On a failed
 * LoadStringA (n == 0) the original still caches an empty string — kept. */
char *ski_str_cache(UINT id)
{
    uint32_t *cache = (uint32_t *)g_c674;
    if (cache[id] == 0) {
        char buf[256];
        int n = LoadStringA(g_c61c, id, buf, 0xff);
        buf[n] = '\0';
        HLOCAL h = LocalAlloc(0, (UINT)(n + 1));
        cache[id] = (uint32_t)h;
        if (h == NULL)
            return "[out o' memory]";
        lstrcpyA((LPSTR)h, buf);
    }
    return (char *)cache[id];
}

/* 0x401270 — assertion box; IDNO destroys the main window. */
void ski_assert_box(const char *caption, const char *msg)
{
    int r = MessageBoxA(NULL, msg, caption, 0x31 /* MB_ICONHAND|MB_YESNO */);
    if (r == 2 /* IDNO */)
        DestroyWindow(g_c6c8);
}

/* 0x401240 — " %s line %u" + box, then the game is paused (0x405760). */
void ski_assert_fail(const char *file, unsigned line)
{
    char buf[32];
    wsprintfA(buf, "%s line %u", file, line);
    ski_assert_box("Assertion Failed", buf);
    ski_pause_toggle();
}

/* 0x404950 — fatal box, caption = string id 1. Does not exit. */
void ski_fatal_msg(const char *msg)
{
    MessageBoxA(NULL, msg, ski_str_cache(STR_TITLE),
                0x30 /* MB_ICONERROR|MB_OK */);
}

/* 0x405620 — resolve PlaySoundA into c790; true when available. */
int ski_sound_init(void)
{
    g_c790 = (void *)PlaySoundA;
    return g_c790 != NULL;
}

/* 0x405640 — FindResourceA(hInst, MAKEINTRESOURCE(id), "WAVE"). The PE has
 * no "WAVE"-type resource node, so this returns NULL by construction
 * (the rebuild's resources.rc has none either) — silent by design. */
int ski_sound_load(UINT id, ski_sound_t *dst)
{
    HRSRC hres = FindResourceA(g_c61c, MAKEINTRESOURCEA(id), "WAVE");
    dst->h = hres;
    if (hres != NULL)
        dst->h = LoadResource(g_c61c, hres);
    if (dst->h != NULL) {
        dst->p = LockResource(dst->h);
        return 1;
    }
    dst->p = NULL;
    return 0;
}

/* 0x405730 */
void ski_sound_free(ski_sound_t *dst)
{
    if (dst->p != NULL)
        dst->p = NULL;
    if (dst->h != 0) {
        FreeResource(dst->h);
        dst->h = 0;
    }
}

/* 0x4056a0 — exit path: stop sound, free the DLL (always 0), free pairs. */
void ski_cleanup(void)
{
    if (g_c794 == 0) {
        if (g_c790 != NULL)
            /* Original pushes two zeros and calls *c790 (PlaySoundA stop). */
            ((BOOL (WINAPI *)(LPCSTR, HMODULE, DWORD))g_c790)(NULL, NULL, 0);
        if (g_c78c != 0)
            FreeLibrary(g_c78c);
        ski_sound_free(&ski_sound_1);
        ski_sound_free(&ski_sound_2);
        ski_sound_free(&ski_sound_3);
        ski_sound_free(&ski_sound_4);
        ski_sound_free(&ski_sound_5);
        ski_sound_free(&ski_sound_6);
        ski_sound_free(&ski_sound_7);
        ski_sound_free(&ski_sound_8);
        ski_sound_free(&ski_sound_9);
    }
}

/* 0x405ea0 */
HBITMAP ski_load_bitmap(UINT id)
{
    return LoadBitmapA(g_c61c, MAKEINTRESOURCEA(id));
}

/* 0x405ab0 — measure the 89 sprites (ids 1..0x59), build the small
 * (width < 33) and big strips in dedicated DC pairs, blit each sprite in
 * with SRCCOPY (image) and MASKPEN (1bpp mask), and finally create the
 * aligned canvas bitmap. Fills the c5f8 column table (ski_col_entry_t). */
int ski_load_bitmaps(HDC hdc)
{
    ski_col_entry_t *tbl = (ski_col_entry_t *)g_c5f8;
    tbl[0].img_dc = NULL;
    tbl[0].mask_dc = NULL;
    tbl[0].yoff = 0;
    tbl[0].width = 0;
    tbl[0].height = 0;
    tbl[0].area = 0;

    short max_w = 0;      /* widest sprite (16-bit) */
    short max_h = 0;      /* tallest sprite (16-bit) */
    short small_h = 0;    /* strip height: sprites with width < 33 */
    short big_h = 0;      /* strip height: sprites with width >= 33 */

    for (UINT id = 1; id < 0x5a; id++) {
        HBITMAP hbm = ski_load_bitmap(id);
        if (hbm == NULL)
            return 0;
        BITMAP bm;
        GetObjectA(hbm, sizeof(BITMAP), &bm);
        if ((short)max_w < bm.bmWidth)
            max_w = (short)bm.bmWidth;
        if (max_h < bm.bmHeight)
            max_h = (short)bm.bmHeight;
        if (bm.bmWidth < 0x21)
            small_h += (short)bm.bmHeight;
        else
            big_h += (short)bm.bmHeight;
        DeleteObject(hbm);
    }

    HBITMAP hbm;
    g_c710 = CreateCompatibleDC(hdc);
    if (g_c710 == NULL)
        return 0;
    hbm = CreateCompatibleBitmap(hdc, 0x20, (int)small_h);
    if (hbm == NULL)
        return 0;
    g_c620 = SelectObject(g_c710, hbm);
    if (g_c620 == NULL) {
        DeleteObject(hbm);
        return 0;
    }
    g_c6a4 = CreateCompatibleDC(hdc);
    if (g_c6a4 == NULL)
        return 0;
    hbm = CreateBitmap(0x20, (int)small_h, 1, 1, NULL);
    if (hbm == NULL)
        return 0;
    g_c6d4 = SelectObject(g_c6a4, hbm);
    if (g_c6d4 == NULL) {
        DeleteObject(hbm);
        return 0;
    }
    g_c730 = CreateCompatibleDC(hdc);
    if (g_c730 != NULL) {
        hbm = CreateCompatibleBitmap(hdc, (int)max_w, (int)big_h);
        if (hbm == NULL)
            return 0;
        g_c644 = SelectObject(g_c730, hbm);
        if (g_c644 == NULL) {
            DeleteObject(hbm);
            return 0;
        }
        g_c6ec = CreateCompatibleDC(hdc);
        if (g_c6ec == NULL)
            return 0;
        hbm = CreateBitmap((int)max_w, (int)big_h, 1, 1, NULL);
        if (hbm != NULL) {
            g_c75c = SelectObject(g_c6ec, hbm);
            if (g_c75c == NULL) {
                DeleteObject(hbm);
                return 0;
            }
            g_c5ec = CreateCompatibleDC(hdc);
            short small_off = 0, big_off = 0;
            for (UINT id = 1; id < 0x5a; id++) {
                ski_col_entry_t *e = &tbl[id];
                HBITMAP s = ski_load_bitmap(id);
                if (s == NULL)
                    return 0;
                BITMAP bminfo;
                GetObjectA(s, sizeof(BITMAP), &bminfo);
                short w = bminfo.bmWidth;
                short h = bminfo.bmHeight;
                e->width = (uint16_t)w;
                e->height = (uint16_t)h;
                e->area = (uint16_t)(w * h);
                short yoff;
                if (w < 0x21) {
                    yoff = small_off;
                    small_off += h;
                    e->img_dc = g_c710;
                    e->mask_dc = g_c6a4;
                } else {
                    yoff = big_off;
                    big_off += h;
                    e->img_dc = g_c730;
                    e->mask_dc = g_c6ec;
                }
                e->yoff = (uint16_t)yoff;
                HGDIOBJ old = SelectObject(g_c5ec, s);
                BitBlt(e->img_dc, 0, yoff, w, h, g_c5ec, 0, 0, 0xcc0020 /*SRCCOPY*/);
                BitBlt(e->mask_dc, 0, yoff, w, h, g_c5ec, 0, 0, 0x330008 /*MASKPEN*/);
                SelectObject(g_c5ec, old);
                DeleteObject(s);
            }
            g_c690 = (uint16_t)(((unsigned)max_w & 0xffc0) + 0x40);
            g_c6e8 = (uint16_t)(((unsigned)max_h & 0xffc0) + 0x40);
            hbm = CreateCompatibleBitmap(hdc, g_c690, g_c6e8);
            if (hbm != NULL) {
                g_c614 = SelectObject(g_c5ec, hbm);
                if (g_c614 == NULL) {
                    DeleteObject(hbm);
                    return 0;
                }
                return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

/* 0x401e20 — TextOut + line advance by tmHeight (low 16 bits of c668). */
void ski_text_draw(HDC hdc, const char *s, short x, short *y, int len)
{
    TextOutA(hdc, x, *y, s, len);
    *y += g_c668;
}

/* 0x406c50 — GetTextExtentPoint32A, keep the max width. */
void ski_text_extent(HDC hdc, short *maxw, const char *s, int len)
{
    SIZE sz;
    GetTextExtentPoint32A(hdc, s, len, &sz);
    if (*maxw < sz.cx)
        *maxw = (short)sz.cx;
}

/* ================= T11 game core =================
 * 1:1 transcription of decompile/ghidra (logic) with disassembly as the
 * authority on offsets/widths/tables. Asserts carry the decompiled line
 * numbers of V:\hack\ski32\ski2.c.
 */

#define SKI_ASSERT_FILE "V:\\hack\\ski32\\ski2.c" /* .data 0x40c090 */

/* x86 `idiv` semantics: quotient truncated TOWARD ZERO — identical to the
 * C signed `/` operator (verified: cltd;idiv at lerp 0x402e65, aim 0x4065f8). */
static int32_t ski_idiv(int32_t a, int32_t b)
{
    return a / b;
}

/* 0x406cda — MSVC CRT rand algorithm (verbatim; does NOT touch c748). */
int ski_rand(void)
{
    g_c16c = g_c16c * 0x343fd + 0x269ec3;
    return (int)((g_c16c >> 0x10) & 0x7fff);
}

/* 0x406cd0 */
void ski_rand_seed(uint32_t seed)
{
    g_c16c = seed;
}

/* 0x4020b0 — rand() % n. Callers read the low 16 bits only. */
int ski_rand_range(short n)
{
    return (int)(int16_t)(ski_rand() % n);
}

/* 0x402310 */
int ski_frame_special(short col)
{
    return (col != 0x1b && col != 0x52) ? 0 : 1;
}

/* 0x402ba0 — PlaySound if the handle is loaded and c790 (init flag) set. */
void ski_snd_play(ski_sound_t *p)
{
    if (p->h != NULL && g_c790 != NULL)
        PlaySoundA(p->h, NULL, 0x8000);
}

/* --- .rdata tables (dumped from the PE, verified against the disasm) --- */

/* a1ac: frame -> sprite column, 264 x u16 (frames 0..0x105), extracted
 * byte-for-byte from the original .rdata (file offset 0xa1ac, 528B). The
 * original indexes it as *(u16*)(0x40a1ac + 2*frame) with NO bounds check,
 * so out-of-range frames (e.g. the 0x103 aim tail) read the same garbage
 * the original reads: frame 0x103 -> col 0x0000 (PE .rdata 0x40a3b2). */
static const uint16_t ski_frame_col[264] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 65, 66, 67, 68, 69, 70, 71, 72, 73,
    74, 75, 76, 77, 78, 79, 80, 81, 83, 84, 85, 84, 49, 87, 88, 89,
    6, 0, 22, 0, 27, 0, 31, 0, 39, 0, 42, 0, 42, 0, 42, 0,
    42, 0, 56, 0, 60, 0, 1, 0, 4, 0, 2, 0, 0, 0, 3, 0,
    1, 0, 7, 0, 2, 0, 0, 0, 5, 0, 4, 0, 6, 0, 5, 0,
    8, 0, 3, 0, 2, 0, 5, 0, 6, 0, 9, 0, 2, 0, 5, 0,
    10, 0, 3, 0, 6, 0, 3, 0, 6, 0, 14, 0, 15, 0, 16, 0,
    13, 0, 13, 0, 16, 0, 15, 0, 14, 0, 14, 0, 15, 0, 20, 0,
    21, 0, 20, 0, 21, 0, 16, 0, 13, 0, 13, 0, 16, 0, 1, 16,
    0, 0, 0, 0, 0, 0, 1, 12, 1, 1, 65535, 0, 1, 0, 1, 6,
    1, 4, 65535, 0, 2, 0, 1, 0, 1, 8, 65535, 0, 3, 0, 1, 12,
    1, 1, 1, 0, 4, 0, 1, 6, 1, 4, 1, 0, 5, 0, 1, 0,
    1, 8, 1, 0, 6, 0, 1, 0, 1, 8, 65535, 0, 7, 0, 1, 0,
    1, 8, 1, 0, 8, 0, 1, 0, 0, 0, 0, 0, 9, 0, 1, 0,
    0, 0, 0, 0, 10, 0, 0, 0
};

/* a22c: spawn frame per type index (12 x u32; types up to 0x11). */
static const uint32_t ski_spawn_frame[12] = {
    6, 22, 27, 31, 39, 42, 42, 42, 42, 56, 60, 1
};

/* a434: frame -> sound id (22 x u32; low16 = id, 0x11 = "crash" special). */
static const uint32_t ski_sound_frame[22] = {
    0x12, 0x00140001, 0, 0, 0x13, 0x00160001, 0, 0,
    0x14, 0x00160001, 0, 0, 0x15, 0, 3, 6,
    0xb, 0xb, 0xb, 0xb, 0xb, 0xb
};

/* a308: player anim rows for frames 0..0x15 (22 x 16B). */
static const ski_anim_row_t ski_anim_rows[22] = {
    {1, 16, 0, 0, 0, {0, 0}, 0, {0, 0}},
    {1, 12, 1, 1, (int16_t)-1, {0, 0}, 1, {0, 0}},
    {1, 6, 1, 4, (int16_t)-1, {0, 0}, 2, {0, 0}},
    {1, 0, 1, 8, (int16_t)-1, {0, 0}, 3, {0, 0}},
    {1, 12, 1, 1, 1, {0, 0}, 4, {0, 0}},
    {1, 6, 1, 4, 1, {0, 0}, 5, {0, 0}},
    {1, 0, 1, 8, 1, {0, 0}, 6, {0, 0}},
    {1, 0, 1, 8, (int16_t)-1, {0, 0}, 7, {0, 0}},
    {1, 0, 1, 8, 1, {0, 0}, 8, {0, 0}},
    {1, 0, 0, 0, 0, {0, 0}, 9, {0, 0}},
    {1, 0, 0, 0, 0, {0, 0}, 10, {0, 0}},
    {0, 0, 0, 0, 0, {0, 0}, 11, {0, 0}},
    {0, 0, 0, 0, 0, {0, 0}, 12, {0, 0}},
    {1, 24, 0, 0, 0, {0, 0}, 13, {0, 0}},
    {1, 22, 0, 0, 0, {0, 0}, 14, {0, 0}},
    {1, 22, 0, 0, 0, {0, 0}, 15, {0, 0}},
    {1, 20, 0, 0, 0, {0, 0}, 16, {0, 0}},
    {1, 24, 0, 0, 0, {0, 0}, 17, {0, 0}},
    {1, 20, 0, 0, 0, {0, 0}, 18, {0, 0}},
    {1, 20, 0, 0, 0, {0, 0}, 19, {0, 0}},
    {1, 22, 0, 0, 0, {0, 0}, 20, {0, 0}},
    {1, 22, 0, 0, 0, {0, 0}, 21, {0, 0}}
};

/* a490: AI-skier (type 1) anim rows for frames 0x16..0x1b. */
static const ski_anim_row_t ski_anim_rows1[6] = {
    {1, 1, 0, 0, 0, {0, 0}, 22, {0, 0}},
    {1, 1, 1, 4, (int16_t)-1, {0, 0}, 23, {0, 0}},
    {1, 1, 1, 4, 1, {0, 0}, 24, {0, 0}},
    {0, 0, 0, 0, 0, {0, 0}, 25, {0, 0}},
    {0, 0, 0, 0, 0, {0, 0}, 26, {0, 0}},
    {2, 18, 2, 1, (int16_t)-1, {0, 0}, 31, {0, 0}}
};

/* a4e0: snowboarder (type 3) anim rows for frames 0x1f..0x26. */
static const ski_anim_row_t ski_anim_rows3[8] = {
    {2, 18, 2, 1, (int16_t)-1, {0, 0}, 31, {0, 0}},
    {2, 18, 2, 1, 1, {0, 0}, 32, {0, 0}},
    {1, 22, 0, 0, 0, {0, 0}, 33, {0, 0}},
    {1, 4, 0, 0, 0, {0, 0}, 34, {0, 0}},
    {1, 4, 0, 0, 0, {0, 0}, 35, {0, 0}},
    {1, 4, 0, 0, 0, {0, 0}, 36, {0, 0}},
    {1, 4, 0, 0, 0, {0, 0}, 37, {0, 0}},
    {1, 4, 0, 0, 0, {0, 0}, 38, {0, 0}}
};

/* Entity template (0x40c030): zeroed 80B, col = 0x12; from_template
 * overwrites colptr with the column table base. */
static ski_ent_t ski_ent_template;

/* --- geometry ---------------------------------------------------------- */

/* 0x401290 — strict AABB overlap (32-bit words, disasm-verified). */
int ski_rect_overlap(const int32_t *a, const int32_t *b)
{
    return (int)(b[0] < a[2] && a[0] < b[2] && b[1] < a[3] && a[1] < b[3]);
}

/* 0x4012f0 */
int ski_rect_equal(const int32_t *a, const int32_t *b)
{
    return (int)(a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3]);
}

/* 0x4014b0 — screen-space rect. Disasm-verified: x1/y1 are computed in
 * 32-bit then truncated to i16 (movswl) and stored sign-extended; x2/x3
 * are the un-truncated 32-bit sums x1+w / y1+h. w/h are sign-extended
 * from the table's u16 fields; c704/c640 read as full dwords (hi16 = 0). */
void ski_rect_calc(int32_t *r, const ski_col_entry_t *c, short x, short y, short mode)
{
    int32_t w = (int32_t)(int16_t)c->width;
    int32_t h = (int32_t)(int16_t)c->height;
    if (r == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x38b);
    if (c == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x38c);
    int32_t t = (int16_t)(g_c5fc - g_c5f2);
    int32_t x1 = (int32_t)(int16_t)((int32_t)x + (int32_t)(int16_t)g_c704 - (w / 2) - (int32_t)(int16_t)g_c640);
    int32_t y1 = (int32_t)(int16_t)((int32_t)y + t - (int32_t)mode - h);
    r[0] = x1;
    r[2] = x1 + w;
    r[1] = y1;
    r[3] = h + y1;
}

/* 0x401410 */
int32_t *ski_entity_rect(ski_ent_t *e)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x3a3);
    if (e->flags & 4) ski_assert_fail(SKI_ASSERT_FILE, 0x3a4);
    if (e->col == 0) ski_assert_fail(SKI_ASSERT_FILE, 0x3a5);
    if ((char *)g_c5f8 + (uint32_t)e->col * 0x10 != e->colptr)
        ski_assert_fail(SKI_ASSERT_FILE, 0x3a6);
    ski_rect_calc(e->rect, (const ski_col_entry_t *)e->colptr, e->x, e->y, e->mode);
    e->flags |= 4;
    return e->rect;
}

/* --- entity lifecycle --------------------------------------------------- */

/* 0x402280 — pop the freelist; 20-dword copy from src. */
ski_ent_t *ski_entity_alloc_copy(ski_ent_t *src, int in_list)
{
    ski_ent_t *n = g_c744;
    if (src == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x348);
    if (n == NULL) {
        ski_assert_fail(SKI_ASSERT_FILE, 0x359);
        return NULL;
    }
    g_c744 = n->next;
    memcpy(n, src, sizeof(*n));
    n->desc = NULL;
    if (in_list != 0) {
        n->next = src->next;
        src->next = n;
        return n;
    }
    n->next = g_c618;
    g_c618 = n;
    return n;
}

/* 0x402330 */
ski_ent_t *ski_entity_from_template(void)
{
    ski_ent_template.colptr = g_c5f8;
    return ski_entity_alloc_copy(&ski_ent_template, 0);
}

/* 0x4020d0 */
ski_ent_t *ski_entity_alloc(int type, uint32_t frame)
{
    ski_ent_t *e = ski_entity_from_template();
    if (e != NULL) {
        if (type < 0) ski_assert_fail(SKI_ASSERT_FILE, 0x56c);
        if (type > 0x11) ski_assert_fail(SKI_ASSERT_FILE, 0x56d);
        e->type = (uint32_t)type; /* 0x402109: mov %edi,0x18(%esi) full dword */
        e = ski_set_frame(e, frame);
    }
    return e;
}

/* 0x4026a0 */
ski_ent_t *ski_entity_new_col(int type, uint16_t col)
{
    ski_ent_t *e = ski_entity_from_template();
    if (e != NULL) {
        if (type < 0) ski_assert_fail(SKI_ASSERT_FILE, 0x57b);
        if (type > 0x11) ski_assert_fail(SKI_ASSERT_FILE, 0x57c);
        e->type = (uint32_t)type; /* 0x4026d9: mov %edi,0x18(%esi) full dword */
        e = ski_entity_set_col(e, col);
    }
    return e;
}

/* 0x402120 */
ski_ent_t *ski_set_frame(ski_ent_t *e, uint32_t frame)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x43c);
    if (frame >= 0x40)
        ski_assert_fail(SKI_ASSERT_FILE, 0x43d); /* fires BEFORE the change-check */
    if (e->frame != frame) {
        uint32_t c;
        if (frame >= 0x40) {
            ski_assert_fail(SKI_ASSERT_FILE, 0x440);
            c = 0;
        } else {
            c = ski_frame_col[frame];
        }
        e->frame = (uint16_t)frame;
        e = ski_entity_set_col(e, (uint16_t)c);
    }
    return e;
}

/* 0x402180 */
ski_ent_t *ski_entity_set_col(ski_ent_t *e, uint16_t col)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x3d3);
    if (col != e->col) {
        g_c6fc -= (int16_t)((const ski_col_entry_t *)e->colptr)->area;
        if (e->flags & 1)
            e = ski_group_split(e);
        e->col = col;
        e->colptr = (char *)g_c5f8 + (uint32_t)col * 0x10;
        g_c6fc += (int16_t)((const ski_col_entry_t *)e->colptr)->area;
        e->flags = (e->flags & 0xfffffffbu) | 0x20u;
        e->flags = ((uint32_t)ski_frame_special((short)col) & 1u) << 6 | (e->flags & 0xffffffbfu);
    }
    return e;
}

/* 0x402220 — split a listed entity into a group pair (was the misnamed
 * T10 stub ski_group_head; no caller existed). */
ski_ent_t *ski_group_split(ski_ent_t *e)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x3b3);
    if ((e->flags & 1) == 0) ski_assert_fail(SKI_ASSERT_FILE, 0x3b5);
    ski_ent_t *n = ski_entity_alloc_copy(e, 1);
    e->partner = n;
    if (n != NULL) {
        n->partner = e;
        n->flags |= 2;
        e->flags &= ~1u;
    }
    return e;
}

/* 0x404070 */
ski_ent_t *ski_group_head(ski_ent_t *e)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x3c5);
    ski_ent_t *p = e->partner;
    if (p == NULL)
        p = e;
    return p;
}

/* 0x401b20 */
void ski_bbox_expand(int32_t *dst, const int32_t *src)
{
    if (dst == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x16d);
    if (src == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x16e);
    if (dst[0] > src[0]) dst[0] = src[0];
    if (dst[1] > src[1]) dst[1] = src[1];
    if (src[2] > dst[2]) dst[2] = src[2];
    if (src[3] > dst[3]) dst[3] = src[3];
}

/* 0x401a60 */
void ski_group_merge(ski_ent_t *a, ski_ent_t *b)
{
    if (a == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x4e4);
    if (b == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x4e5);
    if ((a->flags & 0x10) == 0) ski_assert_fail(SKI_ASSERT_FILE, 0x4e6);
    if ((b->flags & 0x10) == 0) ski_assert_fail(SKI_ASSERT_FILE, 0x4e7);
    if (a == b) ski_assert_fail(SKI_ASSERT_FILE, 0x4e8);
    ski_ent_t *last = a;
    while (last->gnext != NULL) {
        if ((last->gnext->flags & 0x10) == 0)
            ski_assert_fail(SKI_ASSERT_FILE, 0x4ec);
        last = last->gnext;
    }
    last->gnext = b;
    ski_bbox_expand(a->bbox, b->bbox);
    b->flags &= ~0x10u;
}

/* 0x401350 */
void ski_entity_die(ski_ent_t *e)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x361);
    if ((e->flags & 1) == 0) {
        ski_ent_t *p = e->partner;
        if (p != NULL)
            p->partner = NULL;
        e->flags |= 8;
    }
}

/* 0x401390 — walk the active list, splice dead nodes onto the freelist.
 * Disasm: edi = pp (address of the next pointer slot), esi = current node.
 * Dead path splices and reuses esi->next as the freelist link WITHOUT
 * advancing edi, so the next iteration reads *edi = the active successor.
 * Alive path sets edi = esi. Advancing edi on the dead path would step into
 * the freelist chain and loop forever once two dead nodes meet there. */
void ski_entity_reap(void)
{
    ski_ent_t **pp = (ski_ent_t **)&g_c618;
    ski_ent_t *e = (ski_ent_t *)g_c618;
    while (e != NULL) {
        if (e->flags & 8) {
            if (e->desc != NULL) {
                if (((ski_gate_desc_t *)e->desc)->ent != e)
                    ski_assert_fail(SKI_ASSERT_FILE, 0x376);
                ((ski_gate_desc_t *)e->desc)->ent = NULL;
            }
            if (g_c72c == e)
                g_c72c = NULL;
            if (g_c64c == e)
                g_c64c = NULL;
            *pp = e->next;
            e->next = (ski_ent_t *)g_c744;
            g_c744 = e;
        } else {
            pp = (ski_ent_t **)&e->next;
        }
        e = *pp;
    }
}

/* --- motion ------------------------------------------------------------- */

/* 0x402390 — keeps the T10-compatible signature (T10 calls it with void*). */
void *ski_teleport(void *vp, short x, short y, short mode)
{
    ski_ent_t *e = vp;
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x40d);
    int pos_changed = !((int16_t)e->x == x && (int16_t)e->y == y);
    int16_t old_mode = e->mode;
    int is_player = (e == g_c64c);
    if (!pos_changed) {
        if (old_mode == mode)
            return e;
    } else if (is_player) {
        ski_world_shift(x, y);
    }
    uint32_t f = e->flags;
    if (f & 1)
        e = ski_group_split(e);
    int keep = (f & 4) && is_player && (old_mode == mode);
    e->y = y;
    e->x = x;
    e->mode = mode;
    e->flags = (uint32_t)(((uint32_t)keep | 8u) << 2) | (e->flags & 0xfffffffbu);
    return e;
}

/* 0x402470 — move the camera; shift cached rects of visible non-group
 * entities by the delta (16-bit arithmetic throughout). */
void ski_world_shift(short x, short y)
{
    int16_t old_x = (int16_t)g_c640;
    int16_t dy = (int16_t)(y - g_c5f2);
    ski_ent_t *e;
    for (e = g_c618; e != NULL; e = e->next) {
        uint32_t f = e->flags;
        if (e != g_c64c && (f & 4) && !(f & 2)) {
            ski_ent_t *t = e;
            if (f & 1)
                t = ski_group_split(e);
            int32_t dx = (int16_t)(x - old_x);
            t->rect[0] = t->rect[0] - dx;
            t->rect[2] = t->rect[2] - dx;
            t->rect[1] = t->rect[1] - dy;
            t->rect[3] = t->rect[3] - dy;
        }
    }
    g_c5f2 = (uint16_t)y;
    g_c640 = (uint16_t)x;
}

/* 0x402be0 */
void ski_entity_step(ski_ent_t *e)
{
    int16_t x = (int16_t)((int16_t)e->x + e->steer);
    int16_t y = (int16_t)((int16_t)e->y + e->speed);
    int16_t m = (int16_t)((int16_t)e->mode + e->transition);
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x425);
    if (g_c670 != 0) {
        x = (int16_t)(x + e->steer);
        y = (int16_t)(y + e->speed);
        m = (int16_t)(m + e->transition);
    }
    if (m > 0) {
        e->transition = (uint16_t)((int16_t)e->transition - 1);
        ski_teleport(e, x, y, m);
        return;
    }
    e->transition = 0;
    ski_teleport(e, x, y, 0);
}

/* --- spawning ----------------------------------------------------------- */

/* 0x4024f0 — world spawn point for direction 0..3. */
void ski_spawn_pos(int dir, short *px, short *py)
{
    *px = (int16_t)((int16_t)g_c640 - (int16_t)g_c704);
    *py = (int16_t)(g_c5f2 - (int16_t)g_c5fc);
    switch (dir) {
    case 0:
        *px = (int16_t)(*px + (int16_t)((int16_t)g_c6b0.left - 0x3c));
        *py = (int16_t)(*py + (int16_t)ski_rand_range((int16_t)g_c6d8) + (int16_t)g_c6b0.top);
        return;
    case 1:
        *px = (int16_t)(*px + (int16_t)((int16_t)g_c6b0.right + 0x3c));
        *py = (int16_t)(*py + (int16_t)ski_rand_range((int16_t)g_c6d8) + (int16_t)g_c6b0.top);
        return;
    case 2:
    case 3:
        break;
    default:
        ski_assert_fail(SKI_ASSERT_FILE, 0x5ae);
        return;
    }
    *px = (int16_t)(*px + (int16_t)ski_rand_range((int16_t)g_c5f0) + (int16_t)g_c6b0.left);
    if (dir != 2) {
        *py = (int16_t)(*py + (int16_t)((int16_t)g_c6b0.bottom + 0x3c));
        return;
    }
    *py = (int16_t)(*py + (int16_t)((int16_t)g_c6b0.top - 0x3c));
}

/* 0x4026f0 / 0x402770 / 0x4027a0 / 0x4027e0 — spawn pickers. c748 is the
 * window area budget set by wproc_main_size (0x40604c), NOT a rand value;
 * the round-shift idiom degenerates to a plain shift (c748 is unsigned). */
int ski_spawn_pick_wide(void)
{
    if ((g_c748 >> 5) < g_c6fc)
        return 0x12;
    int16_t r = (int16_t)ski_rand_range(1000);
    if (r < 0x32) return 10;
    if (r < 500) return 0xd;
    if (r < 700) return 0xf;
    if (r < 0x2ee) return 0xb;
    if (r < 0x3b6) return 0xe;
    if (r < 0x3ca) return 0x10;
    return 2 - (r < 0x3de);
}

int ski_spawn_pick_speed(void)
{
    /* 0x402786-0x40278b: setle(c6fc, round(c748/64)) — c6fc <= c748>>6. */
    if (g_c6fc <= (g_c748 >> 6))
        return 0xb;
    return 0x12;
}

int ski_spawn_pick_narrow(void)
{
    if ((g_c748 >> 4) < g_c6fc)
        return 0x12;
    return (int16_t)ski_rand_range(0x40) == 0 ? 2 : 0xd;
}

int ski_spawn_pick_mid(void)
{
    if ((g_c748 >> 5) < g_c6fc)
        return 0x12;
    int16_t r = (int16_t)ski_rand_range(100);
    if (r < 2) return 10;
    if (r < 20) return 0xd;
    if (r < 50) return 0xf;
    if (r < 60) return 0xb;
    /* 0x402838-0x402840: sbb -> 0xffffffff/0; and $0xfe,%al -> 0xfffffffe/0;
     * add $0x10 (32-bit) -> 0x10e (r>80) / 0x10 (r<=80). 0x10e is outside
     * sprite_frame's unsigned switch range (0xb..0x10): the original asserts
     * 0x623, then 0x57c in entity_new_col, and still creates the entity
     * (dword type 0x10e, frame 0). Reproduced 1:1; ground-truth 98s run of
     * the original showed no such assert (see NOTES T11). */
    if (r <= 80) return 0x10;
    return 0x10e;
}

/* 0x4025c0 — spawn where the cursor sits; the zone (picker) is chosen by
 * the resulting (x,y): SS band -> speed, GS band -> narrow, center band ->
 * mid, everywhere else -> wide. Type 0x12 = skip. Types < 0xb take their
 * spawn frame from a22c; types >= 0xb get a random col via sprite_frame. */
ski_ent_t *ski_spawn(int dir)
{
    int16_t x, y;
    ski_spawn_pos(dir, &x, &y);
    int type;
    if (x < -0x240 || -0x140 < x || y < 0x280 || 0x21c0 < y) {
        if (x < 0x140 || 0x200 < x || y < 0x280 || 0x4100 < y) {
            if (x < -0xa0 || 0xa0 < x || y < 0x280 || 0x4100 < y)
                type = ski_spawn_pick_wide();
            else
                type = ski_spawn_pick_mid();
        } else {
            type = ski_spawn_pick_narrow();
        }
    } else {
        type = ski_spawn_pick_speed();
    }
    ski_ent_t *e = NULL;
    if (type != 0x12) {
        if (type < 0xb)
            e = ski_entity_alloc(type, ski_spawn_frame[type]);
        else
            e = ski_entity_new_col(type, (uint16_t)ski_sprite_frame(type));
        if (e != NULL)
            e = (ski_ent_t *)ski_teleport(e, x, y, 0);
    }
    return e;
}

/* 0x402350 */
ski_ent_t *ski_spawn_dir(ski_ent_t *e, int dir)
{
    if (e != NULL) {
        int16_t x, y;
        ski_spawn_pos(dir, &x, &y);
        e = (ski_ent_t *)ski_teleport(e, x, y, 0);
    }
    return e;
}

/* 0x402850 */
uint32_t ski_sprite_frame(int type)
{
    uint32_t f;
    switch (type) {
    case 0xb:
        f = 0x1b;
        break;
    case 0xd:
        f = (uint32_t)ski_rand_range(8);
        if (f == 0) f = 0x32;
        else if (f == 1) f = 0x33;
        else f = 0x31;
        break;
    case 0xe:
        f = (uint32_t)ski_rand_range(4) == 0 ? 0x2e : 0x2d;
        break;
    case 0xf:
        f = (uint32_t)ski_rand_range(3) == 0 ? 0x30 : 0x2f;
        break;
    case 0x10:
        f = 0x34;
        break;
    default:
        ski_assert_fail(SKI_ASSERT_FILE, 0x623);
        return 0;
    }
    return f;
}

/* --- gates -------------------------------------------------------------- */

/* 0x403130 */
void ski_gate_set_col(ski_gate_desc_t *d, uint16_t col)
{
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x6ed);
    d->col = col;
    d->colptr = (char *)g_c5f8 + (uint32_t)col * 0x10;
    if (d->ent != NULL)
        ski_entity_set_col(d->ent, col);
}

/* 0x404130 — spawn the desc's entity once it enters the view rect. */
ski_ent_t *ski_gate_update(ski_gate_desc_t *d)
{
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xa2c);
    if (d->ent == NULL) {
        int16_t y = d->y;
        int32_t x = d->x;
        int32_t z = d->z;
        int32_t r[4];
        int32_t world[4] = { g_c680, g_c684, g_c688, g_c68c }; /* contiguous c680..c68f */
        ski_rect_calc(r, (const ski_col_entry_t *)d->colptr, (int16_t)x, y, (int16_t)z);
        if (ski_rect_overlap(r, world)) {
            ski_ent_t *e;
            if (d->col == 0)
                e = ski_entity_alloc(d->type, d->frame);
            else
                e = ski_entity_new_col(d->type, d->col);
            if (e != NULL) {
                ski_teleport(e, (int16_t)x, y, (int16_t)z);
                d->ent = e;
                e->desc = d;
            }
        }
    }
    return d->ent;
}

/* 0x4041c0 — advance the desc, dispatch on type, move its entity. */
void ski_gate_step(ski_gate_desc_t *d)
{
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xae7);
    d->x = (int16_t)(d->x + d->vx);
    d->y = (int16_t)(d->y + d->vy);
    d->z = (int16_t)(d->z + d->fdelta);
    if (d->type == 4)
        ski_gate_type4(d);
    else if (d->type >= 5 && d->type <= 8)
        ski_gate_cruise(d);
    else
        ski_assert_fail(SKI_ASSERT_FILE, 0xaf9);
    ski_ent_t *e = d->ent;
    if (e == NULL)
        return;
    if (e->desc != d)
        ski_assert_fail(SKI_ASSERT_FILE, 0xb03);
    ski_teleport(e, d->x, d->y, d->z);
    ski_set_frame(e, d->frame);
}

/* 0x404290 — the two boundary "gate" benches that recirculate. */
void ski_gate_type4(ski_gate_desc_t *d)
{
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xa49);
    if (d->type != 4) ski_assert_fail(SKI_ASSERT_FILE, 0xa4a);
    if (d->y < -0x3ff) {
        d->frame = 0x29;
        d->vy = 2;
        d->x = 0xff70;
        return;
    }
    if (0x5bff < d->y) {
        d->frame = 0x27;
        d->vy = 0xfffe;
        d->x = 0xff90;
        return;
    }
    if (d->ent != NULL && d->frame == 0x27) {
        if (ski_rand_range(1000) == 0) {
            int32_t z = d->z;
            int16_t y = d->y;
            int32_t x = d->x;
            ski_ent_t *sb = ski_entity_alloc(3, 0x21);
            ski_teleport(sb, (int16_t)x, y, (int16_t)z);
            d->frame = 0x28;
        }
    }
}

/* 0x404350 — cruising benches (types 5..8): homing velocity + pose.
 * All thresholds/clamps/both idiv branches disasm-verified (0x404350-0x4046c0).
 * The frame 0x32..0x37 timer block is transcribed 1:1 but unreachable in
 * the original (no code ever sets a cruise frame to 0x32). */
void ski_gate_cruise(ski_gate_desc_t *d)
{
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xa68);
    int16_t x = d->x, y = d->y;
    int16_t dx_t = 0, dy_t = 0; /* preset targets per type; clamped on reset */
    int type = d->type;
    if (d->z <= 0) {
        d->fdelta = 0;
        d->z = 0;
    } else {
        d->fdelta = (int16_t)((int16_t)d->fdelta - 1);
    }
    if (d->z != 0) {
        d->frame = (uint16_t)((d->frame == 0x2e) + 0x2e);
        return;
    }
    uint32_t frame = d->frame;
    if (frame >= 0x32 && frame < 0x38) {
        uint32_t dt = g_c698 - d->timestamp;
        switch (frame) {
        case 0x32:
            d->frame = 0x33;
            return;
        case 0x33:
            d->frame = (uint16_t)((((dt >= 500) - 1) & ~1) + 0x34);
            return;
        case 0x34:
            if (dt > 700) {
                d->frame = 0x35;
                return;
            }
            break;
        case 0x35:
            if (dt > 1000) {
                d->frame = 0x36;
                return;
            }
            break;
        case 0x36:
            d->frame = 0x37;
            return;
        case 0x37:
            d->frame = (uint16_t)((((dt >= 3000) - 1) & 0xc) + 0x2a);
            return;
        default:
            ski_assert_fail(SKI_ASSERT_FILE, 0xa76);
            return;
        }
    } else {
        int reset = 0;
        switch (type) {
        case 5:
            if (y <= -2000) reset = 1; else dy_t = -10;
            break;
        case 6:
            if (y >= 32000) reset = 1; else dy_t = 0x1a;
            break;
        case 7:
            if (x <= -16000) reset = 1; else dx_t = -0x10;
            break;
        default:
            if (type != 8) ski_assert_fail(SKI_ASSERT_FILE, 0xa76);
            if (x >= 16000) reset = 1; else dx_t = 0x10;
            break;
        }
        if (reset && g_c72c != NULL) {
            ski_ent_t *pl = (ski_ent_t *)g_c72c;
            int32_t px = pl->x, py = pl->y;
            int out = 0;
            if (type == 5) out = (py < -2000);
            else if (type == 6) out = (py > 32000);
            else if (type == 7) out = (px < -16000);
            else out = (px > 16000);
            if (out) {
                int32_t wdx = px - x, wdy = py - y;
                int32_t W = (int16_t)g_c5f0; /* window width */
                if (wdx > W)
                    x = (int16_t)(px - W);
                else if (wdx < -W)
                    x = (int16_t)(px + W);
                int32_t H = (int16_t)g_c6d8; /* window height */
                if (wdy > H)
                    y = (int16_t)(py - H);
                else if (wdy < -H)
                    y = (int16_t)(py + H);
                if (wdx < 16) { if (wdx < -16) wdx = -16; } else wdx = 16;
                if (wdy < 0x1a) { if (wdy < -0xa) wdy = -0xa; } else wdy = 0x1a;
                dx_t = (int16_t)wdx;
                dy_t = (int16_t)wdy;
                ski_snd_play(&ski_sound_7);
            }
        }
    }
    d->x = x;
    d->y = y;
    int32_t adx = dx_t < 0 ? -dx_t : dx_t;
    int32_t ady = dy_t < 0 ? -dy_t : dy_t;
    if (adx > ady) {
        d->vy = (int16_t)ski_idiv((int32_t)d->vx * dy_t, dx_t); /* discarded below */
        d->fdelta = 1;
    } else if (dy_t != 0) {
        d->vx = (int16_t)ski_idiv((int32_t)d->vy * dx_t, dy_t); /* discarded below */
        d->fdelta = 1;
    }
    /* The original always overwrites the computed velocities with the raw
     * (clamped) target vector at 0x40461d; the idiv results are dead. */
    d->vy = dy_t;
    d->vx = dx_t;
    if (dy_t < 0) {
        d->frame = (uint16_t)((frame == 0x30) + 0x30);
        return;
    }
    if (dx_t < 0) {
        d->frame = (uint16_t)((frame == 0x2c) + 0x2c);
        return;
    }
    if (dx_t > 0 || dy_t > 0) {
        d->frame = (uint16_t)((frame == 0x2e) + 0x2e);
        return;
    }
    if (ski_rand_range(10) == 0) {
        d->fdelta = 4;
        d->frame = 0x2b;
        return;
    }
    d->frame = 0x2a;
}

/* 0x4040a0 — step every moving-gate desc; update those in the view band. */
void ski_gate_list_update(ski_gate_list_t *l)
{
    if (l == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xb0d);
    ski_gate_desc_t *d = l->first;
    if (l->end < d) {
        ski_assert_fail(SKI_ASSERT_FILE, 0xb0e);
        d = l->end;
    }
    int16_t view = (int16_t)g_c5fc;
    int16_t top = (int16_t)g_c68c;
    int16_t bot = (int16_t)g_c684;
    ski_gate_desc_t *end = l->end;
    for (; d < end; d = (ski_gate_desc_t *)((char *)d + 0x24)) {
        ski_gate_step(d);
        int16_t dy = (int16_t)((int16_t)d->y - (int16_t)g_c5f2);
        int16_t lo = (int16_t)((top - view) - 0x3c);
        int16_t hi = (int16_t)((bot - view) + 0x3c);
        if ((int32_t)lo <= (int32_t)dy && (int32_t)dy < (int32_t)hi)
            ski_gate_update(d);
    }
}

/* 0x4046e0 — advance the cursor into the view band, then update forward. */
void ski_gate_scan(ski_gate_list_t *l)
{
    if (l == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xb21);
    ski_gate_desc_t *cur = l->cursor;
    if (cur < l->first) ski_assert_fail(SKI_ASSERT_FILE, 0xb22);
    if (l->end < cur) ski_assert_fail(SKI_ASSERT_FILE, 0xb23);
    ski_gate_desc_t *end = l->end;
    int16_t view = (int16_t)g_c5fc;
    int16_t top = (int16_t)g_c68c;
    int16_t low = (int16_t)((int16_t)g_c684 - view - 0x3c);
    if (cur < end) {
        do {
            int16_t dy = (int16_t)((int16_t)cur->y - (int16_t)g_c5f2);
            if ((int32_t)low <= (int32_t)dy)
                break;
            cur = (ski_gate_desc_t *)((char *)cur + 0x24);
        } while (cur < end);
    }
    if (l->first < cur) {
        do {
            int16_t dy = (int16_t)((int16_t)cur->y - (int16_t)g_c5f2);
            if ((int32_t)dy < (int32_t)low)
                break;
            cur = (ski_gate_desc_t *)((char *)cur - 0x24);
        } while (l->first < cur);
    }
    l->cursor = cur;
    if (cur < end) {
        do {
            int16_t dy = (int16_t)((int16_t)cur->y - (int16_t)g_c5f2);
            int16_t high = (int16_t)((top - view) + 0x3c);
            if ((int32_t)high <= (int32_t)dy)
                return;
            ski_gate_desc_t *next = (ski_gate_desc_t *)((char *)cur + 0x24);
            ski_gate_update(cur);
            cur = next;
        } while (cur < l->end);
    }
}

/* 0x405100 — only the first pointer is cleared (end/cursor survive). */
void ski_gate_list_clear(ski_gate_list_t *l)
{
    l->first = NULL;
}

/* 0x405120 — append at end; the 36B copy lands in the c758 pool. */
ski_gate_desc_t *ski_gate_list_add(ski_gate_list_t *l, const ski_gate_desc_t *d)
{
    uint32_t idx = g_c702++;
    ski_gate_desc_t *slot = (ski_gate_desc_t *)((char *)g_c758 + idx * 0x24);
    if (l == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xa1b);
    if (d == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0xa1c);
    if (g_c702 > 0x100) ski_assert_fail(SKI_ASSERT_FILE, 0xa1d);
    if (l->first == NULL) {
        l->cursor = slot;
        l->end = slot;
        l->first = slot;
    }
    if (l->end != slot)
        ski_assert_fail(SKI_ASSERT_FILE, 0xa20);
    l->end = (ski_gate_desc_t *)((char *)l->end + 0x24);
    memcpy(slot, d, 36);
    slot->ent = NULL;
    slot->colptr = (char *)g_c5f8 + (uint32_t)slot->col * 0x10;
    return slot;
}

/* 0x404a70 */
void ski_gate_idx_reset(void)
{
    g_c702 = 0;
}

/* 0x403a00 — pairwise collision. Byte views here (Ghidra): +0x10 = x,
 * +0x11 = mode, +0x12 = speed, +0x13 = flags low byte; int-cast offsets
 * (+0x42 y, +0x46 steer, +0x4a transition) are real. colptr +0x0a = w,
 * +0x0c = h. Disasm-verified. */
ski_ent_t *ski_collide(ski_ent_t *e1, ski_ent_t *e2)
{
    if (e1 == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x92e);
    if (e2 == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x92f);
    if (e1->type > 10)
        return e1;
    int16_t y1 = e1->y;
    int16_t y2 = e2->y;
    ski_ent_t *h1 = ski_group_head(e1);
    ski_ent_t *h2 = ski_group_head(e2);
    int16_t hy1 = h1->y;
    int16_t hy2 = h2->y;
    int sep = 0; /* e1's group interleaves e2 vertically */
    if (!((y1 < y2 || hy2 < hy1) && (y2 < y1 || hy1 < hy2)) &&
        !(y1 == y2 && hy1 == hy2))
        sep = 1;
    int type2 = e2->type;
    uint32_t frame = e1->frame;
    int16_t mode1 = e1->mode;
    int16_t top2 = (int16_t)((int16_t)((const ski_col_entry_t *)e2->colptr)->height + e2->mode);
    switch (e1->type) {
    case 0:
        if (frame == 0x11)
            break;
        switch (type2) {
        case 2:
        case 0xc:
        case 0x11:
            if (sep)
                e1->speed = (int16_t)(e1->speed / 2);
            if (e2->col == 0x52) {
                ski_score_add(-0x10);
                return ski_set_frame(e1, frame);
            }
            break;
        case 0xb:
            if (frame == 0) {
                frame = 0xd;
                e1->transition = 1;
                if (e1->speed > 4) {
                    e1->speed = (int16_t)(e1->speed / 2);
                    return ski_set_frame(e1, 0xd);
                }
            }
            break;
        case 0xe:
            if (mode1 > 0) {
                if (top2 < mode1) {
                    if (e2->col == 0x56) {
                        if (e2->flags & 1)
                            e2 = ski_group_split(e2);
                        ski_entity_die(e2);
                        ski_score_add(100);
                        return ski_set_frame(e1, frame);
                    }
                    break;
                }
                if (!sep)
                    break;
                e1->transition = (uint16_t)((int16_t)e1->speed / 2);
                ski_score_add(1);
                ski_snd_play(&ski_sound_2);
                return ski_set_frame(e1, frame);
            }
            /* mode == 0 falls through to the skier cases */
        case 1:
        case 3:
        case 4:
        case 9:
        case 10:
        case 0xd:
            if (top2 < mode1 ||
                (int16_t)((int16_t)((const ski_col_entry_t *)e1->colptr)->height + mode1) < e2->mode) {
                if (type2 == 9) {
                    ski_score_add(1000);
                    e2->type = 0xd;
                    ski_entity_set_col(e2, 0x32);
                    return ski_set_frame(e1, frame);
                }
                ski_score_add(6);
                return ski_set_frame(e1, frame);
            }
            if (sep) {
                if (type2 == 0xd) {
                    int16_t w = (int16_t)((const ski_col_entry_t *)e2->colptr)->width;
                    int16_t w1 = (int16_t)((const ski_col_entry_t *)e1->colptr)->width;
                    if (w < w1)
                        w = w1;
                    int32_t dx = (int32_t)(int16_t)e1->x - (int32_t)(int16_t)e2->x;
                    int32_t adx = (dx ^ (dx >> 0x1f)) - (dx >> 0x1f);
                    if ((int32_t)w / 2 < adx) {
                        e1->speed = (int16_t)(e1->speed / 2);
                        return ski_set_frame(e1, frame);
                    }
                }
                if (mode1 == 0 && e1->transition == 0) {
                    frame = 0xb;
                } else {
                    frame = 0x11;
                    if (e2->col == 0x32) {
                        e2->type = 9;
                        ski_set_frame(e2, 0x38);
                        ski_score_add(0x10);
                        return ski_set_frame(e1, 0x11);
                    }
                }
                if (e1->speed < 0 && e2->col == 0x2e) {
                    ski_entity_set_col(e2, 0x56);
                    return ski_set_frame(e1, frame);
                }
                ski_score_add(-0x20);
                ski_snd_play(&ski_sound_1);
                return ski_set_frame(e1, frame);
            }
            break;
        case 0xf:
            if (mode1 < 1) {
                e1->transition = 4;
                ski_score_add(1);
                ski_snd_play(&ski_sound_2);
                return ski_set_frame(e1, 0xd);
            }
            if (top2 <= mode1)
                break;
            e1->transition = (uint16_t)((int16_t)e1->speed / 2);
            ski_score_add(1);
            ski_snd_play(&ski_sound_2);
            return ski_set_frame(e1, frame);
        case 0x10:
            if (sep && (int32_t)mode1 < (int32_t)top2 / 2 && e1->speed > 0) {
                e1->transition = (uint16_t)((int16_t)e1->speed);
                ski_score_add(1);
                ski_snd_play(&ski_sound_2);
                return ski_set_frame(e1, 0xd);
            }
        }
        break;
    case 1:
        if (frame > 0x18)
            break;
        if (type2 == 0)
            ski_score_add(0x14);
        frame = (uint32_t)((e2->mode > 0) + 0x19);
        ski_snd_play(&ski_sound_6);
        return ski_set_frame(e1, frame);
    case 2:
        if (frame < 0x1d && (e2->steer != 0 || e2->speed != 0)) {
            if (type2 == 0)
                ski_score_add(3);
            frame = 0x1d;
            ski_snd_play(&ski_sound_3);
            return ski_set_frame(e1, frame);
        }
        break;
    case 3:
        switch (type2) {
        case 0:
            ski_score_add(0x14);
        case 1:
        case 3:
        case 0xd:
        case 0xe:
            if (y1 < top2 && frame != 0x22)
                return ski_set_frame(e1, 0x22);
            break;
        case 0xf:
        case 0x10:
            if (y1 < top2) {
                e1->transition = (uint16_t)((int16_t)e1->speed / 2);
                ski_snd_play(&ski_sound_5);
                return ski_set_frame(e1, 0x21);
            }
        }
        break;
    case 4:
    case 9:
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        if (e2 == g_c72c) {
            if (type2 != 0)
                ski_assert_fail(SKI_ASSERT_FILE, 0x959);
            ski_snd_play(&ski_sound_8);
            if (e2->flags & 1)
                e2 = ski_group_split(e2);
            ski_entity_die(e2);
            if (e1->desc == NULL)
                ski_assert_fail(SKI_ASSERT_FILE, 0x95c);
            ski_gate_desc_t *d = (ski_gate_desc_t *)e1->desc;
            d->frame = 0x32;
            e1->steer = 0;
            d->fdelta = 0; /* +0x1a */
            e1->speed = 0;
            d->vx = 0; /* +0x1c; vy (+0x1e) is NOT touched */
            d->timestamp = g_c698;
            return ski_set_frame(e1, 0x32);
        }
        break;
    case 10:
        e1->steer = 0;
        return ski_set_frame(e1, 0x3c);
    default:
        ski_assert_fail(SKI_ASSERT_FILE, 0x948);
    }
    return ski_set_frame(e1, frame);
}

/* --- player style sections (SS / FS / GS) ------------------------------ */

/* 0x402e30 — linear interpolation at `target` between (t0,b) and (t1,a);
 * division is idiv (truncates toward zero). */
int ski_lerp(int a, int b, int t1, int t0, int target)
{
    if (t1 == t0)
        ski_assert_fail(SKI_ASSERT_FILE, 0x64c);
    return a - ski_idiv((a - b) * (t1 - target), t1 - t0);
}

/* 0x402e80 — snap the player's pose and refresh the status panel. */
void ski_player_face(void)
{
    if (g_c72c != NULL) {
        ski_ent_t *e = (ski_ent_t *)g_c72c;
        uint32_t f = e->frame;
        if (f != 0xb && f != 0x11)
            f = (((e->mode < 1) - 1) & 0xb) + 3;
        ski_set_frame(e, f);
        ski_status_draw_values(g_c6cc);
    }
}

/* .data score-panel blobs passed to ski_score_show (T12 renders them). */
static const char g_c0d8[16] = {
    'S', 'S', 0, 0, 0x0a, 0x0a, 0, 0, '%', 's', 0, 0, '%', '9', 'l', 'd'
};
static const char g_c0f4[4] = { 'F', 'S', 0, 0 };
static const char g_c0f8[4] = { 'G', 'S', 0, 0 };

/* 0x402c60 — slalom section (left, y 0x280..0x21c0), gates at c94c. */
void ski_style_ss(ski_ent_t *e, short x_prev, short y_prev)
{
    if (e != (ski_ent_t *)g_c72c)
        return;
    int16_t x = e->x, y = e->y;
    if (e->type != 0)
        ski_assert_fail(SKI_ASSERT_FILE, 0x6fc);
    if (g_c95c == 0) {
        if (y_prev < 0x281 && 0x280 < y) {
            int32_t xa = ski_lerp(x, x_prev, y, y_prev, 0x280);
            if (-0x241 < xa && xa < -0x13f) {
                g_c95c = 1;
                g_c948 = (uint32_t)ski_lerp((int32_t)g_c698, (int32_t)g_c708, y, y_prev, 0x280);
                g_c944 = g_c948 - g_c698;
                g_c6f8 = g_c94c;
            }
        }
    } else {
        g_c944 = g_c698 - g_c948;
        if (0x21c0 < y) {
            int32_t t = ski_lerp((int32_t)g_c698, (int32_t)g_c708, y, y_prev, 0x21c0);
            g_c95c = 0;
            g_c944 = (uint32_t)(t - (int32_t)g_c948);
            g_c964 = 1;
            ski_player_face();
            ski_score_show(g_c0d8, g_c944, 1);
            return;
        }
        if (y < 0x281) {
            g_c95c = 0;
            return;
        }
        ski_gate_desc_t *g = (ski_gate_desc_t *)g_c6f8;
        if (g->y < y) {
            uint16_t col = 0x19;
            int32_t xa = ski_lerp(x, x_prev, y, y_prev, g->y);
            if ((g->col == 0x17 && g->x < xa) || (g->col == 0x18 && xa < g->x)) {
                col = 0x1a;
                g_c948 = (uint32_t)((int32_t)g_c948 - 5000);
            }
            ski_gate_set_col(g, col);
            g_c6f8 = (void *)((char *)g_c6f8 + 0x24);
            return;
        }
    }
}

/* 0x403180 — fun section (center, y 0x280..0x4100). */
void ski_style_fs(ski_ent_t *e, short x_prev, short y_prev)
{
    if (e != (ski_ent_t *)g_c72c)
        return;
    int16_t x = e->x, y = e->y;
    if (e->type != 0)
        ski_assert_fail(SKI_ASSERT_FILE, 0x72f);
    if (g_c954 == 0) {
        if (y_prev < 0x281 && 0x280 < y) {
            int32_t xa = ski_lerp(x, x_prev, y, y_prev, 0x280);
            if (-0xa1 < xa && xa < 0xa1)
                g_c954 = 1;
        }
    } else {
        if (0x4100 < y) {
            g_c954 = 0;
            g_c968 = 1;
            ski_player_face();
            ski_score_show(g_c0f4, g_c6a8, 0);
            return;
        }
        if (y < 0x281) {
            g_c954 = 0;
            return;
        }
    }
}

/* 0x403250 — giant slalom (right, y 0x280..0x4100), gates at c950. */
void ski_style_gs(ski_ent_t *e, short x_prev, short y_prev)
{
    if (e != (ski_ent_t *)g_c72c)
        return;
    int16_t x = e->x, y = e->y;
    if (e->type != 0)
        ski_assert_fail(SKI_ASSERT_FILE, 0x74e);
    if (g_c958 == 0) {
        if (y_prev < 0x281 && 0x280 < y) {
            int32_t xa = ski_lerp(x, x_prev, y, y_prev, 0x280);
            if (0x13f < xa && xa < 0x201) {
                g_c958 = 1;
                g_c948 = (uint32_t)ski_lerp((int32_t)g_c698, (int32_t)g_c708, y, y_prev, 0x280);
                g_c944 = g_c948 - g_c698;
                g_c6f8 = g_c950;
            }
        }
    } else {
        g_c944 = g_c698 - g_c948;
        if (0x4100 < y) {
            int32_t t = ski_lerp((int32_t)g_c698, (int32_t)g_c708, y, y_prev, 0x4100);
            g_c958 = 0;
            g_c944 = (uint32_t)(t - (int32_t)g_c948);
            g_c960 = 1;
            ski_player_face();
            ski_score_show(g_c0f8, g_c944, 1);
            return;
        }
        if (y < 0x281) {
            g_c958 = 0;
            return;
        }
        ski_gate_desc_t *g = (ski_gate_desc_t *)g_c6f8;
        if (g->y < y) {
            uint16_t col = 0x19;
            int32_t xa = ski_lerp(x, x_prev, y, y_prev, g->y);
            if ((g->col == 0x17 && g->x < xa) || (g->col == 0x18 && xa < g->x)) {
                col = 0x1a;
                g_c948 = (uint32_t)((int32_t)g_c948 - 5000);
            }
            ski_gate_set_col(g, col);
            g_c6f8 = (void *)((char *)g_c6f8 + 0x24);
            return;
        }
    }
}

/* --- entity animation --------------------------------------------------- */

/* 0x403430 — shared speed/steer easing. Disasm-verified: when row[8] != 0
 * the multiplier is the entity's FRAME (0x403467 loads the DWORD at +0x1c
 * into eax; 0x403487 stores eax as the multiplier); when row[8] == 0 it is
 * signum(steer) (0x40348d test; jge/setg: -1 if steer < 0, 1 if steer > 0,
 * else 0). X = multiplier * steer (0x4034ac imul, sign-extended steer). */
ski_ent_t *ski_anim_update(ski_ent_t *e, const ski_anim_row_t *row)
{
    int32_t st = (int32_t)(int16_t)e->steer;
    uint16_t sgn = row->sign;
    int32_t step;
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x79f);
    if (row == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x7a0);
    if (e->frame != row->fidx) ski_assert_fail(SKI_ASSERT_FILE, 0x7a1);
    if (sgn != 0)
        step = (int32_t)(uint32_t)*(const uint32_t *)((const uint8_t *)e + 0x1c); /* frame | pad word */
    else if (st < 0)
        step = -1;
    else
        step = st > 0 ? 1 : 0;
    int32_t s2 = step * st;
    int32_t sp = ((int32_t)(int16_t)e->speed > 0) ? (int32_t)(int16_t)e->speed : 0;
    int32_t p = (int32_t)(int16_t)row->win * sp;
    int32_t s3 = (p - (p >> 0x1f)) >> 1; /* floor /2 */
    int16_t s2l = (int16_t)s2, s3l = (int16_t)s3;
    int32_t cl;
    if ((int32_t)s2l <= (int32_t)s3l)
        cl = ((int32_t)s3l < (int32_t)s2l + (int32_t)(int16_t)row->decay)
                  ? (int32_t)s3l
                  : (int32_t)s2l + (int32_t)(int16_t)row->decay;
    else
        cl = ((int32_t)s3l <= (int32_t)s2l - 2) ? (int32_t)s2l - 2 : (int32_t)s3l;
    int16_t mx = (int16_t)row->max, ac = (int16_t)row->accel;
    int16_t spd = (int16_t)e->speed;
    int16_t nspd;
    if (spd > mx)
        nspd = ((int32_t)mx > (int32_t)spd - 2) ? mx : (int16_t)((int32_t)spd - 2);
    else
        nspd = ((int32_t)mx < (int32_t)spd + (int32_t)ac) ? mx : (int16_t)((int32_t)spd + (int32_t)ac);
    e->speed = nspd;
    e->steer = (int16_t)((uint32_t)step * (uint32_t)cl);
    return e;
}

/* 0x403540 — AI skier (type 1), frames 0x16..0x1b. */
ski_ent_t *ski_anim_type1(ski_ent_t *e)
{
    uint32_t frame = e->frame;
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x852);
    if (e->type != 1) ski_assert_fail(SKI_ASSERT_FILE, 0x853);
    if (frame > 0x18)
        return e;
    ski_entity_step(e);
    if (frame - 0x16 > 4)
        ski_assert_fail(SKI_ASSERT_FILE, 0x857);
    e = ski_anim_update(e, &ski_anim_rows1[frame - 0x16]);
    if (ski_rand_range(0xc) == 0) {
        int16_t r = (int16_t)ski_rand_range(3);
        if (r == 0)
            frame = 0x16;
        else if (r == 1)
            return ski_set_frame(e, 0x17);
        else if (r == 2)
            return ski_set_frame(e, 0x18);
    }
    return ski_set_frame(e, frame);
}

/* 0x403610 — signpost (type 2), frames 0x1b..0x1e. */
void ski_anim_type2(ski_ent_t *e)
{
    uint32_t frame = e->frame;
    if (e->type != 2)
        ski_assert_fail(SKI_ASSERT_FILE, 0x872);
    switch (frame) {
    case 0x1b:
        e->speed = (int16_t)((int16_t)ski_rand_range(3) - 1);
        ski_entity_step(e);
        ski_set_frame(e, 0x1c);
        return;
    case 0x1c:
        e->steer = 4;
        ski_entity_step(e);
        ski_set_frame(e, 0x1b);
        return;
    case 0x1d:
        e->speed = 0;
        e->steer = 0;
        {
            int16_t r = (int16_t)ski_rand_range(0x20);
            ski_entity_step(e);
            ski_set_frame(e, (r != 0) ? 0x1e : 0x1b);
        }
        return;
    case 0x1e:
        if (ski_rand_range(100) != 0) {
            ski_entity_step(e);
            ski_set_frame(e, 0x1d);
            return;
        }
        {
            int16_t mode = e->mode;
            int16_t x = e->x;
            int16_t y = (int16_t)((int16_t)e->y - 2);
            ski_ent_t *p = ski_entity_new_col(0x11, 0x52);
            ski_teleport(p, (int16_t)(x - 4), y, mode);
            frame = 0x1b;
            ski_snd_play(&ski_sound_9);
        }
    }
    ski_entity_step(e);
    ski_set_frame(e, frame);
}

/* 0x403750 — banner (type 9), frames 0x38..0x3b (loop). */
void ski_anim_type9(ski_ent_t *e)
{
    int32_t frame = e->frame;
    if (e->type != 9) ski_assert_fail(SKI_ASSERT_FILE, 0x89c);
    if (frame < 0x38) ski_assert_fail(SKI_ASSERT_FILE, 0x89d);
    if (frame > 0x3b) ski_assert_fail(SKI_ASSERT_FILE, 0x89e);
    uint32_t nf = (uint32_t)(frame + 1);
    if (nf > 0x3b)
        nf = 0x38;
    ski_set_frame(e, nf);
}

/* 0x4037b0 — green-pine sway (type 10 = 0xa), frames 0x3c..0x3f.
 * Disasm-verified 2026-08-26: jump table @0x4038fc =
 * {0x403809, 0x40385c, 0x4038c0, 0x4038ce}, common tail 0x4038e9 =
 * step(e) + set_frame(e, edi). The decompiler mis-rendered case 0x3c's
 * rand(100) != 0 branch as "frame = 0x3d": the branch (jne 0x4038e9 at
 * 0x40382c) reaches the tail with edi still the ORIGINAL frame — the
 * `mov $0x3d,%edi` at 0x403837 sits inside the rand==0 branch only.
 * Advancing to 0x3d with steer still 0 tripped the 0x8b8 assert two
 * ticks after the first pine spawn. */
void ski_anim_type10(ski_ent_t *e)
{
    uint32_t frame = e->frame;
    if (e->type != 10) ski_assert_fail(SKI_ASSERT_FILE, 0x8a9);
    if (frame < 0x3c) ski_assert_fail(SKI_ASSERT_FILE, 0x8aa);
    if (frame > 0x3f) ski_assert_fail(SKI_ASSERT_FILE, 0x8ab);
    switch (frame) {
    case 0x3c:
        if (e->steer != 0)
            ski_assert_fail(SKI_ASSERT_FILE, 0x8af);
        if (ski_rand_range(100) == 0) {
            int16_t r = (int16_t)ski_rand_range(2);
            e->steer = (int16_t)(r * 2 - 1); /* lea -1(,%eax,2), 0x403841 */
            ski_entity_step(e);
            ski_set_frame(e, 0x3d);
            return;
        }
        /* 0x40382c -> tail with original frame: step + set_frame(0x3c). */
        break;
    case 0x3d:
        if (e->steer == 0)
            ski_assert_fail(SKI_ASSERT_FILE, 0x8b8);
        if (ski_rand_range(10) == 0) {
            e->steer = 0;
            ski_entity_step(e);
            ski_set_frame(e, 0x3c);
            return;
        }
        {
            int16_t st = e->steer;
            ski_entity_step(e);
            ski_set_frame(e, (uint32_t)((st > -1) + 0x3e));
        }
        return;
    case 0x3e:
        if (e->steer > -1)
            ski_assert_fail(SKI_ASSERT_FILE, 0x8c3);
        break;
    case 0x3f:
        if (e->steer < 1)
            ski_assert_fail(SKI_ASSERT_FILE, 0x8c8);
        break;
    default:
        break; /* original keeps the existing frame (unreachable) */
    }
    ski_entity_step(e);
    ski_set_frame(e, frame);
}

/* 0x403910 — snowboarder (type 3), frames 0x1f..0x27. */
void ski_anim_type3(ski_ent_t *e)
{
    uint32_t frame = e->frame;
    if (e->type != 3) ski_assert_fail(SKI_ASSERT_FILE, 0x8e2);
    ski_entity_step(e);
    if (frame - 0x1f > 7)
        ski_assert_fail(SKI_ASSERT_FILE, 0x8e5);
    e = ski_anim_update(e, &ski_anim_rows3[frame - 0x1f]);
    if (frame == 0x1f) {
        if (ski_rand_range(10) == 0)
            frame = 0x20;
    } else if (frame == 0x20) {
        if (ski_rand_range(10) == 0) {
            ski_set_frame(e, 0x1f);
            return;
        }
    } else if (frame == 0x21) {
        if (e->mode == 0) {
            ski_set_frame(e, 0x20);
            return;
        }
    } else {
        if (frame < 0x22 || frame > 0x26)
            ski_assert_fail(SKI_ASSERT_FILE, 0x8fa);
        frame = frame + 1;
        if (frame == 0x27) {
            ski_set_frame(e, 0x20);
            return;
        }
    }
    ski_set_frame(e, frame);
}

/* 0x4028e0 — per-tick update dispatch (player + types 1,2,3,9,10). */
ski_ent_t *ski_entity_activate(ski_ent_t *e)
{
    if (e == NULL) ski_assert_fail(SKI_ASSERT_FILE, 0x907);
    if (e->type > 10 || e->desc != NULL)
        ski_assert_fail(SKI_ASSERT_FILE, 0x908);
    switch (e->type) {
    case 1:
        return ski_anim_type1(e);
    case 2:
        ski_anim_type2(e);
        return e;
    case 3:
        ski_anim_type3(e);
        return e;
    case 9:
        ski_anim_type9(e);
        return e;
    case 10:
        ski_anim_type10(e);
        return e;
    default:
        ski_assert_fail(SKI_ASSERT_FILE, 0x91f);
        return e;
    case 0:
        break;
    }
    /* player (type 0) */
    int16_t x_prev = e->x;
    int16_t y_prev = e->y;
    uint32_t frame = e->frame;
    if (frame == 0xb) {
        if (e->mode != 0) ski_assert_fail(SKI_ASSERT_FILE, 0x7eb);
        if (e->transition != 0) ski_assert_fail(SKI_ASSERT_FILE, 0x7ec);
        int16_t steer = e->steer;
        if (steer == 0 && e->speed == 0)
            frame = 0xc;
        uint16_t step = (steer < 0) ? 0xffff : (uint16_t)(0 < steer);
        e->steer = (int16_t)((int16_t)steer - (int16_t)step);
        int16_t spd = e->speed;
        if (spd < 0)
            e->speed = (int16_t)(spd + 1);
        else
            e->speed = (int16_t)((uint16_t)spd - (uint16_t)(0 < spd));
    } else {
        ski_entity_step(e);
        if (frame > 0x15)
            ski_assert_fail(SKI_ASSERT_FILE, 0x7f8);
        e = ski_anim_update(e, &ski_anim_rows[frame]);
        switch (frame) {
        case 7:
        case 9:
            frame = 3;
            break;
        case 8:
        case 10:
            frame = 6;
            break;
        case 0xd:
        case 0xe:
        case 0xf:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
            if (e->mode == 0) {
                if (frame - 0xd > 8)
                    ski_assert_fail(SKI_ASSERT_FILE, 0x812);
                uint32_t snd = ski_sound_frame[frame];
                if (snd == 0x11) {
                    ski_score_add(-0x40);
                    ski_snd_play(&ski_sound_1);
                } else {
                    ski_snd_play(&ski_sound_4);
                }
            }
            break;
        }
    }
    e = ski_set_frame(e, frame);
    int32_t score;
    switch (frame) {
    case 7:
    case 8:
    case 9:
    case 10:
        score = -1;
        break;
    case 0x10:
        score = 2;
        break;
    case 0x12:
    case 0x13:
        score = 4;
        break;
    case 0x14:
    case 0x15:
        score = 8;
        break;
    default:
        goto no_score;
    }
    ski_score_add(score);
no_score:
    ski_style_ss(e, x_prev, y_prev);
    ski_style_fs(e, x_prev, y_prev);
    ski_style_gs(e, x_prev, y_prev);
    return e;
}

/* --- level lifecycle ---------------------------------------------------- */

/* 0x404a00 — build the entity freelist over the 100x80B pool (c648). */
void ski_level_init(void)
{
    ski_ent_t *pool = (ski_ent_t *)g_c648;
    g_c618 = NULL;
    g_c744 = g_c648;
    for (uint32_t i = 0; i < 99; i++)
        pool[i].next = &pool[i + 1];
    pool[99].next = NULL;
}

/* 0x4051e0 — the four start poles, sized from the column table.
 * (c5f0's high word is never written: always 0.) */
void ski_startpoles_spawn(void)
{
    const ski_col_entry_t *tbl = (const ski_col_entry_t *)g_c5f8;
    ski_ent_t *p;
    short x, y;

    p = ski_entity_new_col(0x11, 0x35);
    x = (short)(-0x28 - (int16_t)tbl[0x35].width / 2);
    ski_teleport(p, x, g_c5f2, 0); /* pole y-basis = DAT_0040c5f2 (camera Y) */

    p = ski_entity_new_col(0x11, 0x36);
    y = (short)(g_c5f2 + (int16_t)tbl[0x36].height + 4);
    ski_teleport(p, x, y, 0);

    x = ((int16_t)tbl[0x37].width <= (int16_t)tbl[0x38].width) ? (short)tbl[0x38].width
                                                               : (short)tbl[0x37].width;
    p = ski_entity_new_col(0x11, 0x37);
    ski_teleport(p, x, tbl[0x37].height, 0);

    p = ski_entity_new_col(0x11, 0x38);
    y = (short)((int16_t)tbl[0x37].height + (int16_t)tbl[0x38].height + 4);
    ski_teleport(p, x, y, 0);
}

/* 0x404a80 — spawn the player, lay out gates, arm the timer. */
int ski_game_start(void)
{
    ski_ent_t *p = ski_entity_alloc(0, 3);
    g_c64c = ski_teleport(p, 0, 0, 0);
    g_c72c = g_c64c;
    if (g_c64c == NULL)
        return 0;
    ski_startpoles_spawn();
    ski_level_layout();
    g_c650 = 0;
    ski_resume();
    return 1;
}

/* 0x404b50 — lay out the four gate lists. RNG draws (must stay 1:1):
 * the FS loop draws 3 per iteration (sprite_frame(0xd), rand_range(0x20),
 * a DISCARDED rand_range(400)) for 39 iterations. The original leaves
 * desc->ent/colptr/frame/timestamp as stack garbage for the static lists;
 * those fields are never read (col == 0 descs only exist in c720, which
 * sets frame explicitly), so they are zeroed here for determinism. */
void ski_level_layout(void)
{
    ski_gate_desc_t d = {0};

    /* slalom (left) */
    ski_gate_list_clear(&g_c630);
    d.type = 0x11;
    d.x = ((short)g_c6b0.left - (short)g_c704) + 0x3c + (short)g_c640;
    if (d.x < -0x140)
        d.x = -0x140;
    d.col = 0x3d;
    d.y = (short)g_c6b0.bottom - (short)g_c5fc - 0x3c + (short)g_c5f2; /* 0x404bbe: add 0x40c5f2,%ax */
    if (d.y > 0x280)
        d.y = 0x208;
    ski_gate_list_add(&g_c630, &d);

    d.col = 0x39; d.x = 0xfdc0; d.y = 0x280;
    ski_gate_list_add(&g_c630, &d);
    d.col = 0x3a; d.x = 0xfec0;
    ski_gate_list_add(&g_c630, &d);

    d.type = 0xc;
    int b = 1;
    g_c94c = NULL;
    for (int16_t y = 0x3c0; y < 0x21c0; y += 0x140) {
        d.col = (uint16_t)(0x18 - (uint16_t)b);
        /* 0x404c44: neg/sbb -> 0xffffffff; and $0xa0,%al (8-BIT) -> 0xffffffa0;
         * add $0xfffffe70 -> 0xfffffe10 = -496 (b=1); b=0 -> -400. */
        d.x = (short)((b ? 0xffffffa0u : 0u) + 0xfffffe70u);
        b = !b;
        d.y = y;
        ski_gate_desc_t *p = ski_gate_list_add(&g_c630, &d);
        if (g_c94c == NULL)
            g_c94c = p;
    }

    d.type = 0x11;
    d.col = 0x3b; d.x = 0xfdc0; d.y = 0x21c0;
    ski_gate_list_add(&g_c630, &d);
    d.col = 0x3c; d.x = 0xfec0;
    ski_gate_list_add(&g_c630, &d);

    /* fun section (center) */
    ski_gate_list_clear(&g_c5e0);
    d.type = 0x11;
    d.x = (short)g_c6b0.right - (short)g_c704 - 0x3c + (short)g_c640;
    if (d.x > 0x140)
        d.x = 0x140;
    d.col = 0x3e;
    d.y = (short)g_c6b0.bottom - (short)g_c5fc - 0x3c + (short)g_c5f2; /* add 0x40c5f2,%ax */
    if (d.y > 0x280)
        d.y = 0x208;
    ski_gate_list_add(&g_c5e0, &d);

    d.col = 0x39; d.x = 0x140; d.y = 0x280;
    ski_gate_list_add(&g_c5e0, &d);
    d.col = 0x3a; d.x = 0x200;
    ski_gate_list_add(&g_c5e0, &d);

    b = 1;
    g_c950 = NULL;
    for (int16_t y = 0x410; y < 0x4100; y += 400) {
        d.type = 0xc;
        d.col = (uint16_t)(0x18 - (uint16_t)b);
        /* 0x404dbd: neg/sbb -> 0xffffffff; and $0xffffffe0 (32-bit);
         * add $0x1b0 (16-bit) -> 400 (b=1); b=0 -> 432. */
        d.x = (short)((b ? (short)0xffe0 : 0) + 0x1b0);
        b = !b;
        d.y = y;
        ski_gate_desc_t *p = ski_gate_list_add(&g_c5e0, &d);
        if (g_c950 == NULL)
            g_c950 = p;

        d.type = 0xd;
        d.col = (uint16_t)(int16_t)ski_sprite_frame(0xd);
        d.x = (short)(ski_rand_range(0x20) + 400);
        (void)ski_rand_range(400); /* DISCARDED — consumes RNG */
    }

    d.type = 0x11;
    d.col = 0x3b; d.x = 0x140; d.y = 0x4100;
    ski_gate_list_add(&g_c5e0, &d);
    d.col = 0x3c; d.x = 0x200;
    ski_gate_list_add(&g_c5e0, &d);

    /* giant slalom (right) */
    ski_gate_list_clear(&g_c658);
    d.type = 0x11;
    d.x = 0;
    d.col = 0x3f;
    d.y = (short)g_c6b0.bottom - (short)g_c5fc - 0x3c + (short)g_c5f2; /* add 0x40c5f2,%ax */
    if (d.y > 0x280)
        d.y = 0x208;
    ski_gate_list_add(&g_c658, &d);

    d.col = 0x39; d.x = 0xff60; d.y = 0x280;
    ski_gate_list_add(&g_c658, &d);
    d.col = 0x3a; d.x = 0xa0;
    ski_gate_list_add(&g_c658, &d);
    d.col = 0x3b; d.x = 0xff60; d.y = 0x4100;
    ski_gate_list_add(&g_c658, &d);
    d.col = 0x3c; d.x = 0xa0;
    ski_gate_list_add(&g_c658, &d);

    g_c968 = 0;
    g_c954 = 0;

    /* banner loop (vertical strip) */
    ski_gate_list_clear(&g_c738);
    for (int16_t y = -0x400; y < 0x5c01; y += 0x800) {
        d.type = 0xd;
        d.col = 0x40;
        d.x = 0xff80;
        d.z = 0;
        d.vx = 0;
        d.vy = 0;
        d.fdelta = 0;
        d.y = y;
        ski_gate_list_add(&g_c738, &d);
    }

    /* moving gates (cruise list) */
    ski_gate_list_clear(&g_c720);
    for (int16_t y = -0x400; y < 0x5c01; y += 0x800) {
        d.type = 4;
        d.col = 0;
        d.vx = 0;
        d.fdelta = 0;
        d.z = 0x20;
        d.y = y;
        if (y > -0x400) {
            d.frame = 0x27;
            d.x = 0xff90;
            d.vy = 0xfffe;
            ski_gate_list_add(&g_c720, &d);
        }
        if (y < 0x5c00) {
            d.frame = 0x29;
            d.x = 0xff70;
            d.vy = 2;
            ski_gate_list_add(&g_c720, &d);
        }
    }

    d.type = 7; d.frame = 0x2a; d.col = 0;
    d.x = 0xc144; d.z = 0; d.y = 0; d.fdelta = 0; d.vy = 0; d.vx = 0;
    ski_gate_list_add(&g_c720, &d);
    d.type = 8; d.x = 0x3ebc; /* frame stays 0x2a, col stays 0 */
    ski_gate_list_add(&g_c720, &d);
    d.type = 5; d.x = 0; d.y = 0xf7f4;
    ski_gate_list_add(&g_c720, &d);
    d.type = 6; d.y = 0x7d3c;
    ski_gate_list_add(&g_c720, &d);
}

/* --- per-tick update ----------------------------------------------------- */

#if SKI_HARNESS
/* Hang diagnostics: /tmp/ski_phase = last phase reached in the in-flight
 * tick (1 physics entry, 2 after pass 1, 3 after gates+reap, 4
 * pre-collision, 5 post-collision, 6 physics exit, 7 tick exit);
 * /tmp/ski_state = entity + gate state at physics entry. */
static void ski_dbg_phase(int p)
{
    static FILE *fp;
    if (!fp)
        fp = fopen("/tmp/ski_phase", "w");
    if (fp) {
        fprintf(fp, "%d tick=%u\n", p, g_ski_tick);
        fflush(fp);
    }
}

static void ski_dbg_state(void)
{
    static FILE *fs;
    if (!fs)
        fs = fopen("/tmp/ski_state", "w");
    if (!fs)
        return;
    fprintf(fs, "tick=%u cam_y=%u view=%u area=%u desc_n=%u c5d8=%d c714=%d c72c=%p\n",
            g_ski_tick, g_c5f2, g_c5fc, g_c6fc, g_c702, (int)(int16_t)g_c5d8,
            (int)(int16_t)g_c714, (void *)g_c72c);
    int n = 0;
    for (const ski_ent_t *e = (const ski_ent_t *)g_c618; e != NULL && n < 500;
         e = e->next) {
        fprintf(fs,
                "e %08lx next=%08lx part=%08lx desc=%08lx type=%u fr=%u "
                "x=%d y=%d mode=%u steer=%d speed=%d fl=%x\n",
                (unsigned long)(uintptr_t)e, (unsigned long)(uintptr_t)e->next,
                (unsigned long)(uintptr_t)e->partner,
                (unsigned long)(uintptr_t)e->desc, e->type, e->frame, e->x,
                e->y, e->mode, e->steer, e->speed, e->flags);
        if (e == (const ski_ent_t *)g_c72c) {
            const unsigned char *b = (const unsigned char *)e;
            fprintf(fs, "RAW ");
            for (int k = 0x18; k < 0x28; k++)
                fprintf(fs, "%02x", b[k]);
            fprintf(fs, "\n");
        }
        n++;
    }
    if (n >= 500)
        fprintf(fs, "CYCLE (>=500 nodes)\n");
    fprintf(fs, "free=%08lx\n", (unsigned long)(uintptr_t)g_c744);
    {
        const char *nm[5] = { "c630", "c5e0", "c658", "c738", "c720" };
        const ski_gate_list_t *ls[5] = {
            &g_c630, &g_c5e0, &g_c658, &g_c738, &g_c720
        };
        for (int i = 0; i < 5; i++)
            fprintf(fs, "%s first=%08lx end=%08lx cur=%08lx\n", nm[i],
                    (unsigned long)(uintptr_t)ls[i]->first,
                    (unsigned long)(uintptr_t)ls[i]->end,
                    (unsigned long)(uintptr_t)ls[i]->cursor);
        int i = 0;
        for (const ski_gate_desc_t *d = g_c720.first; d < g_c720.end && i < 8;
             d++, i++)
            fprintf(fs, "c720[%d] %08lx ent=%08lx type=%u fr=%u x=%d y=%d "
                        "vx=%d vy=%d\n",
                    i, (unsigned long)(uintptr_t)d,
                    (unsigned long)(uintptr_t)d->ent, d->type, d->frame,
                    d->x, d->y, d->vx, d->vy);
    }
    fflush(fs);
}
#endif

/* 0x401e50 — physics pass. c5d8/c714 arithmetic is 16-bit throughout
 * (movw/addw; u16 stores zero the high word), reproduced with u16 wraps. */
void ski_game_physics(void)
{
#if SKI_HARNESS
    ski_dbg_state();
    ski_dbg_phase(1);
#endif
    g_c714 = (uint32_t)(uint16_t)((uint16_t)g_c714 - (uint16_t)g_c640);
    g_c5d8 = (uint32_t)(uint16_t)((uint16_t)g_c5d8 - (uint16_t)g_c5f2);

    /* pass 1: activate, then cull entities outside the world rect */
    int32_t world[4] = { g_c680, g_c684, g_c688, g_c68c }; /* contiguous c680..c68f */
    for (ski_ent_t *e = (ski_ent_t *)g_c618; e != NULL; e = e->next) {
        if (e->flags & 0xa)
            continue;
        e->flags &= 0xffffffdfu;
        if (e->desc == NULL && e->type < 0xb)
            ski_entity_activate(e);
        if ((e->flags & 1) == 0 && e != (ski_ent_t *)g_c72c) {
            int32_t *r = (e->flags & 4) ? e->rect : ski_entity_rect(e);
            if (ski_rect_overlap(r, world) == 0) {
                g_c6fc -= (uint32_t)(int16_t)((const ski_col_entry_t *)e->colptr)->area;
                ski_entity_die(e);
            }
        }
    }
    #if SKI_HARNESS
    ski_dbg_phase(2);
    #endif
    ski_gate_scan(&g_c630);
    ski_gate_scan(&g_c5e0);
    ski_gate_scan(&g_c658);
    ski_gate_scan(&g_c738);
    ski_gate_list_update(&g_c720);
    ski_entity_reap();
    #if SKI_HARNESS
    ski_dbg_phase(3);
    #endif

    /* pass 2: pairwise collisions (each unordered pair once) */
    #if SKI_HARNESS
    ski_dbg_phase(4);
    #endif
    for (ski_ent_t *e = (ski_ent_t *)g_c618; e != NULL; e = e->next) {
        if (e->flags & 2)
            continue;
        int32_t *re = (e->flags & 4) ? e->rect : ski_entity_rect(e);
        uint32_t fe = e->flags; /* read once, after the rect refresh */
        for (ski_ent_t *o = (ski_ent_t *)g_c618; o != NULL && o != e; o = o->next) {
            uint32_t fo = o->flags;
            if ((fo & 2) != 0)
                continue;
            if (!((int32_t)(fe << 0x1a) < 0) && !(fo & 0x20))
                continue;
            int32_t *ro = (fo & 4) ? o->rect : ski_entity_rect(o);
            if (ski_rect_overlap(re, ro) != 0) {
                ski_collide(e, o);
                if ((e->flags & 8) == 0 && (o->flags & 8) == 0)
                    ski_collide(o, e);
            }
        }
    }
    #if SKI_HARNESS
    ski_dbg_phase(5);
    #endif

    /* restore spawn cursors, then fill the spawn bands */
    g_c5d8 = (uint32_t)(uint16_t)((uint16_t)g_c5d8 + (uint16_t)g_c5f2);
    g_c714 = (uint32_t)(uint16_t)((uint16_t)g_c714 + (uint16_t)g_c640);
    while ((int16_t)g_c5d8 > 0x3c) {
        ski_spawn(3);
        g_c5d8 = (uint32_t)(uint16_t)((uint16_t)g_c5d8 - 0x3c);
    }
    while ((int16_t)g_c5d8 < -0x3c) {
        ski_spawn(2);
        g_c5d8 = (uint32_t)(uint16_t)((uint16_t)g_c5d8 + 0x3c);
    }
    while ((int16_t)g_c714 > 0x3c) {
        ski_spawn(1);
        g_c714 = (uint32_t)(uint16_t)((uint16_t)g_c714 - 0x3c);
    }
    while ((int16_t)g_c714 < -0x3c) {
        ski_spawn(0);
        g_c714 = (uint32_t)(uint16_t)((uint16_t)g_c714 + 0x3c);
    }
    if (ski_rand_range(0x29a) == 0) {
        ski_ent_t *e = ski_entity_alloc(3, 0x1f);
        ski_spawn_dir(e, 2);
    }
#if SKI_HARNESS
    ski_dbg_phase(6);
#endif
}

/* 0x401000 — one fixed-step tick (40 ms timer callback). */
void ski_tick(void)
{
    DWORD now;
#if SKI_DETERMINISTIC
    /* T8 seed freeze: the original's GetTickCount() advances 40 ms per
     * timer tick; the deterministic build mirrors that exactly (NOTES M1#2). */
    now = g_c698 + SKI_TIMER_MS;
#else
    now = GetTickCount();
#endif
    g_c5f4 = now - g_c698;
    g_c708 = g_c698;
    g_c698 = now;
    ski_game_physics();
    ski_render(g_c63c, &g_c6b0);
    g_c610 = 1;
    if (g_c698 - g_c5dc > 0x147)
        ski_status_draw_values(g_c6cc);
#if SKI_HARNESS
    ski_dbg_phase(7);
#endif

    g_ski_tick++;
#if SKI_HARNESS
    /* Harness liveness probe (T13 builds): dump the tick counter to a
     * known file once per second (25 ticks at the 40 ms period). */
    if ((g_ski_tick & 0x1f) == 0) {
        FILE *f = fopen("/tmp/ski_ticks", "w");
        if (f) {
            fprintf(f, "%u\n", g_ski_tick);
            fclose(f);
        }
    }
#endif
}

/* --- window resize / aim ------------------------------------------------- */

/* 0x406060 — WM_SIZE: drop the rect cache, store the window center. */
void ski_size_hook(short cx, short cy)
{
    for (ski_ent_t *e = (ski_ent_t *)g_c618; e != NULL; e = e->next) {
        uint32_t f = e->flags;
        if ((f & 4) != 0 && (f & 2) == 0) {
            ski_ent_t *p = e;
            if ((f & 1) != 0)
                p = ski_group_split(e);
            p->flags &= 0xfffffffbu;
        }
    }
    g_c5fc = (uint16_t)cy;
    g_c704 = (uint16_t)cx;
}

/* 0x4065e0 — facing pose from the aim vector. Disasm-verified:
 * r = idiv(dy * 4, dx) (truncating); negative ladder is INCLUSIVE
 * (cmp;jg: return at <=): r<=-12->0, r<=-6->1, r<=-3->2, r<=-1->3;
 * positive: r>=12->0, r>=6->4, r>=3->5, r>=1->6; dy>0 && dx==0 -> 0
 * (0x4065e5-0x4065ec); tail (dy<=0 or r==0): dx>=0 -> 6, dx<0 -> 0x103.
 * 0x103 (259) is an ORIGINAL BUG — an out-of-range frame that flows into
 * ski_set_frame (soft asserts 0x43d/0x440) and reads the frame-col table at
 * a1ac + 2*0x103. Reproduced faithfully (strict 1:1 bar). */
uint32_t ski_aim_facing(short cx, short dx)
{
    int32_t r;
    /* Decompile param_1 = CX reg, param_2 = DX reg (see ski_game.h).
     * 0x4065e0: test DX; jle tail; test CX; ==0 -> 0; r = idiv(DX*4, CX). */
    if (dx > 0) {
        if (cx == 0)
            return 0;
        r = ski_idiv((int32_t)dx << 2, (int32_t)cx);
        if (r <= -12) return 0;
        if (r <= -6) return 1;
        if (r <= -3) return 2;
        if (r <= -1) return 3;
        if (r >= 12) return 0;
        if (r >= 6) return 4;
        if (r >= 3) return 5;
        if (r >= 1) return 6;
    }
    /* 0x406655: test CX; setge; dec; and $0xfd; add $6 -> CX>=0 -> 6,
     * CX<0 -> 0x103 (out-of-range frame; original quirk, .rdata col 0). */
    return cx >= 0 ? 6 : 0x103;
}

/* 0x406670 — crouch pose 0xd..0x10 from the aim vector. */
uint32_t ski_aim_crouch(short cx, short dx)
{
    /* 1:1 with the decompile (param_1 = CX reg, param_2 = DX reg); each of
     * the 4 quadrant paths was re-verified against disasm 0x406670-0x4066c4. */
    if (cx >= 0) {
        if (dx < 0)
            return (uint32_t)((-cx != dx && cx <= -dx) + 15);
        return (uint32_t)((dx <= cx) + 13);
    }
    if (dx < 0)
        return (uint32_t)((((cx <= dx) - 1) & 2) + 14);
    return (uint32_t)((cx <= -dx) + 13);
}

/* --- high-score table ---------------------------------------------------- */

/* 0x403420 — only counts during an active fun-section run (c954). */
void ski_score_add(int d)
{
    if (g_c954 != 0)
        g_c6a8 += (uint32_t)d;
}

/* 0x402ec0 — high-score table in entpack.ini ([Ski] "SS"/"FS"/"GS"),
 * newest best-10, then the modal "High Scores" box. .rdata formats:
 * c0dc = "\n\0" pair, c0e0 = "%s", c0e4 = "%9ld", c0ec = "%ld ".
 * The token parser is the CRT _atoi (0x406cf8) == atoi(). */
void ski_score_show(const char *panel, uint32_t value, int kind)
{
    static const char fmt_dc[] = { 0x0a, 0x0a, 0, 0 }; /* 0x40c0dc */
    static const char fmt_s[] = "%s";                  /* 0x40c0e0 */
    static const char fmt_9ld[] = "%9ld";              /* 0x40c0e4 */
    static const char fmt_ld_sp[] = "%ld ";            /* 0x40c0ec */
    int table[10];
    char buf[0x100];
    int count = 0;
    int pos = 0;
    char *p;
    int i;

    if (kind != 0)
        value = (uint32_t)(-(int32_t)value);

    GetPrivateProfileStringA("Ski", panel, g_c788, buf, 0x100, "entpack.ini");
    p = buf;
    while (*p != 0 && count < 10) {
        char *q;
        while (*p == 0x20)
            p++;
        if (*p != 0x20) {
            q = p;
            do {
                if (*q == 0)
                    break;
            } while (*++q != 0x20);
            if (q != p) {
                if (*q != 0) {
                    *q = 0;
                    q++;
                }
                table[count] = atoi(p);
                count++;
                p = q;
            }
        }
    }
    if (count != 0) {
        i = 0;
        do {
            if (table[i] < (int32_t)value)
                break;
            i++;
        } while (i < count);
        pos = i;
        if (pos > 9)
            goto serialize;
    }
    if (count == 10)
        count = 9;
    if (pos < count)
        for (i = count; i > pos; i--)
            table[i] = table[i - 1];
    count++;
    table[pos] = (int32_t)value;
serialize:
    p = buf;
    for (i = 0; i < count; i++)
        p += wsprintfA(p, fmt_ld_sp, table[i]);
    WritePrivateProfileStringA("Ski", panel, buf, "entpack.ini");
    p = buf;
    for (i = 0; i < count; i++) {
        int n;
        if (i != 0)
            *p++ = 10;
        if (kind == 0)
            n = wsprintfA(p, fmt_9ld, table[i]);
        else
            n = ski_fmt_time((uint32_t)(-(int32_t)table[i]), p) & 0xffff;
        p += n;
        if (i == pos)
            p += wsprintfA(p, fmt_s, ski_str_cache(STR_SUFFIX_YOU));
    }
    if (pos == 10) {
        int n;
        p += wsprintfA(p, fmt_dc);
        if (kind == 0)
            n = wsprintfA(p, fmt_9ld, (int32_t)value);
        else
            n = ski_fmt_time((uint32_t)(-(int32_t)value), p);
        p += wsprintfA(p + n, fmt_s, ski_str_cache(STR_SUFFIX_TRY));
    }
    MessageBoxA(g_c6c8, buf, ski_str_cache(STR_HIGHSCORE), 0);
}

/* ================= T12 render/status (stubs) ================= */

/* 0x401060 — scene render into the main DC. */
void ski_render(HDC hdc, const RECT *rc)
{
    (void)hdc; (void)rc;
    /* T12 */
}

/* 0x406100 — walk the active list, refresh rects, intersect-test, render. */
void ski_paint_scene(HDC hdc, const RECT *rc)
{
    (void)hdc; (void)rc;
    /* T12 */
}

/* 0x401b80 — panel values (style/dist/speed/score) at x = label_w + 2;
 * stamps g_c5dc = g_c698 (status throttle). */
void ski_status_draw_values(HDC hdc)
{
    (void)hdc;
    /* T12 */
    g_c5dc = g_c698;
}

/* 0x401d70 — ms -> "%2u:%2.2u:%2.2u.%2.2u" (string id 11) into buf;
 * returns the written length (used by score_show 0x402ec0). */
int ski_fmt_time(uint32_t ms, char *buf)
{
    int32_t i = (int32_t)ms;
    uint32_t centi = ((i % 1000) & 0xffffU) / 10;
    uint32_t s = (i / 1000) % 0x3c & 0xffffU;
    uint32_t rest = (i / 1000) / 0x3c;
    uint32_t m = rest % 0x3c & 0xffffU;
    uint32_t h = rest / 0x3c & 0xffffU;
    return wsprintfA(buf, ski_str_cache(STR_FMT_TIME), h, m, s, centi);
}
