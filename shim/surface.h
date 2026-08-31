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
 *   0x00EE0086 SRCPAINT   D' = S | D     (the game's "SRCOR" blit)
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
 *   - 32bpp -> 1bpp (NOTSRCCOPY): a bit is set iff the source pixel is
 *     NOT SHIM_MONO_ON (the blit inverts the "is background color" bit),
 *   - 1bpp -> 32bpp (SRCPAINT et al.): bit 1 expands to SHIM_MONO_ON,
 *     bit 0 expands to SHIM_MONO_OFF.
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

#endif /* SHIM_SURFACE_H */
