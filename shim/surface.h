/* Shim core: the GDI surface model (DCs + bitmaps) and the blit/ROP core.
 *
 * Pixel model: 32bpp surfaces are row-major RGBA bytes (4 per pixel; the
 * alpha byte is always 0xFF — the visible pipeline is RGB-only, matching
 * the native reference's 24bpp DIBs after RGB conversion). Colors travel
 * as Win32 COLORREFs (0x00BBGGRR). 1bpp surfaces are MSB-first DIB bits
 * with rows padded to a 4-byte stride (the GDI layout);
 * CreateBitmap(w,h,1,1,NULL) zeroes them, like the reference (wine 9.0)
 * GDI does.
 *
 * ROP set — the five values the game passes (values authoritative from
 * the decompilation; names from the real Win32 ROP table, cross-checked
 * against the mingw-w64 SDK wingdi.h and wine 9.0's R2 decoder):
 *   0x00CC0020 SRCCOPY    D' = S
 *   0x00330008 NOTSRCCOPY D' = ~S        (the game's "MASKPEN" blit)
 *   0x008800C6 SRCAND     D' = S & D
 *   0x00EE0086 SRCPAINT   D' = S | D
 *   0x00FF0062 WHITENESS  D' = 1         (PatBlt canvas/window clears)
 * The decompiled comments mislabeled 0xFF0062 "BLACKNESS"; in the real
 * ROP table 0x000042 is BLACKNESS and 0xFF0062 is WHITENESS (mingw SDK
 * wingdi.h). Both are handled, so the canvas clears come out white —
 * which is what the M2 reference (wine GDI) produced.
 *
 * 1bpp mono pipeline (sprite -> mask strip -> canvas chain): ground truth
 * is wine 9.0's dibdrv, which the M2 reference ran on (0-px frame parity
 * against the original). Its effective semantics: the mono DC pair has
 * background = white, text = black, so
 *   - color -> 1bpp (NOTSRCCOPY): a bit is set iff the source pixel is
 *     NOT SHIM_MONO_ON (the blit inverts the "is background color" bit),
 *   - 1bpp -> 32bpp (SRCPAINT et al.): bit 1 expands to SHIM_MONO_ON,
 *     bit 0 expands to SHIM_MONO_OFF.
 * The color->1bpp rule was measured for 32bpp sources in T17
 * (probe_mono.c Q1: [white,red,black,blue] -> bits 0 1 1 1). The sprite
 * sources are 4bpp indexed DIBs in the PE; T20 probed the 4bpp case
 * directly (real RT_BITMAP 4bpp DIBs + LoadBitmapA + NOTSRCCOPY, wine
 * 9.0, /tmp/t20probe; discriminators and results in the
 * harness/embed_sprites.py header): the bit is set iff the
 * PALETTE-EXPANDED color is not white — the index is never compared
 * (a second white palette entry also maps to 0; index-0 and
 * index-of-white rules are both rejected). Wine also converts the
 * loaded 4bpp DIB to a 32bpp device bitmap (GetObjectA reports 32), so
 * the shim stores sprites pre-expanded (shim_bmp_from_rgb) and the
 * single color->1bpp rule above implements both cases exactly.
 * SHIM_MONO_ON is the single polarity knob: if the T21 WASM-vs-rebuild
 * diff shows the masks inverted, flip it there (SHIM_MONO_OFF derives
 * from it, so both directions flip together).
 */
#ifndef SHIM_SURFACE_H
#define SHIM_SURFACE_H
#include "win32.h"

#define SHIM_MONO_ON  0x00FFFFFFu /* color bit 1 expands to; 32bpp->1bpp threshold */
#define SHIM_MONO_OFF ((SHIM_MONO_ON) ? 0x000000u : 0x00FFFFFFu)

/* Low-level surface helpers (tests, and the win.c/canvas.c layers). A
 * shim_dc_new DC owns its RGBA surface and starts with nothing selected;
 * a CreateCompatibleDC DC has no own surface and starts with the shared
 * default 1x1 black bitmap selected (real-GDI behavior: SelectObject on a
 * fresh memory DC returns a non-NULL previous object — the game's sprite
 * loader treats a NULL return as failure, ski_core.c:268). */
HDC        shim_dc_new(int w, int h);
void       shim_dc_free(HDC dc);

/* Draw-hook: win.c registers a callback that fires after any DC mutation
 * (core_fill / core_blt / TextOutA). It emulates real Win32's "drawing to a
 * window's DC invalidates the window" — the game draws its status-panel
 * values straight onto the status DC (ski_status_draw_values -> TextOutA,
 * ski_core.c:1601/2675) OUTSIDE a WM_PAINT, and under wine 9.0 that draw
 * invalidates the child so the next WM_PAINT erases + redraws it clean
 * (verified: original s03 frame 500/700 panels are crisp after hundreds of
 * value changes). Without it the shim's status panel accumulates subpixel-AA
 * text and smears. No-op until win.c calls shim_set_draw_hook. */
void       shim_set_draw_hook(void (*hook)(HDC dc));
void       shim_dc_mutated(HDC dc);
void       shim_dc_size(HDC dc, int *w, int *h);
void       shim_dc_px(const HDC dc, const uint8_t **px); /* own surface; NULL if none */
void       shim_dc_fill(HDC dc, int x, int y, int w, int h, COLORREF c);
void       shim_dc_blt(HDC dst, int dx, int dy, int w, int h,
                       HDC src, int sx, int sy, uint32_t rop);

/* Bitmap accessors (tests + the T21 frame-dump path). */
int        shim_bmp_w(HBITMAP b);
int        shim_bmp_h(HBITMAP b);
int        shim_bmp_bpp(HBITMAP b); /* 1 or 32 */
int        shim_bmp_stride(HBITMAP b); /* bytes per row, GDI padding */
uint32_t   shim_bmp_px(HBITMAP b, int x, int y); /* COLORREF; 1bpp: ON/OFF */
void       shim_bmp_set_px(HBITMAP b, int x, int y, uint32_t c); /* 1bpp: c==ON sets the bit */

/* New 32bpp bitmap from palette-expanded RGB rows (the embedded sprites,
 * misc.c LoadBitmapA). The sprite resources are 4bpp indexed DIBs in the
 * PE, but the reference's wine 9.0 converts a loaded resource DIB to a
 * 32bpp device bitmap (T20 probe: GetObjectA reports bpp=32 after
 * LoadBitmapA of a 4bpp RT_BITMAP), so the shim stores them pre-expanded;
 * the NOTSRCCOPY 4bpp->1bpp mask pass is then exactly the 32bpp->1bpp
 * "not white" rule above (T17 + T20 probe evidence in the embed script
 * header, harness/embed_sprites.py). */
HBITMAP    shim_bmp_from_rgb(int w, int h, const uint8_t *rgb);

#endif /* SHIM_SURFACE_H */
