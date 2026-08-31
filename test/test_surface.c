/* Host-side unit tests for the shim surface core (Task 17).
 * Proves the DC/bitmap model, clipping, SelectObject swap, and the full
 * verified ROP set (SRCCOPY / NOTSRCCOPY / SRCAND / SRCPAINT / WHITENESS,
 * incl. 1bpp mask-strip behavior) without a browser. Built natively by
 * CMake (see the test target in CMakeLists.txt) and run via ctest. */
#include "shim/surface.h"
#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            failures++;                                                    \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
        }                                                                  \
    } while (0)

#define WHITE 0x00FFFFFFu
#define RED   0x000000FFu
#define GREEN 0x0000FF00u
#define BLUE  0x00FF0000u

/* ROP values the game passes as raw literals (API.md "ROP set"). */
#define ROP_SRCCOPY    0xCC0020u
#define ROP_NOTSRCCOPY 0x330008u /* game "MASKPEN" */
#define ROP_SRCAND     0x8800C6u
#define ROP_SRCPAINT   0xEE0086u
#define ROP_WHITENESS  0xFF0062u
#define ROP_BLACKNESS  0x000042u /* real GDI BLACKNESS; defensive */

static uint32_t at(HDC dc, int x, int y)
{
    const uint8_t *px;
    int w;
    shim_dc_px(dc, &px);
    shim_dc_size(dc, &w, NULL);
    const uint8_t *p = px + (size_t)(y * w + x) * 4;
    return (uint32_t)p[2] << 16 | (uint32_t)p[1] << 8 | p[0];
}

/* Plan step 3: fill + clipped fill, SRCCOPY blit + clipped/negative blit. */
static void test_plan_cases(void)
{
    HDC a = shim_dc_new(8, 4);
    HDC b = shim_dc_new(4, 2);
    CHECK(a && b);
    shim_dc_fill(a, 0, 0, 8, 4, WHITE);
    shim_dc_fill(b, 0, 0, 4, 2, RED);
    CHECK(at(a, 0, 0) == WHITE && at(a, 7, 3) == WHITE);

    shim_dc_blt(a, 1, 1, 2, 1, b, 0, 0, ROP_SRCCOPY);
    CHECK(at(a, 1, 1) == RED);
    CHECK(at(a, 2, 1) == RED);
    CHECK(at(a, 0, 0) == WHITE);
    CHECK(at(a, 3, 3) == WHITE);

    shim_dc_fill(a, 7, 3, 4, 4, 0); /* clipped fill, no overflow (ASan) */
    CHECK(at(a, 7, 3) == 0);
    CHECK(at(a, 4, 0) == WHITE);

    /* Negative-origin blit: only src pixels b(-1..6,-1..6) inside b(4x2)
     * AND dst (2+i,-2+j) inside a(8x4) land: i in 1..4, j == 2 ->
     * a(3,0)..a(6,0) = red. */
    shim_dc_blt(a, 2, -2, 8, 8, b, -1, -1, ROP_SRCCOPY);
    CHECK(at(a, 3, 0) == RED);
    CHECK(at(a, 4, 0) == RED);
    CHECK(at(a, 5, 0) == RED);
    CHECK(at(a, 6, 0) == RED);
    CHECK(at(a, 7, 0) == WHITE); /* src x=4 out of b */
    CHECK(at(a, 2, 2) == WHITE); /* dst x=2 never reached at j=2 */
    CHECK(at(a, 3, 1) == WHITE); /* src y=2 out of b */
    CHECK(at(a, 1, 1) == RED);   /* earlier blit untouched */

    shim_dc_free(a);
    shim_dc_free(b);
}

/* NOTSRCCOPY (game "MASKPEN"), 32bpp -> 1bpp: the mask-strip pass.
 * 2x2 src with one white hole and three solid colors -> bits 0,1,1,1
 * (bit set iff the source pixel is NOT white = SHIM_MONO_ON). */
static void test_mask_strip_generation(void)
{
    HDC src = shim_dc_new(2, 2);
    HBITMAP mask = CreateBitmap(2, 2, 1, 1, NULL);
    HDC dc = CreateCompatibleDC(NULL);
    HGDIOBJ def;
    CHECK(src && mask && dc);

    shim_dc_fill(src, 0, 0, 2, 2, WHITE);
    shim_dc_fill(src, 1, 0, 1, 1, RED);
    shim_dc_fill(src, 0, 1, 1, 1, BLUE);
    shim_dc_fill(src, 1, 1, 1, 1, GREEN);

    def = SelectObject(dc, mask);
    CHECK(def != NULL);
    CHECK(BitBlt(dc, 0, 0, 2, 2, src, 0, 0, ROP_NOTSRCCOPY));
    CHECK(shim_bmp_px(mask, 0, 0) == SHIM_MONO_OFF); /* white hole -> 0 */
    CHECK(shim_bmp_px(mask, 1, 0) == SHIM_MONO_ON);  /* solid red -> 1 */
    CHECK(shim_bmp_px(mask, 0, 1) == SHIM_MONO_ON);  /* solid blue -> 1 */
    CHECK(shim_bmp_px(mask, 1, 1) == SHIM_MONO_ON);  /* solid green -> 1 */

    SelectObject(dc, def);
    shim_dc_free(src);
    DeleteObject(mask);
    DeleteDC(dc);
}

/* SRCPAINT, 1bpp -> 32bpp: the canvas mask pass.
 * Bit 1 expands to white (paints), bit 0 to black (no-op). */
static void test_srcor_from_1bpp(void)
{
    HBITMAP mask = CreateBitmap(2, 2, 1, 1, NULL);
    HDC dc = CreateCompatibleDC(NULL);
    HDC src = shim_dc_new(2, 2);
    HGDIOBJ def;
    CHECK(mask && dc && src);

    shim_bmp_set_px(mask, 0, 0, WHITE); /* bit 1 */
    shim_bmp_set_px(mask, 1, 1, WHITE); /* bit 1 */
    /* (1,0) and (0,1) stay bit 0 */

    shim_dc_fill(src, 0, 0, 2, 2, 0);
    shim_dc_fill(src, 0, 0, 1, 1, RED);

    def = SelectObject(dc, mask);
    CHECK(def != NULL);
    CHECK(BitBlt(dc, 0, 0, 2, 2, src, 0, 0, ROP_SRCPAINT));
    CHECK(shim_bmp_px(mask, 0, 0) == WHITE); /* red | white */
    CHECK(shim_bmp_px(mask, 1, 0) == 0);     /* black | black */
    CHECK(shim_bmp_px(mask, 0, 1) == 0);
    CHECK(shim_bmp_px(mask, 1, 1) == WHITE); /* black | white */

    SelectObject(dc, def);
    shim_dc_free(src);
    DeleteObject(mask);
    DeleteDC(dc);
}

/* SRCAND, 32bpp -> 32bpp: the canvas image pass (C = C & I). */
static void test_srcand_32_32(void)
{
    HDC dst = shim_dc_new(2, 2);
    HDC src = shim_dc_new(2, 2);
    CHECK(dst && src);
    shim_dc_fill(dst, 0, 0, 2, 2, WHITE);
    shim_dc_fill(src, 0, 0, 2, 2, 0);
    shim_dc_fill(src, 0, 0, 1, 1, RED);
    shim_dc_fill(src, 1, 0, 1, 1, WHITE);
    shim_dc_fill(src, 0, 1, 1, 1, BLUE);

    CHECK(BitBlt(dst, 0, 0, 2, 2, src, 0, 0, ROP_SRCAND));
    CHECK(at(dst, 0, 0) == RED);   /* red & white */
    CHECK(at(dst, 1, 0) == WHITE); /* white & white */
    CHECK(at(dst, 0, 1) == BLUE);  /* blue & white */
    CHECK(at(dst, 1, 1) == 0);     /* black & white */

    shim_dc_free(dst);
    shim_dc_free(src);
}

/* SRCAND with a 1bpp source (defensive; the game never does this): the
 * bit expands to white (1) / black (0), then D' = D & S — bit 1 keeps the
 * dst pixel, bit 0 blacks it out. */
static void test_srcand_from_1bpp(void)
{
    HBITMAP mask = CreateBitmap(1, 2, 1, 1, NULL);
    HDC mdc = CreateCompatibleDC(NULL);
    HDC dst = shim_dc_new(1, 2);
    HGDIOBJ def;
    CHECK(mask && mdc && dst);

    shim_bmp_set_px(mask, 0, 0, WHITE); /* bit 1 */
    /* (0,1) stays bit 0 */
    shim_dc_fill(dst, 0, 0, 1, 1, RED);
    shim_dc_fill(dst, 0, 1, 1, 1, BLUE);

    def = SelectObject(mdc, mask);
    CHECK(def != NULL);
    CHECK(BitBlt(dst, 0, 0, 1, 2, mdc, 0, 0, ROP_SRCAND));
    CHECK(at(dst, 0, 0) == RED); /* red & white */
    CHECK(at(dst, 0, 1) == 0);   /* blue & black */

    SelectObject(mdc, def);
    shim_dc_free(dst);
    DeleteObject(mask);
    DeleteDC(mdc);
}

/* PatBlt WHITENESS 0xFF0062 (canvas/window clears): fills white, incl.
 * on 1bpp surfaces; clipped variant stays in bounds. */
static void test_patblt_whiteness(void)
{
    HDC a = shim_dc_new(4, 2);
    HBITMAP mask = CreateBitmap(3, 2, 1, 1, NULL);
    HDC md = CreateCompatibleDC(NULL);
    HGDIOBJ def;
    CHECK(a && mask && md);

    shim_dc_fill(a, 0, 0, 4, 2, RED);
    shim_dc_fill(a, 2, 1, 2, 1, BLUE);
    CHECK(PatBlt(a, 0, 0, 4, 2, ROP_WHITENESS));
    CHECK(at(a, 0, 0) == WHITE && at(a, 3, 1) == WHITE);
    CHECK(PatBlt(a, 3, 1, 8, 8, ROP_WHITENESS)); /* clipped, no overflow */
    CHECK(at(a, 0, 0) == WHITE);

    def = SelectObject(md, mask);
    CHECK(def != NULL);
    CHECK(PatBlt(md, 0, 0, 3, 2, ROP_WHITENESS));
    CHECK(shim_bmp_px(mask, 0, 0) == SHIM_MONO_ON && shim_bmp_px(mask, 2, 1) == SHIM_MONO_ON);

    SelectObject(md, def);
    shim_dc_free(a);
    DeleteObject(mask);
    DeleteDC(md);
}

/* PatBlt BLACKNESS 0x000042 (defensive; real-GDI value). */
static void test_patblt_blackness(void)
{
    HDC a = shim_dc_new(2, 2);
    CHECK(a);
    shim_dc_fill(a, 0, 0, 2, 2, WHITE);
    CHECK(PatBlt(a, 0, 0, 2, 2, ROP_BLACKNESS));
    CHECK(at(a, 0, 0) == 0 && at(a, 1, 1) == 0);
    shim_dc_free(a);
}

/* SelectObject: swap semantics + return of the previous object; the
 * blit reads from the currently selected bitmap. */
static void test_select_object_swap(void)
{
    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP b1 = CreateCompatibleBitmap(dc, 3, 2);
    HBITMAP b2 = CreateCompatibleBitmap(dc, 3, 2);
    HGDIOBJ old;
    CHECK(dc && b1 && b2);

    old = SelectObject(dc, b1);
    CHECK(old != NULL);            /* fresh DC: default 1x1, not NULL */
    CHECK(old != (HGDIOBJ)b1);
    old = SelectObject(dc, b2);
    CHECK(old == (HGDIOBJ)b1);
    old = SelectObject(dc, b1);
    CHECK(old == (HGDIOBJ)b2);

    shim_dc_fill(dc, 0, 0, 3, 2, RED); /* fills b1 (selected) */
    SelectObject(dc, b2);
    shim_dc_fill(dc, 0, 0, 3, 2, BLUE); /* fills b2 */
    CHECK(shim_bmp_px(b1, 1, 1) == RED);
    CHECK(shim_bmp_px(b2, 1, 1) == BLUE);

    /* Deleting a selected bitmap unselects it (no dangling reads). */
    DeleteObject(b2);
    SelectObject(dc, b1);
    CHECK(shim_bmp_px(b1, 1, 1) == RED);

    DeleteObject(b1);
    DeleteDC(dc);
}

/* GetObjectA: the 24-byte BITMAP prefix (the game's sizing pass,
 * ski_core.c:250), 1bpp GDI row padding, and rejection of non-bitmaps. */
static void test_get_object(void)
{
    HBITMAP m1 = CreateBitmap(5, 3, 1, 1, NULL);
    HBITMAP m2 = CreateCompatibleBitmap(NULL, 5, 3);
    HDC dc = shim_dc_new(2, 2);
    BITMAP bm;
    CHECK(m1 && m2 && dc);

    CHECK(GetObjectA(m1, sizeof(BITMAP), &bm));
    CHECK(bm.bmWidth == 5 && bm.bmHeight == 3);
    CHECK(bm.bmWidthBytes == 4); /* (5+31)&~31 bits = 4 bytes */
    CHECK(bm.bmPlanes == 1 && bm.bmBitsPixel == 1);

    CHECK(GetObjectA(m2, sizeof(BITMAP), &bm));
    CHECK(bm.bmWidth == 5 && bm.bmHeight == 3);
    CHECK(bm.bmWidthBytes == 20);
    CHECK(bm.bmPlanes == 1 && bm.bmBitsPixel == 32);

    CHECK(!GetObjectA(dc, sizeof(BITMAP), &bm));
    CHECK(!GetObjectA(m2, (int)sizeof(BITMAP) - 1, &bm));

    shim_dc_free(dc);
    DeleteObject(m1);
    DeleteObject(m2);
}

/* CreateBitmap: zeroed 1bpp strips (the M2 mask-strip precondition) and
 * strict plane/bpp acceptance. */
static void test_create_bitmap(void)
{
    HBITMAP m = CreateBitmap(10, 2, 1, 1, NULL);
    CHECK(m);
    CHECK(shim_bmp_bpp(m) == 1 && shim_bmp_w(m) == 10 && shim_bmp_h(m) == 2);
    CHECK(shim_bmp_stride(m) == 4);
    for (int y = 0; y < 2; y++)
        for (int x = 0; x < 10; x++)
            CHECK(shim_bmp_px(m, x, y) == SHIM_MONO_OFF);
    DeleteObject(m);

    CHECK(CreateBitmap(10, 2, 1, 24, NULL) == NULL);
    CHECK(CreateBitmap(0, 2, 1, 1, NULL) == NULL);
    CHECK(CreateBitmap(10, 2, 2, 1, NULL) == NULL);
}

/* Fill into a 1bpp-selected bitmap: the ON/OFF threshold. */
static void test_fill_1bpp(void)
{
    HBITMAP m = CreateBitmap(3, 1, 1, 1, NULL);
    HDC dc = CreateCompatibleDC(NULL);
    HGDIOBJ def;
    CHECK(m && dc);
    def = SelectObject(dc, m);
    CHECK(def != NULL);
    shim_dc_fill(dc, 0, 0, 3, 1, SHIM_MONO_ON);
    CHECK(shim_bmp_px(m, 0, 0) == SHIM_MONO_ON && shim_bmp_px(m, 2, 0) == SHIM_MONO_ON);
    shim_dc_fill(dc, 1, 0, 1, 1, SHIM_MONO_OFF);
    CHECK(shim_bmp_px(m, 0, 0) == SHIM_MONO_ON);
    CHECK(shim_bmp_px(m, 1, 0) == SHIM_MONO_OFF);
    CHECK(shim_bmp_px(m, 2, 0) == SHIM_MONO_ON);
    SelectObject(dc, def);
    DeleteObject(m);
    DeleteDC(dc);
}

int main(void)
{
    test_plan_cases();
    test_mask_strip_generation();
    test_srcor_from_1bpp();
    test_srcand_32_32();
    test_srcand_from_1bpp();
    test_patblt_whiteness();
    test_patblt_blackness();
    test_select_object_swap();
    test_get_object();
    test_create_bitmap();
    test_fill_1bpp();
    if (failures == 0)
        printf("surface tests PASS\n");
    else
        printf("%d surface test(s) FAILED\n", failures);
    return failures ? 1 : 0;
}
