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
 *   ski_window_png(n, buf, cap)  PNG dataURL of window n's client
 *                            framebuffer (own surface + visible child
 *                            overlays), written as UTF-8 into buf
 *                            (NUL-terminated, at most cap bytes);
 *                            returns the length in bytes (0 on error).
 *   ski_key_event(vk, down)  interactive key -> WM_KEYDOWN (+ WM_CHAR for
 *                            printable VCs; the binary has no WM_KEYUP
 *                            path, so up events are ignored).
 *   ski_click(x, y)          canvas coords -> WM_LBUTTONDOWN/UP on the
 *                            window whose client contains the point.
 *   ski_mouse_move(x, y)     canvas coords -> WM_MOUSEMOVE on the topmost
 *                            window (children included) whose client
 *                            contains the point — the game's mouse
 *                            steering (ski_mouse_aim, ski_win.c:431).
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

/* PNG dataURL of a tight w*h*4 RGBA heap block (offscreen 2D canvas).
 * The dataURL crosses via a C buffer: on emcc 6.0.6 a JS string
 * returned as 'const char*' marshals to NULL (T20-review Chrome probe:
 * _ski_window_png(0) returned 0; repros /tmp/t20review/emjs) — so JS
 * writes UTF-8 into buf (cap bytes) and returns the length (ints
 * marshal fine both ways). */
EM_JS(int, canvas_png, (int w, int h, int ptr, char *buf, int cap),
{
  const cv = document.createElement('canvas');
  cv.width = w;
  cv.height = h;
  const ctx = cv.getContext('2d');
  const u8 = new Uint8Array(Module.HEAPU8.buffer, ptr, w * h * 4);
  const img = new ImageData(
      new Uint8ClampedArray(u8.buffer, ptr, w * h * 4), w, h);
  ctx.putImageData(img, 0, 0);
  const s = cv.toDataURL('image/png');
  if (cap > 0)
    stringToUTF8(s, buf, cap);
  /* The dataURL is pure ASCII ("data:image/png;base64," + base64), so
   * s.length is the exact UTF-8 byte length — lengthBytesUTF8 is NOT
   * in EM_JS scope on emcc 6.0.6 (T20-fix web probe). */
  return s.length;
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

/* ---- T21 frame seal (WASM diff build) --------------------------------------
 * The SKI_HARNESS capture (ski_harness_frame, src/ski_core.c) fires at
 * the TOP of every ski_tick: frame k = main-window client after tick
 * body k, BEFORE word k is injected — the exact game state the native
 * harness dumps as frame_%06d_main.ppm, so a seal at this call point
 * makes WASM frame k == rebuild frame k (diff.py --shift 0). On WASM
 * the harness mjs owns the file writes, so the game-side capture seals
 * the client composite (own surface + visible child overlays — the
 * status panel; the panel region is inside diff.py's mask, and the
 * native BitBlt's parent-DC content there is the stale parent backing
 * anyway) into this ring and bumps the counter; the mjs polls
 * _ski_frame_get() and pulls each sealed frame as a PNG dataURL
 * (_ski_frame_pull). The ring is 4 deep: a CDP pull can lag up to 4
 * ticks (160 ms at the 40 ms real-time cadence) without overwriting
 * an un-pulled frame. Seal = one composite + memcpy at tick time
 * (~2.2 MB, sub-millisecond); the PNG encode happens on pull, so the
 * tick path stays fast and the virtual clock — a pure function of tick
 * history — is untouched by capture latency.
 *
 * Compiled only in the SKI_HARNESS diff build: the ring (4 x 2.2 MB)
 * is dead static mass in the shipping web build (HARNESS=OFF), where
 * the game never calls the seal, and the debug exports must not ship.
 */
#if SKI_HARNESS
#define SKI_SEAL_RING 4
static uint8_t *g_seal; /* SKI_SEAL_RING * w * h * 4 (RGBA, top-down) */
static int g_seal_w, g_seal_h;
static int g_seal_n;    /* frames sealed since boot; frame idx == k seals at n=k+1 */

void ski_shim_frame_seal(void)
{
    ShimWin *w = shim_window(0); /* the main window (g_c6c8) */
    int pw, ph;
    const uint8_t *c;
    size_t npx;
    if (!w)
        return;
    c = window_composite(w, &pw, &ph);
    if (!c)
        return;
    npx = (size_t)pw * (size_t)ph * 4;
    if (g_seal_w != pw || g_seal_h != ph) {
        /* The client is fixed for the run (the game never resizes a
         * top-level); a realloc here would only rebase on a surprise. */
        g_seal = realloc(g_seal, (size_t)SKI_SEAL_RING * npx);
        if (!g_seal)
            return;
        g_seal_w = pw;
        g_seal_h = ph;
        g_seal_n = 0;
    }
    memcpy(g_seal + (size_t)(g_seal_n % SKI_SEAL_RING) * npx, c, npx);
    g_seal_n++;
}

EMSCRIPTEN_KEEPALIVE int ski_frame_get(void)
{
    return g_seal_n; /* sealed frames so far (== frame index + 1) */
}

/* PNG dataURL of sealed frame idx (0-based == the rebuild's
 * frame_%06d index) into buf (at most cap bytes, NUL-terminated);
 * returns the length in bytes, 0 if idx was never sealed or has been
 * evicted from the ring (idx < g_seal_n - SKI_SEAL_RING). */
EMSCRIPTEN_KEEPALIVE int ski_frame_pull(int idx, char *buf, int cap)
{
    size_t npx;
    const uint8_t *s;
    if (!g_seal || cap <= 0)
        return 0;
    if (idx < 0 || idx >= g_seal_n || idx < g_seal_n - SKI_SEAL_RING)
        return 0;
    npx = (size_t)g_seal_w * (size_t)g_seal_h * 4;
    s = g_seal + (size_t)(idx % SKI_SEAL_RING) * npx;
    return canvas_png(g_seal_w, g_seal_h, (int)(uintptr_t)s, buf, cap);
}
#endif

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

/* Writes the window's PNG dataURL into buf (at most cap bytes,
 * NUL-terminated) and returns its length in bytes (0 on error — no
 * such window / no surface / cap <= 0). The dataURL of the 760x734
 * main client is ~35 KB (T20-fix web probe: 35142); the caller (the
 * T21 harness) _malloc's the buffer and reads it back with
 * Module.UTF8ToString (both exported — CMakeLists build notes). */
EMSCRIPTEN_KEEPALIVE int ski_window_png(int n, char *buf, int cap)
{
    ShimWin *w = shim_window(n);
    int pw, ph;
    uint8_t *c;
    if (!w || cap <= 0)
        return 0;
    c = window_composite(w, &pw, &ph);
    if (!c)
        return 0;
    return canvas_png(pw, ph, (int)(uintptr_t)c, buf, cap);
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

/* Interactive mouse move -> WM_MOUSEMOVE on the window whose client
 * contains the point (canvas coords are reference-screen space, same
 * convention as ski_click). The game's WndProc routes it to
 * ski_mouse_aim (ski_win.c:649-653, client coords in lParam) — that is
 * the original's mouse steering. Z-order: the status child sits ABOVE
 * the parent and is created after it, so scan the table backwards and
 * (unlike ski_click) do NOT skip children — a move over the panel is
 * eaten by the child proc (default -> DefWindowProc, no-op,
 * ski_win.c:751-765), exactly as under wine. The game's own g_c67c
 * gate (input only while active) applies inside ski_mouse_aim's
 * caller; posting while inactive is a faithful no-op. */
EMSCRIPTEN_KEEPALIVE void ski_mouse_move(unsigned x, unsigned y)
{
    int i;
    for (i = shim_window_count() - 1; i >= 0; i--) {
        ShimWin *w = shim_window(i);
        int lx = (int)x - (w->x + w->cx);
        int ly = (int)y - (w->y + w->cy);
        unsigned long lp;
        if (!w->visible || w->minimized)
            continue;
        if (lx < 0 || ly < 0 || lx >= w->cw || ly >= w->ch)
            continue;
        lp = (unsigned long)(((unsigned)(unsigned short)lx) |
                             ((unsigned)(unsigned short)ly << 16));
        shim_post((HWND)w, WM_MOUSEMOVE, 0, lp);
        return;
    }
}
