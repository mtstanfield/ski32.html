/* Reconstructed SkiFree 1.04 — globals + game core (Tasks 11/12).
 *
 * The window/message layer lives in src/ski_win.c. This file defines every
 * game global (VAs as in the original PE) and the non-window functions.
 * T10 leaves (string cache, fatal/assert, sound path, bitmap loader, text
 * helpers) are implemented now; T11 fills the game-logic stubs, T12 the
 * render/status stubs.
 */
#include <stdio.h>
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
    g_c648 = LocalAlloc(0, 0x2000); /* entity pool: 100 x 80B */
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

/* ================= T11 game core (stubs) ================= */

/* 0x404a00 — freelist build over the entity pool (100 x 80B at g_c648). */
void ski_level_init(void)
{
    /* T11: c618 = NULL; freelist c744 = pool[0]; pool[i].next = pool[i+1];
     * pool[99].next = NULL. */
}

/* 0x404a80 — spawn the player and start the run. */
int ski_game_start(void)
{
    /* T11:
     *   void *e = ski_entity_alloc(0, 3);
     *   g_c64c = ski_teleport(e, 0, 0, 0);
     *   g_c72c = g_c64c;
     *   if (g_c64c == NULL) return 0;
     *   ski_start_benches(); ski_start_decor();
     *   g_c650 = 0; ski_resume();
     *   return 1;
     * Stays a stub while ski_entity_alloc/ski_teleport return NULL (the
     * faithful body would fail and destroy the window at boot). */
    return 1;
}

/* 0x401000 — one fixed-step tick. */
void ski_tick(void)
{
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
    /* T11: full tick (physics, spawn scan, gate update, style, throttle). */
}

/* 0x4020d0 */
void *ski_entity_alloc(int type, uint32_t frame)
{
    (void)type; (void)frame;
    return NULL; /* T11 */
}

/* 0x402390 */
void *ski_teleport(void *ent, short x, short y, short mode)
{
    (void)ent; (void)x; (void)y; (void)mode;
    return ent; /* T11 */
}

/* 0x402120 */
void *ski_set_frame(void *ent, uint32_t frame)
{
    (void)ent; (void)frame;
    return ent; /* T11 */
}

/* 0x402220 */
void *ski_group_head(void *ent)
{
    return ent; /* T11 */
}

/* 0x406060 — clear rect-cache flags; store window center. */
void ski_size_hook(short cx, short cy)
{
    (void)cx; (void)cy;
    /* T11: walk g_c618; if (flag & 4) && !(flag & 2): if (flag & 1)
     * e = ski_group_head(e); e->flag &= ~4. */
    g_c5fc = cy;
    g_c704 = cx;
}

/* 0x4065e0 — aim angle -> facing frame 0-7 (slope (dy<<2)/dx bands). */
uint32_t ski_aim_facing(short dx, short dy)
{
    (void)dx; (void)dy;
    return 0; /* T11 */
}

/* 0x406670 — aim -> crouch/jump frame 0xd-0xf (zero-extended byte). */
uint32_t ski_aim_crouch(short dx, short dy)
{
    (void)dx; (void)dy;
    return 0; /* T11 */
}

/* 0x4051e0 — spawn the bench figures (types 0x11, frames 0x35-0x38). */
void ski_start_benches(void)
{
    /* T11 */
}

/* 0x404b50 — level layout: gates/benches/yetis; consumes the bulk of the
 * 117 rand() calls per start (see NOTES "Level layout"). */
void ski_start_decor(void)
{
    /* T11 */
}

/* 0x406cda — MSVC CRT rand algorithm (verbatim). */
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

/* 0x401d70 — ms -> "%2u:%2.2u:%2.2u.%2.2u" (string id 11) into buf. */
void ski_fmt_time(uint32_t ms, char *buf)
{
    uint32_t centi = (ms % 1000) / 10;
    uint32_t s = (ms / 1000) % 0x3c;
    uint32_t rest = (ms / 1000) / 0x3c;
    uint32_t m = rest % 0x3c;
    uint32_t h = rest / 0x3c;
    wsprintfA(buf, ski_str_cache(STR_FMT_TIME), h, m, s, centi);
}
