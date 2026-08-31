/* JS canvas bridge + debug hooks (T18). 2D canvas only (no WebGL).
 *
 * The browser canvas is the reference screen: 1024x768, with the main
 * window client at (132,30) exactly where the reference Xvfb placed it
 * (evidence/m0-geometry.txt). Child windows composite on top of their
 * parent's client at their child coordinates — in the reference the
 * status panel (a Win32 child) is painted inside the main window's
 * client area, top-right (evidence/m0-geometry.txt), so T21's
 * ski_window_png(0) capture of the main client includes it.
 *
 * Debug hooks (API.md "Debug hooks", EMSCRIPTEN_KEEPALIVE — exported on
 * the module object by -sEXPORT_KEEPALIVE):
 *   ski_set_input(bytes, n)  per-tick input array, same 2-byte/tick
 *                            layout as harness/gen_input.py (little-endian
 *                            u16; bit i = one WM_KEYDOWN of key i at the
 *                            start of tick i+1). The SKI_HARNESS build's
 *                            input reader (src/ski_core.c, wired in T21)
 *                            consumes g_ski_in when set instead of
 *                            fopen("ski_in.bin").
 *   ski_tick_get()           ticks completed (== g_ski_tick; the T21
 *                            mjs polls it to step exactly N ticks).
 *   ski_window_png(n)        PNG dataURL of window n's client framebuffer
 *                            (own surface + visible child overlays).
 *   ski_key_event(vk, down)  interactive key -> WM_KEYDOWN (+ WM_CHAR for
 *                            printable VCs; the binary has no WM_KEYUP
 *                            path, so up events are ignored).
 *   ski_click(x, y)          canvas coords -> WM_LBUTTONDOWN/UP on the
 *                            window whose client contains the point.
 */
#include <emscripten.h>
#include <emscripten/em_js.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "win.h"
#include "surface.h"

/* ---- JS primitives ------------------------------------------------------ */
/* Upload a tight w*h*4 RGBA block (heap ptr; alpha 0xFF) to the 2D canvas
 * at (x, y). The canvas is created/sized on first use: 1024x768 (the
 * reference screen), black desktop. */
EM_JS(void, canvas_put, (int x, int y, int w, int h, int ptr),
{
  const cv = document.getElementById('canvas');
  if (!cv) return;
  let ctx = cv.getContext('2d');
  if (cv.width !== 1024 || cv.height !== 768) {
    cv.width = 1024;
    cv.height = 768;
    ctx = cv.getContext('2d');
    ctx.fillStyle = '#000000';
    ctx.fillRect(0, 0, 1024, 768);
  }
  const u8 = new Uint8Array(Module.HEAPU8.buffer, ptr, w * h * 4);
  const img = new ImageData(
      new Uint8ClampedArray(u8.buffer, ptr, w * h * 4), w, h);
  ctx.putImageData(img, x, y);
});

/* PNG dataURL of a tight w*h*4 RGBA heap block (offscreen 2D canvas). */
EM_JS(const char *, canvas_png, (int w, int h, int ptr),
{
  const cv = document.createElement('canvas');
  cv.width = w;
  cv.height = h;
  const ctx = cv.getContext('2d');
  const u8 = new Uint8Array(Module.HEAPU8.buffer, ptr, w * h * 4);
  const img = new ImageData(
      new Uint8ClampedArray(u8.buffer, ptr, w * h * 4), w, h);
  ctx.putImageData(img, 0, 0);
  return cv.toDataURL('image/png');
});

/* ---- compositing ---------------------------------------------------------- */
static uint8_t *g_comp;
static size_t g_comp_n;

/* Window w's client framebuffer: its own surface (clipped to the client —
 * child surfaces are over-allocated, see mkwin in win.c) plus visible
 * child surfaces at their child coordinates. Returns a tight cw*ch*4 RGBA
 * heap block (reused buffer). */
static uint8_t *window_composite(const ShimWin *w, int *pw, int *ph)
{
    const uint8_t *src;
    int sw, sh, cw, ch, y, i;
    if (!w->dc)
        return NULL;
    shim_dc_size(w->dc, &sw, &sh);
    shim_dc_px(w->dc, &src);
    if (!src)
        return NULL;
    cw = w->cw < sw ? w->cw : sw;
    ch = w->ch < sh ? w->ch : sh;
    if (cw <= 0 || ch <= 0)
        return NULL;
    if ((size_t)cw * ch * 4 > g_comp_n) {
        uint8_t *p = realloc(g_comp, (size_t)cw * ch * 4);
        if (!p)
            return NULL;
        g_comp = p;
        g_comp_n = (size_t)cw * ch * 4;
    }
    for (y = 0; y < ch; y++)
        memcpy(g_comp + (size_t)y * cw * 4, src + (size_t)y * sw * 4,
               (size_t)cw * 4);
    for (i = 0; i < shim_window_count(); i++) {
        const ShimWin *c = shim_window(i);
        const uint8_t *cpx;
        int csw, csh, x;
        if (!c->parent || c->parent != w || !c->dc)
            continue;
        if (!c->visible || c->minimized)
            continue;
        shim_dc_size(c->dc, &csw, &csh);
        shim_dc_px(c->dc, &cpx);
        if (!cpx)
            continue;
        for (y = 0; y < c->ch && y < csh; y++) {
            int dy = c->y + y;
            if (dy < 0 || dy >= ch)
                continue;
            for (x = 0; x < c->cw && x < csw; x++) {
                int dx = c->x + x;
                if (dx < 0 || dx >= cw)
                    continue;
                memcpy(g_comp + ((size_t)dy * cw + (size_t)dx) * 4,
                       cpx + ((size_t)y * csw + (size_t)x) * 4, 4);
            }
        }
    }
    *pw = cw;
    *ph = ch;
    return g_comp;
}

/* Upload every visible top-level window's client (with child overlays) to
 * the canvas at its screen position. */
void canvas_flush(void)
{
    int i, j;
    for (i = 0; i < shim_window_count(); i++) {
        ShimWin *w = shim_window(i);
        const uint8_t *px;
        int sw, sh, cw, ch, sx, sy;
        if (w->child || !w->visible || w->minimized || !w->dc)
            continue;
        shim_dc_size(w->dc, &sw, &sh);
        shim_dc_px(w->dc, &px);
        if (!px)
            continue;
        cw = w->cw < sw ? w->cw : sw;
        ch = w->ch < sh ? w->ch : sh;
        if (cw <= 0 || ch <= 0)
            continue;
        sx = w->x + w->cx;
        sy = w->y + w->cy;
        if (sw == cw && sh == ch) {
            canvas_put(sx, sy, cw, ch, (int)(uintptr_t)px);
        } else {
            int pw, ph;
            uint8_t *c = window_composite(w, &pw, &ph);
            if (c)
                canvas_put(sx, sy, pw, ph, (int)(uintptr_t)c);
        }
        /* child overlays (the status panel lives inside the parent client) */
        for (j = 0; j < shim_window_count(); j++) {
            ShimWin *c = shim_window(j);
            const uint8_t *cpx;
            int csw, csh, ccw, cch;
            if (c->parent != w || !c->dc || !c->visible || c->minimized)
                continue;
            shim_dc_size(c->dc, &csw, &csh);
            shim_dc_px(c->dc, &cpx);
            if (!cpx)
                continue;
            ccw = c->cw < csw ? c->cw : csw;
            cch = c->ch < csh ? c->ch : csh;
            if (ccw <= 0 || cch <= 0)
                continue;
            if (csw == ccw && csh == cch) {
                canvas_put(sx + c->x, sy + c->y, ccw, cch,
                           (int)(uintptr_t)cpx);
            } else {
                int pw, ph;
                uint8_t *cc = window_composite(c, &pw, &ph);
                if (cc)
                    canvas_put(sx + c->x, sy + c->y, pw, ph,
                               (int)(uintptr_t)cc);
            }
        }
    }
}

/* ---- debug hooks ---------------------------------------------------------- */
/* Linkable globals (NOT static): the SKI_HARNESS input reader in
 * src/ski_core.c references these from T21 on. */
const unsigned char *g_ski_in = NULL;
int g_ski_in_n = 0;

EMSCRIPTEN_KEEPALIVE void ski_set_input(const unsigned char *b, int n)
{
    g_ski_in = b;
    g_ski_in_n = n;
}

EMSCRIPTEN_KEEPALIVE int ski_tick_get(void)
{
    /* == g_ski_tick (ski_core.c): the timer is armed exactly while the
     * game is active, so every dispatch is one completed tick. */
    return shim_ticks_fired();
}

EMSCRIPTEN_KEEPALIVE const char *ski_window_png(int n)
{
    ShimWin *w = shim_window(n);
    int pw, ph;
    uint8_t *c;
    if (!w)
        return "";
    c = window_composite(w, &pw, &ph);
    if (!c)
        return "";
    return canvas_png(pw, ph, (int)(uintptr_t)c);
}

EMSCRIPTEN_KEEPALIVE void ski_key_event(unsigned vk, int down)
{
    ShimWin *w = shim_window(0); /* the main window takes all keys */
    if (!w || !down)
        return; /* the binary has no WM_KEYUP path: up events are no-ops */
    shim_post((HWND)w, WM_KEYDOWN, (unsigned long)vk, 0);
    /* The game's char handler (ski_char_key) takes lowercase codes only;
     * derive the char from the VK (A-Z: vk|0x20, 0-9: vk+0x1E). Posting
     * the raw VK as char would, e.g. map 'r' to 0x52 ('R', unhandled)
     * while passing the char code would make WM_KEYDOWN hit the VK_F3
     * pause case (ski_win.c:266). */
    if (vk >= 'A' && vk <= 'Z')
        shim_post((HWND)w, WM_CHAR, (unsigned long)(vk | 0x20), 0);
    else if (vk >= '0' && vk <= '9')
        shim_post((HWND)w, WM_CHAR, (unsigned long)(vk + 0x1E), 0);
}

EMSCRIPTEN_KEEPALIVE void ski_click(unsigned x, unsigned y)
{
    int i;
    for (i = 0; i < shim_window_count(); i++) {
        ShimWin *w = shim_window(i);
        int lx = (int)x - (w->x + w->cx);
        int ly = (int)y - (w->y + w->cy);
        unsigned long lp;
        if (w->child || !w->visible || w->minimized)
            continue;
        if (lx < 0 || ly < 0 || lx >= w->cw || ly >= w->ch)
            continue;
        lp = (unsigned long)(((unsigned)(unsigned short)lx) |
                             ((unsigned)(unsigned short)ly << 16));
        shim_post((HWND)w, WM_LBUTTONDOWN, 1, lp);
        shim_post((HWND)w, WM_LBUTTONUP, 1, lp);
        return;
    }
}
