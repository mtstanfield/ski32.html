/* Shim core implementation: DC/bitmap model + the full verified ROP set.
 * See surface.h for the pixel model and the ROP/mono-pipeline notes. */
#include "surface.h"
#include <stdlib.h>
#include <string.h>

/* ---- object model ---------------------------------------------------- */
/* HDC is a ShimDC*, HBITMAP is a ShimBmp*. Every live object registers in
 * g_objs so SelectObject/DeleteObject/GetObjectA can identify a handle's
 * kind without dereferencing values they do not own (brush handles from
 * GetStockObject are opaque to this layer). */
typedef struct ShimBmp {
    int w, h, bpp; /* bpp: 1 or 32 */
    uint8_t *px;
} ShimBmp;

typedef struct ShimDC {
    int w, h;
    uint8_t *px;      /* own RGBA surface (window DCs, tests); NULL for memory DCs */
    HGDIOBJ cur;      /* selected object: ShimBmp*, an opaque brush, or NULL */
    ShimBmp *cur_bmp; /* cur when it is a bitmap */
} ShimDC;

enum { OBJ_DC = 1, OBJ_BMP = 2 };

typedef struct ShimObjNode {
    uint8_t kind;
    void *ptr;
    struct ShimObjNode *next;
} ShimObjNode;

static ShimObjNode *g_objs;

static void obj_track(uint8_t kind, void *p)
{
    ShimObjNode *n = malloc(sizeof *n);
    if (!n)
        abort();
    n->kind = kind;
    n->ptr = p;
    n->next = g_objs;
    g_objs = n;
}

static int obj_is(uint8_t kind, const void *p)
{
    for (const ShimObjNode *n = g_objs; n; n = n->next)
        if (n->kind == kind && n->ptr == p)
            return 1;
    return 0;
}

static void obj_untrack(uint8_t kind, void *p)
{
    ShimObjNode **pp = &g_objs;
    while (*pp) {
        if ((*pp)->kind == kind && (*pp)->ptr == p) {
            ShimObjNode *dead = *pp;
            *pp = dead->next;
            free(dead);
            return;
        }
        pp = &(*pp)->next;
    }
}

/* Shared default 1x1 black bitmap: every fresh memory DC starts with it
 * selected (real GDI behavior — SelectObject must return a non-NULL
 * previous object on a fresh DC or the game's sprite loader bails,
 * ski_core.c:268/278/288/298). Process-wide singleton: DeleteObject on it
 * is a no-op, like deleting a stock object. */
static ShimBmp *g_default_bmp;

static ShimBmp *default_bmp(void)
{
    if (!g_default_bmp) {
        g_default_bmp = calloc(1, sizeof *g_default_bmp);
        g_default_bmp->w = 1;
        g_default_bmp->h = 1;
        g_default_bmp->bpp = 32;
        g_default_bmp->px = calloc(4, 1); /* black; alpha set below */
        g_default_bmp->px[3] = 0xFF;
        obj_track(OBJ_BMP, g_default_bmp);
    }
    return g_default_bmp;
}

/* ---- pixel access ----------------------------------------------------- */
static int bmp_stride(int w, int bpp)
{
    return bpp == 1 ? ((w + 31) & ~31) / 8 : w * 4; /* GDI 32-bit row pad */
}

typedef struct {
    const uint8_t *px;
    int w, h, bpp;
} shim_src;

static uint32_t src_px(const shim_src *s, int x, int y)
{
    const uint8_t *row = s->px + (size_t)y * bmp_stride(s->w, s->bpp);
    if (s->bpp == 1)
        return (row[x >> 3] >> (7 - (x & 7))) & 1 ? SHIM_MONO_ON : SHIM_MONO_OFF;
    const uint8_t *p = row + 4 * (size_t)x;
    return (uint32_t)p[2] << 16 | (uint32_t)p[1] << 8 | p[0];
}

static void put_px(uint8_t *px, int w, int x, int y, uint32_t c)
{
    uint8_t *p = px + (size_t)y * bmp_stride(w, 32) + 4 * (size_t)x;
    p[0] = (uint8_t)c;
    p[1] = (uint8_t)(c >> 8);
    p[2] = (uint8_t)(c >> 16);
    p[3] = 0xFF;
}

/* ---- ROPs (per surface.h: the five game values + defensive extras) ---- */
static uint32_t rop32(uint32_t rop, uint32_t d, uint32_t s)
{
    switch (rop) {
    case 0x00CC0020u: return s;             /* SRCCOPY */
    case 0x00330008u: return s ^ 0xFFFFFFu; /* NOTSRCCOPY: D' = ~S */
    case 0x008800C6u: return s & d;         /* SRCAND */
    case 0x00EE0086u: return s | d;         /* SRCPAINT */
    case 0x00FF0062u: return 0xFFFFFFu;     /* WHITENESS */
    case 0x000042u:   return 0u;            /* BLACKNESS (real value, unused by the game) */
    default:          return d;
    }
}

static int rop_bit(uint32_t rop, int d, int s)
{
    switch (rop) {
    case 0x00CC0020u: return s;
    case 0x00330008u: return s ^ 1;
    case 0x008800C6u: return d & s;
    case 0x00EE0086u: return d | s;
    case 0x00FF0062u: return 1; /* WHITENESS */
    case 0x000042u:   return 0; /* BLACKNESS */
    default:          return d;
    }
}

/* ---- core ops (clipping on every path) -------------------------------- */
static int core_fill(ShimDC *d, int x, int y, int w, int h, uint32_t c)
{
    uint8_t *px;
    int sw, sh, bpp;
    if (!d || w <= 0 || h <= 0)
        return 0;
    if (d->cur_bmp) {
        px = d->cur_bmp->px; sw = d->cur_bmp->w; sh = d->cur_bmp->h; bpp = d->cur_bmp->bpp;
    } else if (d->px) {
        px = d->px; sw = d->w; sh = d->h; bpp = 32;
    } else {
        return 0;
    }
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= sh)
            continue;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= sw)
                continue;
            if (bpp == 1) {
                uint8_t *p = px + (size_t)j * bmp_stride(sw, 1) + (i >> 3);
                uint8_t m = (uint8_t)(1u << (7 - (i & 7)));
                if (c == SHIM_MONO_ON)
                    *p |= m;
                else
                    *p &= (uint8_t)~m;
            } else {
                put_px(px, sw, i, j, c);
            }
        }
    }
    return 1;
}

/* The surface a blit reads from: the selected bitmap if any, else the DC's
 * own surface, else nothing (blit no-ops). */
static int resolve_src(const ShimDC *d, shim_src *s)
{
    if (d->cur_bmp) {
        s->px = d->cur_bmp->px; s->w = d->cur_bmp->w; s->h = d->cur_bmp->h; s->bpp = d->cur_bmp->bpp;
        return 1;
    }
    if (d->px) {
        s->px = d->px; s->w = d->w; s->h = d->h; s->bpp = 32;
        return 1;
    }
    return 0;
}

static int core_blt(ShimDC *dst, int dx, int dy, int w, int h,
                    const ShimDC *src, int sx, int sy, uint32_t rop)
{
    shim_src s;
    uint8_t *dpx;
    int sw, sh, dbpp;
    if (!dst || !src || w <= 0 || h <= 0 || !resolve_src(src, &s))
        return 0;
    if (dst->cur_bmp) {
        dpx = dst->cur_bmp->px; sw = dst->cur_bmp->w; sh = dst->cur_bmp->h; dbpp = dst->cur_bmp->bpp;
    } else if (dst->px) {
        dpx = dst->px; sw = dst->w; sh = dst->h; dbpp = 32;
    } else {
        return 0;
    }
    for (int j = 0; j < h; j++) {
        int sj = sy + j, dj = dy + j;
        if (sj < 0 || sj >= s.h || dj < 0 || dj >= sh)
            continue;
        for (int i = 0; i < w; i++) {
            int si = sx + i, di = dx + i;
            if (si < 0 || si >= s.w || di < 0 || di >= sw)
                continue;
            if (dbpp == 1) {
                /* 1bpp destination: color sources threshold against
                 * SHIM_MONO_ON (the wine "is background" bit); 1bpp
                 * sources pass their raw bit; then the R2 op. */
                int sb = src_px(&s, si, sj) == SHIM_MONO_ON;
                uint8_t *p = dpx + (size_t)dj * bmp_stride(sw, 1) + (di >> 3);
                uint8_t m = (uint8_t)(1u << (7 - (di & 7)));
                int db = (*p & m) != 0;
                if (rop_bit(rop, db, sb))
                    *p |= m;
                else
                    *p &= (uint8_t)~m;
            } else {
                /* 1bpp sources expand to ON/OFF, then the per-plane ROP. */
                uint8_t *dp = dpx + (size_t)dj * bmp_stride(sw, 32) + 4 * (size_t)di;
                uint32_t d = (uint32_t)dp[2] << 16 | (uint32_t)dp[1] << 8 | dp[0];
                put_px(dpx, sw, di, dj, rop32(rop, d, src_px(&s, si, sj)));
            }
        }
    }
    return 1;
}

/* ---- Win32 surface API (shim/win32.h declarations) --------------------- */
HDC CreateCompatibleDC(HDC ref)
{
    (void)ref;
    ShimDC *d = calloc(1, sizeof *d);
    if (!d)
        return NULL;
    d->cur = (HGDIOBJ)default_bmp();
    d->cur_bmp = (ShimBmp *)d->cur;
    obj_track(OBJ_DC, d);
    return d;
}

BOOL DeleteDC(HDC h)
{
    ShimDC *d = (ShimDC *)h;
    if (!d || !obj_is(OBJ_DC, d))
        return FALSE;
    obj_untrack(OBJ_DC, d);
    free(d->px); /* the selected bitmap is game-owned: not freed here */
    free(d);
    return TRUE;
}

static HBITMAP bmp_alloc(int w, int h, int bpp)
{
    ShimBmp *b = calloc(1, sizeof *b);
    if (!b)
        return NULL;
    b->w = w;
    b->h = h;
    b->bpp = bpp;
    b->px = calloc(1, (size_t)bmp_stride(w, bpp) * h); /* zeroed: 1bpp all-0 bits */
    if (b->px && bpp == 32)
        for (int i = 0; i < w * h; i++)
            b->px[4 * i + 3] = 0xFF;
    if (!b->px) {
        free(b);
        return NULL;
    }
    obj_track(OBJ_BMP, b);
    return b;
}

HBITMAP CreateCompatibleBitmap(HDC ref, int w, int h)
{
    (void)ref;
    if (w <= 0 || h <= 0)
        return NULL;
    return bmp_alloc(w, h, 32);
}

HBITMAP CreateBitmap(int w, int h, int planes, int bpp, const void *bits)
{
    if (w <= 0 || h <= 0 || planes != 1 || (bpp != 1 && bpp != 32))
        return NULL;
    HBITMAP b = bmp_alloc(w, h, bpp);
    if (b && bits) {
        ShimBmp *s = (ShimBmp *)b;
        memcpy(s->px, bits, (size_t)bmp_stride(w, bpp) * h);
        if (bpp == 32)
            for (int i = 0; i < w * h; i++)
                s->px[4 * i + 3] = 0xFF;
    }
    return b;
}

/* New 32bpp bitmap from palette-expanded RGB rows (the embedded sprites,
 * misc.c LoadBitmapA; see surface.h for the wine 4bpp->32bpp conversion
 * evidence). Alpha is 0xFF like every other 32bpp surface. */
HBITMAP shim_bmp_from_rgb(int w, int h, const uint8_t *rgb)
{
    HBITMAP b = bmp_alloc(w, h, 32);
    if (!b)
        return NULL;
    ShimBmp *s = (ShimBmp *)b;
    for (int i = 0; i < w * h; i++) {
        s->px[4 * i] = rgb[3 * i];
        s->px[4 * i + 1] = rgb[3 * i + 1];
        s->px[4 * i + 2] = rgb[3 * i + 2];
    }
    return b;
}

HGDIOBJ SelectObject(HDC dc, HGDIOBJ obj)
{
    ShimDC *d = (ShimDC *)dc;
    if (!d || !obj_is(OBJ_DC, d) || !obj)
        return NULL;
    HGDIOBJ prev = d->cur;
    d->cur = obj;
    d->cur_bmp = obj_is(OBJ_BMP, obj) ? (ShimBmp *)obj : NULL;
    return prev;
}

BOOL DeleteObject(HGDIOBJ obj)
{
    if (!obj)
        return FALSE;
    if (obj == (HGDIOBJ)g_default_bmp)
        return TRUE; /* process-wide singleton */
    if (!obj_is(OBJ_BMP, obj))
        return FALSE; /* brushes etc: not ours to free */
    for (ShimObjNode *n = g_objs; n; n = n->next) {
        if (n->kind == OBJ_DC) {
            ShimDC *d = (ShimDC *)n->ptr;
            if (d->cur == obj) {
                d->cur = NULL;
                d->cur_bmp = NULL;
            }
        }
    }
    obj_untrack(OBJ_BMP, obj);
    free(((ShimBmp *)obj)->px);
    free(obj);
    return TRUE;
}

BOOL GetObjectA(HANDLE obj, int cnt, void *out)
{
    if (!obj_is(OBJ_BMP, obj) || cnt < (int)sizeof(BITMAP) || !out)
        return FALSE;
    const ShimBmp *b = (const ShimBmp *)obj;
    BITMAP *m = (BITMAP *)out;
    m->bmType = 0;
    m->bmWidth = b->w;
    m->bmHeight = b->h;
    m->bmWidthBytes = bmp_stride(b->w, b->bpp);
    m->bmPlanes = 1;
    m->bmBitsPixel = (WORD)b->bpp;
    m->bmBits = b->px;
    return TRUE;
}

BOOL BitBlt(HDC dst, int x, int y, int w, int h, HDC src, int sx, int sy, DWORD rop)
{
    return core_blt((ShimDC *)dst, x, y, w, h, (const ShimDC *)src, sx, sy, rop) ? TRUE : FALSE;
}

BOOL PatBlt(HDC dc, int x, int y, int w, int h, DWORD rop)
{
    uint32_t fill;
    if (rop == 0x00FF0062u)
        fill = 0xFFFFFFu; /* WHITENESS */
    else if (rop == 0x000042u)
        fill = 0u;        /* BLACKNESS */
    else
        return FALSE;     /* the game passes only WHITENESS */
    return core_fill((ShimDC *)dc, x, y, w, h, fill) ? TRUE : FALSE;
}

/* ---- low-level helpers (surface.h) ------------------------------------- */
HDC shim_dc_new(int w, int h)
{
    if (w <= 0 || h <= 0)
        return NULL;
    ShimDC *d = calloc(1, sizeof *d);
    if (!d)
        return NULL;
    d->w = w;
    d->h = h;
    d->px = calloc(1, (size_t)w * h * 4);
    if (!d->px) {
        free(d);
        return NULL;
    }
    for (int i = 0; i < w * h; i++)
        d->px[4 * i + 3] = 0xFF;
    obj_track(OBJ_DC, d);
    return d;
}

void shim_dc_free(HDC h)
{
    ShimDC *d = (ShimDC *)h;
    if (!d || !obj_is(OBJ_DC, d))
        return;
    obj_untrack(OBJ_DC, d);
    free(d->px);
    free(d);
}

void shim_dc_size(HDC h, int *w, int *hh)
{
    const ShimDC *d = (const ShimDC *)h;
    if (w)
        *w = d->w;
    if (hh)
        *hh = d->h;
}

void shim_dc_px(const HDC h, const uint8_t **px)
{
    *px = ((const ShimDC *)h)->px;
}

void shim_dc_fill(HDC dc, int x, int y, int w, int h, COLORREF c)
{
    core_fill((ShimDC *)dc, x, y, w, h, c);
}

void shim_dc_blt(HDC dst, int dx, int dy, int w, int h, HDC src, int sx, int sy, uint32_t rop)
{
    core_blt((ShimDC *)dst, dx, dy, w, h, (const ShimDC *)src, sx, sy, rop);
}

int shim_bmp_w(HBITMAP b)
{
    return ((const ShimBmp *)b)->w;
}

int shim_bmp_h(HBITMAP b)
{
    return ((const ShimBmp *)b)->h;
}

int shim_bmp_bpp(HBITMAP b)
{
    return ((const ShimBmp *)b)->bpp;
}

int shim_bmp_stride(HBITMAP b)
{
    const ShimBmp *s = (const ShimBmp *)b;
    return bmp_stride(s->w, s->bpp);
}

uint32_t shim_bmp_px(HBITMAP b, int x, int y)
{
    const ShimBmp *s = (const ShimBmp *)b;
    if (x < 0 || y < 0 || x >= s->w || y >= s->h)
        return 0;
    shim_src t = { s->px, s->w, s->h, s->bpp };
    return src_px(&t, x, y);
}

void shim_bmp_set_px(HBITMAP b, int x, int y, uint32_t c)
{
    ShimBmp *s = (ShimBmp *)b;
    if (x < 0 || y < 0 || x >= s->w || y >= s->h)
        return;
    if (s->bpp == 1) {
        uint8_t *p = s->px + (size_t)y * bmp_stride(s->w, 1) + (x >> 3);
        uint8_t m = (uint8_t)(1u << (7 - (x & 7)));
        if (c == SHIM_MONO_ON)
            *p |= m;
        else
            *p &= (uint8_t)~m;
    } else {
        put_px(s->px, s->w, x, y, c);
    }
}
