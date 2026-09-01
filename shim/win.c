/* Win32 window / message / timer emulation for emscripten (T18).
 *
 * Model — mirrors the reference run's observed behavior (wine 9.0 + Xvfb
 * 1024x768; the ground truth the M1/M2 frame diffs were validated against):
 *
 *  - CreateWindowExA dispatches WM_NCCREATE + WM_CREATE synchronously, like
 *    real Win32: the WndProc runs inside the call, so wm_create_main (sprite
 *    load, g_c63c) completes before ski_create_windows moves on.
 *  - ShowWindow dispatches WM_MOUSEACTIVATE / WM_ACTIVATE / WM_SIZE
 *    synchronously in the reference boot order (decompile/NOTES.md "Boot
 *    sequence" message trace) and marks the client dirty; a single WM_PAINT
 *    is then delivered by the pump once the queue drains (Windows'
 *    low-priority paint).
 *  - MoveWindow dispatches WM_SIZE / WM_MOVE synchronously on geometry
 *    change and marks the window dirty (status_reposition path).
 *  - InvalidateRect marks dirty (the game always passes NULL rc -> full
 *    client); there is no WM_ERASEBKGND pass: painting is
 *    dirty-driven. (The decompiled class records carry
 *    hbrBackground = GetStockObject(0), which under the reference's
 *    wine 9.0 is the WHITE brush — erase-white, matching the
 *    white-initial surface below; T19 review, 66993e1.)
 *  - Timers: the rAF main loop's scheduler posts the game's 40 ms callback
 *    timer (id 0x29a) at a fixed real-time cadence with catch-up after
 *    throttling. The virtual clock (GetTickCount) advances by exactly the
 *    period at each timer DISPATCH: it is a pure function of tick history,
 *    never of wall time (determinism — the SKI_DETERMINISTIC build derives
 *    now = c698 + 40 per tick; only clock DELTAS ever reach rendered
 *    values, and those are 40*k under the virtual clock exactly as under
 *    the deterministic reference's wall clock).
 *  - Modals: MessageBoxA does NOT stop the pump. Wine 9.0 (the contract)
 *    keeps the main window's callback timer firing AND its WM_PAINTs
 *    delivering while a modal box is up — the world keeps ticking and
 *    painting behind it (controlled probe: /tmp/t20review/
 *    modal_probe2.log, ticks 6..145 + paints 16..144 all with box_up=1).
 *    The box is pump-level state (shim_modal_raise /
 *    ski_messagebox_answer); the dialog itself is a separate window that
 *    never appears in ski_window_png captures. Input is dropped while
 *    the box is up (the dialog owns it — see mq_pop). Full design and
 *    the empirically rejected emscripten_sleep alternatives: the modal
 *    note below.
 *  - The game's own WinMain message loop is deliberately not pumped:
 *    GetMessageA returns 0 (quit) so WinMain exits cleanly — ski_cleanup()
 *    is a no-op in the web build (no sound loads: FindResourceA -> NULL) —
 *    and the rAF main loop does all dispatching.
 *
 * Geometry: the reference's 768x768 outer main window had a 760x734 client
 * at screen (132,30) — 4px borders left/right/bottom, 30px title+top
 * (evidence/m0-geometry.txt). The screen is 1024x768 (API.md GetDeviceCaps
 * parity: the reference ran under a 1024x768 Xvfb).
 *
 * Initial window surface: white. The decompiled class records carry
 * hbrBackground = NULL (GetStockObject(0) is out of the valid stock range
 * 5..14 -> NULL under real GDI), so GDI erases nothing; the reference's
 * initial surface color comes from the wine/Xvfb window backing and is
 * empirically white (evidence/m0-original-menu.png corners, and
 * evidence/m0-status-window.png is 98.7% white although status_paint draws
 * only text — no fill ever touches the status DC). Pre-fill white matches
 * the reference; it is overwritten by the boot WM_PAINT before any frame
 * capture.
 */
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "win.h"
#include "surface.h"

#define WS_CHILD      0x40000000u
#define WM_MOVE       0x002A
#define SIZE_MINIMIZED 1

/* reference screen + non-client metrics (see header) */
#define SKI_SCREEN_W 1024
#define SKI_SCREEN_H 768
#define SKI_BX 4      /* left border */
#define SKI_BY 30     /* title bar + top border */
#define SKI_BW 8      /* left + right border */
#define SKI_BH 34     /* title/top + bottom border */

static ShimWin g_wins[SKI_WIN_MAX];

typedef struct { ShimWin *h; UINT msg; WPARAM wp; LPARAM lp; } MqMsg;
static MqMsg g_mq[SKI_MQ_CAP];
static int g_mq_r, g_mq_c; /* head index, occupied count (ring) */

typedef struct { const char *name; WNDPROC proc; HBRUSH bg; } ClassRec;
static ClassRec g_classes[8];
static int g_class_n;

static ShimWin *g_hmain;      /* the SkiMain window (timer target) */
static HDC      g_screen_dc;  /* GetDC(NULL) */
static int      g_loop_started;
static int      g_quit;

/* ---- MessageBoxA modal state (Task 20 rework — T20 review) -------------
 * Wine 9.0 (the contract) keeps the main window's callback timer
 * firing AND its WM_PAINTs delivering while a modal MessageBoxA is up —
 * the world keeps ticking and painting behind the box. Controlled probe
 * (i686-mingw under ~/.wine-ski, :99; /tmp/t20review/modal_probe2.c/.log):
 * a window-attached 40 ms callback timer whose handler raises
 * MessageBoxA — ticks 6..145 fire at full 40 ms cadence and WM_PAINT
 * 16..144 reaches the main WndProc, all with box_up=1; on close the
 * ticks continue seamlessly. The old T20 rationale "M2 s08: 0px while a
 * box is up" was misattributed — s08 contains NO modal: an instrumented
 * re-run (harness protocol) stalls at frame 300/1200 on the F3-pause
 * (/tmp/t20review/s08-orig/, 301 frames; frame 300 = active gameplay).
 * The shim does what wine does: while the box is up the rAF pump keeps
 * dispatching (timer fires advance the virtual clock one period each;
 * paints update the canvas). The dialog itself is a separate window
 * that never appears in ski_window_png captures, so there is nothing
 * else to render.
 *
 * Yield mechanism — the PUMP-FLAG + ANSWER-HOOK alternative. The
 * emscripten_sleep loop was tried and rejected EMPIRICALLY (emcc 6.0.6
 * probes, /tmp/t21fix/): (a) with -sASYNCIFY + emscripten_set_main_loop
 * a frame suspended in emscripten_sleep blocks the scheduling of the
 * NEXT rAF — 0 frames in 2.5 s of modal; the pump would freeze (the
 * very deviation this rework fixes); (b) with -sASYNCIFY + a JS rAF
 * driver that keeps frames arriving, the new frames clobber the
 * suspended frame's C-stack locals — single-threaded asyncify does not
 * preserve a suspended frame's stack region across a concurrent entry
 * (8 KB sentinel buffer corrupted; the game's box-text buffers would
 * corrupt on resume). A C call therefore cannot block: MessageBoxA
 * (misc.c) logs the box, shim_modal_raise records the state, and it
 * RETURNS IDOK immediately. The game's own post-box code (the
 * per-call-site epilogues, ski_core.c:141-152 — verified faithful)
 * then runs at raise time:
 *   score site (owner main, type 0 — ski_core.c:2812) and fatal site
 *     (owner NULL, type 0x30 — ski_core.c:158): post-box code is plain
 *     `return;` / `return 0` — unobservable;
 *   assert site (owner NULL, type 0x31 — ski_core.c:141, the only site
 *     that reads the return value): `if (r == 2) DestroyWindow(g_c6c8)`
 *     is skipped (r == 1 — the answer hook below replays it with the
 *     real answer); ski_pause_toggle() runs at raise time, so ticks
 *     stop behind that bug-only box (the old T20 observable) and the
 *     post-answer state matches wine for both answer values.
 * The harness closes the box with ski_messagebox_answer(r) (IDOK 1 /
 * IDCANCEL 2); it runs the per-site epilogue and the box state clears.
 *
 * Determinism: each timer fire is one mq_post + one dispatch1, and
 * dispatch1 advances the virtual clock by exactly ONE period per fire,
 * so game state stays a pure function of the tick count while a modal
 * is up (a frame may batch several fires only after real throttling —
 * hidden tab; no catch-up burst accumulates on close because the pump
 * never stopped). A second box raised while one is up overwrites the
 * state (both MessageBoxA calls have already returned; the answer
 * resolves the last-raised site) — no call site raises a box from code
 * that runs while a box is up: the section flags are cleared before
 * ski_score_show (ski_core.c:1636/1677/1713) and assert/fatal are
 * boot/bug-only paths.
 *
 * A box raised OUTSIDE the pump (the boot-time fatal path runs inside
 * WinMain, before the rAF loop exists) returns IDOK synchronously,
 * sets no modal state, and the caller's plain-return epilogues run
 * unmodified. */
static int     g_in_pump;      /* dispatching inside ski_mainloop */
static int     g_modal_active; /* box up (pending or answered) */
static int     g_modal_answered;
static int     g_modal_answer;
static int     g_modal_type;
static int     g_modal_is_assert;
static char    g_modal_text[512];
static char    g_modal_caption[128];

extern HWND g_c6c8;          /* src/ski_game.h:0x40c6c8 main window */

int shim_modal_raise(HWND owner, const char *text, const char *caption,
                     UINT type)
{
    if (!g_in_pump)
        return 1; /* boot-time box: IDOK, no modal state (see note) */
    g_modal_is_assert = (owner == NULL && type == 0x31);
    g_modal_type = (int)type;
    g_modal_active = 1;
    g_modal_answered = 0;
    g_modal_answer = 0;
    snprintf(g_modal_text, sizeof g_modal_text, "%s", text ? text : "");
    snprintf(g_modal_caption, sizeof g_modal_caption, "%s",
             caption ? caption : "");
    return 1; /* IDOK — the yield is pump-level: the pump keeps
                dispatching until ski_messagebox_answer (see note) */
}

int shim_modal_pending(void) { return g_modal_active && !g_modal_answered; }

int shim_modal_type(void) { return g_modal_active ? g_modal_type : 0; }

const char *shim_modal_text(void) { return g_modal_active ? g_modal_text : ""; }

const char *shim_modal_caption(void)
{
    return g_modal_active ? g_modal_caption : "";
}

EMSCRIPTEN_KEEPALIVE void ski_messagebox_answer(int r)
{
    if (!g_modal_active || g_modal_answered)
        return;
    g_modal_answer = r;
    g_modal_answered = 1;
    g_modal_active = 0; /* box closed */
    /* Per-site epilogue — the statement the immediate return value
     * skipped (see the modal note): the assert site's
     * `if (r == 2) DestroyWindow(g_c6c8)` (ski_core.c:142; 2 is the
     * IDCANCEL literal the original compares). The game ran the rest of
     * its post-box code at raise time: the assert site's
     * ski_pause_toggle() (ski_core.c:152), the score/fatal sites' plain
     * returns (ski_core.c:1638/1679/1715, 117, ski_win.c:506) — so this
     * hook must NOT re-run the toggle (a second ski_pause_toggle would
     * resume the game: g_c650 latches 0 -> ski_resume, ski_win.c:162). */
    if (g_modal_is_assert && g_modal_answer == 2)
        DestroyWindow(g_c6c8);
}

EMSCRIPTEN_KEEPALIVE int ski_messagebox_get(void)
{
    return shim_modal_pending() ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE int ski_messagebox_type(void) { return shim_modal_type(); }

EMSCRIPTEN_KEEPALIVE const char *ski_messagebox_text(void)
{
    return shim_modal_text();
}

EMSCRIPTEN_KEEPALIVE const char *ski_messagebox_caption(void)
{
    return shim_modal_caption();
}

/* Test hook (T20-review verification; the T21 harness needs it too):
 * raise a modal box from JS without playing a full run. `site` selects
 * the call shape: 0 = score site (owner = main window, type 0,
 * ski_core.c:2812), 1 = assert site (owner NULL, type 0x31,
 * ski_core.c:141). The hook calls the REAL MessageBoxA (shim modal
 * state + console log) so the pump-during-modal, input-drop and answer
 * behavior are the game's, but the game's own post-box epilogues
 * (ski_core.c:141-152) do NOT run — ski_messagebox_answer replays the
 * shim-side one (assert site: `if (r == 2) DestroyWindow(g_c6c8)`).
 * Returns the value MessageBoxA returned — IDOK: the yield is
 * pump-level (see the modal note).
 *
 * The call comes from JS — outside ski_mainloop — so g_in_pump is
 * faked around the MessageBoxA call: a game raise always happens
 * inside the pump's dispatch (timer -> ski_tick -> ... -> MessageBoxA),
 * and shim_modal_raise only records state for in-pump raises. */
EMSCRIPTEN_KEEPALIVE
int ski_messagebox_raise(int site)
{
    int r;
    g_in_pump = 1;
    if (site == 1)
        r = MessageBoxA(NULL, "T20 test box", "Assertion Failed", 0x31);
    else
        r = MessageBoxA(g_hmain, "T20 test box", "High Scores", 0);
    g_in_pump = 0;
    return r;
}

static void ski_mainloop(void);              /* rAF pump + scheduler */
extern int WinMain(HINSTANCE, HINSTANCE, LPSTR, int); /* src/ski_win.c */

/* virtual clock + scheduler */
static DWORD  g_now_ms;       /* GetTickCount */
static int    g_ticks_fired;  /* game-timer dispatches (== g_ski_tick) */
static int    g_timer_armed;
static UINT   g_tick_id;
static UINT   g_timer_period;
static TIMERPROC g_timer_fn;  /* Win32 callback timer proc (ski_tick_cb) */
static double g_next_real;    /* emscripten_get_now() ms deadline of next fire */

/* ---- message queue (ring; WM_PAINT is not queued — see mq_pop) --------- */
static int mq_space(void) { return g_mq_c < SKI_MQ_CAP; }

static void mq_post(ShimWin *w, UINT msg, WPARAM wp, LPARAM lp)
{
    int idx;
    if (!mq_space())
        return; /* scheduler waits for room: no tick is ever dropped */
    idx = (g_mq_r + g_mq_c) % SKI_MQ_CAP;
    g_mq[idx].h = w;
    g_mq[idx].msg = msg;
    g_mq[idx].wp = wp;
    g_mq[idx].lp = lp;
    g_mq_c++;
}

void shim_post(HWND h, unsigned msg, unsigned long wp, long lp)
{
    mq_post((ShimWin *)h, (UINT)msg, (WPARAM)wp, (LPARAM)lp);
}

/* Input messages: the only keyboard/mouse the game can receive (posted
 * by ski_key_event / ski_click). While a modal box is up the DIALOG
 * owns the input — wine 9.0 delivers no keys/clicks to the main window
 * behind the box (the modal note) — so mq_pop drops them; timers and
 * paints flow. */
static int is_input_msg(UINT msg)
{
    return msg == 0x0100 /* WM_KEYDOWN */ ||
           msg == 0x0102 /* WM_CHAR */ ||
           msg == 0x0201 /* WM_LBUTTONDOWN */ ||
           msg == 0x0202; /* WM_LBUTTONUP */
}

/* Next message: first any queued (non-dropped) message in FIFO order,
 * then — only when the queue is empty, as in real Windows — a
 * full-client WM_PAINT for the first dirty visible window. */
static int mq_pop(MqMsg *out)
{
    int i;
    while (g_mq_c > 0) {
        *out = g_mq[g_mq_r];
        g_mq_r = (g_mq_r + 1) % SKI_MQ_CAP;
        g_mq_c--;
        if (shim_modal_pending() && is_input_msg(out->msg))
            continue; /* modal up: the dialog eats the input */
        return 1;
    }
    for (i = 0; i < SKI_WIN_MAX; i++) {
        ShimWin *w = &g_wins[i];
        if (w->used && !w->dead && w->visible && !w->minimized &&
            w->dirty && w->proc) {
            out->h = w;
            out->msg = WM_PAINT;
            out->wp = 0;
            out->lp = 0;
            return 1;
        }
    }
    return 0;
}

/* ---- dispatch ---------------------------------------------------------- */
static LRESULT dispatch1(const MqMsg *m)
{
    ShimWin *w = m->h;
    if (m->msg == WM_TIMER && w == g_hmain && m->wp == g_tick_id) {
        g_now_ms += g_timer_period; /* virtual clock: advances per fire */
        g_ticks_fired++;
        /* Win32 callback timer: the system invokes the TIMERPROC directly
         * and never posts WM_TIMER to the WndProc; the game's wproc has no
         * WM_TIMER case (decompile FUN_00405800). The callback (ski_tick_cb,
         * ski_win.c:110) gates on c67c and calls ski_tick. */
        if (g_timer_fn)
            return g_timer_fn((HWND)w, m->wp, WM_TIMER, GetTickCount());
        return 0;
    }
    if (w && !w->dead && w->proc)
        return w->proc((HWND)w, m->msg, m->wp, m->lp);
    return 0;
}

/* ---- windows ----------------------------------------------------------- */
static void win_free(ShimWin *v)
{
    if (v->dc)
        shim_dc_free(v->dc);
    v->dc = NULL;
    v->used = 0;
    if (v == g_hmain)
        g_hmain = NULL;
}

static ShimWin *mkwin(const char *cls, const char *title, DWORD style,
                      int x, int y, int w, int h, ShimWin *parent, WNDPROC proc)
{
    int i, dw, dh;
    ShimWin *v;
    for (i = 0; i < SKI_WIN_MAX; i++) {
        if (g_wins[i].used)
            continue;
        v = &g_wins[i];
        memset(v, 0, sizeof *v);
        v->used = 1;
        v->cls = cls;
        v->title = title;
        v->x = x; v->y = y; v->w = w; v->h = h;
        v->child = (style & WS_CHILD) != 0 || parent != NULL;
        v->parent = parent;
        v->proc = proc;
        v->cx = v->child ? 0 : SKI_BX;
        v->cy = v->child ? 0 : SKI_BY;
        v->cw = v->child ? w : w - SKI_BW;
        v->ch = v->child ? h : h - SKI_BH;
        if (v->cw < 0) v->cw = 0;
        if (v->ch < 0) v->ch = 0;
        /* The window DC keeps a stable handle for the window's lifetime
         * (wm_create_main/status_create hold it across the repositioning
         * MoveWindow, so the surface can never be reallocated). Top-levels
         * get exactly the client size (the game never resizes them).
         * Children are created 0x0 and sized later by MoveWindow, so their
         * DC is allocated up front at the PARENT client size: an
         * over-allocation that is never visible — the game only draws
         * inside GetClientRect bounds, and the canvas composites copy
         * exactly the client region. */
        dw = v->cw;
        dh = v->ch;
        if (v->child && v->parent) {
            dw = v->parent->cw;
            dh = v->parent->ch;
        }
        if (dw <= 0 || dh <= 0) {
            dw = 1;
            dh = 1;
        }
        v->dc = shim_dc_new(dw, dh);
        if (v->dc)
            shim_dc_fill(v->dc, 0, 0, dw, dh, 0x00FFFFFFu); /* see header */
        return v;
    }
    return NULL;
}

ATOM RegisterClassA(const WNDCLASSA *wc)
{
    int i;
    if (!wc || !wc->lpszClassName ||
        g_class_n >= (int)(sizeof g_classes / sizeof g_classes[0]))
        return 0;
    for (i = 0; i < g_class_n; i++)
        if (!strcmp(g_classes[i].name, wc->lpszClassName))
            return (ATOM)(i + 1); /* re-register: return the existing atom */
    g_classes[g_class_n].name = wc->lpszClassName;
    g_classes[g_class_n].proc = wc->lpfnWndProc;
    g_classes[g_class_n].bg = wc->hbrBackground;
    return (ATOM)++g_class_n;
}

HWND CreateWindowExA(DWORD ex, LPCSTR cls, LPCSTR title, DWORD style,
                     int x, int y, int w, int h,
                     HWND parent, void *menu, HINSTANCE inst, void *param)
{
    static struct { int a, b, c, d, e, f; int cx, cy; } cs; /* CREATESTRUCT stand-in */
    WNDPROC proc = NULL;
    ShimWin *p, *v;
    int i;
    (void)ex; (void)menu; (void)inst; (void)param;
    if (!cls)
        return NULL;
    for (i = 0; i < g_class_n; i++)
        if (!strcmp(g_classes[i].name, cls)) {
            proc = g_classes[i].proc;
            break;
        }
    p = parent ? (ShimWin *)parent : NULL;
    v = mkwin(cls, title, style, x, y, w, h, p, proc);
    if (!v)
        return NULL;
    if (!v->child && !strcmp(v->cls, "SkiMain"))
        g_hmain = v;
    /* The rAF main loop (ski_mainloop) is started by the JS glue via
     * ski_start_pump() after main() returns. emscripten_set_main_loop
     * with simulate_infinite_loop=1 throws 'unwind' to hand control to
     * the event loop by unwinding the C stack — inside main() that
     * abandons the rest of boot (the status window, ShowWindow and
     * ski_game_start never ran; T20 boot trace). g_loop_started stays
     * the single-registration guard. */
    /* WM_NCCREATE + WM_CREATE: synchronous, real-Win32 semantics. The
     * CREATESTRUCT (lParam) is a stand-in with cxWindow/cyWindow; the
     * game's wprocs never read it (the lp+0x18/+0x1c stores belong to a
     * dead handler for message 36, which Windows never delivers). */
    cs.cx = w;
    cs.cy = h;
    if (v->proc) {
        v->proc((HWND)v, 0x81 /* WM_NCCREATE */, 1, (LPARAM)&cs);
        if (v->proc((HWND)v, WM_CREATE, 0, (LPARAM)&cs) == -1) {
            win_free(v);
            return NULL;
        }
    }
    return (HWND)v;
}

HWND FindWindowA(LPCSTR cls, LPCSTR title)
{
    int i;
    if (!cls)
        return NULL;
    for (i = 0; i < SKI_WIN_MAX; i++) {
        ShimWin *w = &g_wins[i];
        if (!w->used || w->dead)
            continue;
        if (strcmp(w->cls, cls) == 0 &&
            (!title || !w->title || strcmp(w->title, title) == 0))
            return (HWND)w;
    }
    return NULL; /* fresh web boot: the single-instance guard passes */
}

static void win_resize(ShimWin *v, int w, int h)
{
    v->w = w;
    v->h = h;
    if (v->child) {
        v->cw = w;
        v->ch = h;
    } else {
        /* Top-level resize: the record follows; the DC surface keeps its
         * size (stable handle — see mkwin) and blits clip to it. The game
         * never resizes a top-level window (SetWindowPos is only ever
         * called with SWP_NOSIZE|SWP_NOMOVE). */
        v->cw = w - SKI_BW;
        v->ch = h - SKI_BH;
        if (v->cw < 0) v->cw = 0;
        if (v->ch < 0) v->ch = 0;
    }
}

void MoveWindow(HWND h, int x, int y, int w, int hh, BOOL repaint)
{
    ShimWin *v = h;
    int moved, sized;
    if (!v || !v->used || v->dead)
        return;
    moved = (v->x != x || v->y != y);
    sized = (v->w != w || v->h != hh);
    if (!moved && !sized) {
        if (repaint)
            v->dirty = 1;
        return;
    }
    v->x = x;
    v->y = y;
    win_resize(v, w, hh);
    /* Synchronous WM_SIZE/WM_MOVE, like real Win32. */
    if (v->proc) {
        if (sized)
            v->proc((HWND)v, WM_SIZE, 0, 0);
        if (moved)
            v->proc((HWND)v, WM_MOVE, 0,
                    (LPARAM)(((unsigned)(unsigned short)x) |
                             ((unsigned)(unsigned short)y << 16)));
    }
    if (repaint)
        v->dirty = 1;
}

BOOL SetWindowPos(HWND h, HWND after, int x, int y, int w, int hh, UINT flags)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead)
        return FALSE;
    (void)after;
    if (flags & 0x0020 /* SWP_MINIMIZE */) {
        v->minimized = 1;
        v->visible = 0;
        if (v->proc)
            v->proc((HWND)v, WM_SIZE, SIZE_MINIMIZED, 0);
    } else {
        if (flags & 0x0040 /* SWP_SHOWWINDOW */) {
            v->minimized = 0;
            v->visible = 1;
        }
        if (!(flags & SWP_NOSIZE) || !(flags & SWP_NOMOVE))
            MoveWindow(h,
                       (flags & SWP_NOMOVE) ? v->x : x,
                       (flags & SWP_NOMOVE) ? v->y : y,
                       (flags & SWP_NOSIZE) ? v->w : w,
                       (flags & SWP_NOSIZE) ? v->h : hh, 1);
    }
    return TRUE;
}

BOOL SetWindowTextA(HWND h, LPCSTR s)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead || !s)
        return FALSE;
    v->title = s;
    return TRUE;
}

BOOL ShowWindow(HWND h, int cmd)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead)
        return FALSE;
    switch (cmd) {
    case SW_HIDE:
        v->visible = 0;
        break;
    case SW_MINIMIZE:
        v->minimized = 1;
        v->visible = 0;
        if (!v->child && v->proc)
            v->proc((HWND)v, WM_SIZE, SIZE_MINIMIZED, 0); /* c770 -> pause */
        break;
    case SW_SHOWNORMAL:
    case SW_SHOW:
    default:
        v->visible = 1;
        v->minimized = 0;
        if (!v->child && v->proc) {
            /* Reference boot trace (decompile/NOTES.md): WM_MOUSEACTIVATE
             * (lp=0) -> WM_ACTIVATE(1) -> WM_SIZE(0); the initial
             * WM_PAINT follows once the queue drains (dirty). */
            v->proc((HWND)v, WM_MOUSEACTIVATE, 0, 0);
            v->proc((HWND)v, WM_ACTIVATE, 1 /* WA_ACTIVATE */, 0);
            v->proc((HWND)v, WM_SIZE, 0 /* SIZE_RESTORED */, 0);
        }
        v->dirty = 1;
        break;
    }
    return TRUE;
}

BOOL UpdateWindow(HWND h) { (void)h; return TRUE; } /* paint is dirty-driven */
HWND SetFocus(HWND h) { (void)h; return NULL; }

BOOL InvalidateRect(HWND h, const RECT *rc, BOOL erase)
{
    (void)rc; (void)erase; /* the game always passes NULL rc: full client */
    ShimWin *v = h;
    if (v && v->used && !v->dead)
        v->dirty = 1;
    return TRUE;
}

BOOL KillTimer(HWND h, UINT id)
{
    (void)h;
    if (g_timer_armed && g_tick_id == id)
        g_timer_armed = 0;
    return TRUE;
}

UINT SetTimer(HWND h, UINT id, UINT ms, TIMERPROC fn)
{
    (void)h;
    g_timer_fn = fn; /* callback timer: the pump invokes fn per fire */
    if (!g_timer_armed)
        g_next_real = emscripten_get_now() + (double)ms;
    g_timer_armed = 1;
    g_tick_id = id;
    g_timer_period = ms;
    return 1;
}

DWORD GetTickCount(void) { return g_now_ms; }

/* Start the rAF pump (the shim's message/timer loop). Called exactly
 * once by the JS glue (web/boot.js) after the module's main() has
 * returned — see the CreateWindowExA note for why it cannot start from
 * inside main(). Idempotent.
 *
 * simulate_infinite_loop = 0: ski_mainloop is a plain frame callback,
 * NOT the C infinite loop the flag's 'unwind' handoff is meant for.
 * With flag 1, set_main_loop throws 'unwind' to unwind the *C* stack
 * back to the event loop — right inside main(), that abandons the rest
 * of boot (observed: status window / ShowWindow / ski_game_start never
 * ran); called from JS it would reject the module promise instead. */
EMSCRIPTEN_KEEPALIVE
void ski_start_pump(void)
{
    if (!g_loop_started) {
        g_loop_started = 1;
        emscripten_set_main_loop(ski_mainloop, 0, 0);
    }
}

BOOL BeginPaint(HWND h, PAINTSTRUCT *ps)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead || !ps)
        return FALSE;
    ps->hdc = v->dc;
    ps->rcPaint.left = 0;
    ps->rcPaint.top = 0;
    ps->rcPaint.right = v->cw;
    ps->rcPaint.bottom = v->ch;
    ps->fErase = 1;
    v->dirty = 0;
    return TRUE;
}

BOOL EndPaint(HWND h, const PAINTSTRUCT *ps)
{
    (void)h; (void)ps;
    canvas_flush();
    return TRUE;
}

HDC GetDC(HWND h)
{
    if (!h) {
        /* the screen DC: GetDeviceCaps' source (T19 returns HORZRES/VERTRES
         * from it; size must match the reference 1024x768 Xvfb). */
        if (!g_screen_dc)
            g_screen_dc = shim_dc_new(SKI_SCREEN_W, SKI_SCREEN_H);
        return g_screen_dc;
    }
    ShimWin *v = h;
    if (!v || !v->used || v->dead)
        return NULL;
    return v->dc;
}

int ReleaseDC(HWND h, HDC dc) { (void)h; (void)dc; return 1; }

BOOL GetClientRect(HWND h, RECT *rc)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead || !rc)
        return FALSE;
    rc->left = 0;
    rc->top = 0;
    rc->right = v->cw;
    rc->bottom = v->ch;
    return TRUE;
}

LRESULT DefWindowProcA(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)h; (void)msg; (void)wp; (void)lp;
    return 0; /* undriven messages: no state, no side effects */
}

void PostQuitMessage(int code) { (void)code; g_quit = 1; }

int GetMessageA(MSG *msg, HWND q, UINT min, UINT max)
{
    (void)q; (void)min; (void)max;
    if (msg)
        memset(msg, 0, sizeof *msg); /* WM_QUIT shape: wParam 0 */
    /* The shim's rAF main loop (ski_mainloop) is the pump. Returning 0
     * exits the game's WinMain loop cleanly; see the file header. */
    return 0;
}

BOOL TranslateMessage(const MSG *msg) { (void)msg; return TRUE; }

LRESULT DispatchMessageA(const MSG *msg)
{
    MqMsg m;
    if (!msg)
        return 0;
    m.h = (ShimWin *)msg->hwnd;
    m.msg = msg->msg;
    m.wp = msg->wParam;
    m.lp = msg->lParam;
    return dispatch1(&m);
}

BOOL DestroyWindow(HWND h)
{
    ShimWin *v = h;
    int i;
    if (!v || !v->used || v->dead)
        return FALSE;
    /* real Win32 destroys a window's children before its WM_DESTROY */
    if (!v->child)
        for (i = 0; i < SKI_WIN_MAX; i++)
            if (g_wins[i].used && g_wins[i].parent == v && !g_wins[i].dead)
                DestroyWindow((HWND)&g_wins[i]);
    if (v->proc)
        v->proc((HWND)v, WM_DESTROY, 0, 0); /* main wproc posts quit */
    v->dead = 1;
    win_free(v);
    return TRUE;
}

BOOL IsIconic(HWND h)
{
    ShimWin *v = h;
    if (!v || !v->used || v->dead)
        return 0;
    return v->minimized;
}

HICON LoadIconA(HINSTANCE inst, LPCSTR name)
{
    (void)inst; (void)name;
    return (HICON)1; /* non-NULL dummy */
}

HCURSOR LoadCursorA(HINSTANCE inst, LPCSTR name)
{
    (void)inst; (void)name;
    return (HCURSOR)1;
}

HICON OpenIcon(HICON icon) { return icon; }

int shim_ticks_fired(void) { return g_ticks_fired; }

ShimWin *shim_window(int n)
{
    int i, k = -1;
    for (i = 0; i < SKI_WIN_MAX; i++) {
        if (!g_wins[i].used || g_wins[i].dead)
            continue;
        if (++k == n)
            return &g_wins[i];
    }
    return NULL;
}

int shim_window_count(void)
{
    int i, c = 0;
    for (i = 0; i < SKI_WIN_MAX; i++)
        if (g_wins[i].used && !g_wins[i].dead)
            c++;
    return c;
}

/* ---- main loop (rAF; the shim's pump + timer scheduler) ----------------- */
static void ski_mainloop(void)
{
    MqMsg m;
    int acted = 0;
    if (g_quit) {
        emscripten_cancel_main_loop();
        return;
    }
    /* A modal box up does NOT suspend the pump: wine 9.0 keeps the
     * main window's timer firing and painting behind the box (the modal
     * note) — the world ticks and repaints until
     * ski_messagebox_answer closes it. Input is dropped in mq_pop. */
    g_in_pump = 1;
    if (g_timer_armed && g_hmain && !g_hmain->dead) {
        double now = emscripten_get_now();
        /* Catch-up after REAL throttling (hidden tab) only: with no
         * pump suspension, no backlog accumulates behind a modal, so
         * nothing bursts on close. Each fire posts one WM_TIMER and
         * dispatch1 advances the virtual clock exactly one period per
         * dispatch — state stays a pure function of the tick count. */
        while (now >= g_next_real && mq_space()) {
            mq_post(g_hmain, WM_TIMER, g_tick_id, 0);
            g_next_real += (double)g_timer_period;
            acted = 1;
        }
    }
    while (mq_pop(&m)) {
        dispatch1(&m);
        acted = 1;
    }
    g_in_pump = 0;
    if (acted)
        canvas_flush();
}

/* ---- entry --------------------------------------------------------------- */
/* emscripten needs main; the game's entry is WinMain. hPrev = NULL so the
 * class registration runs (ski_win.c:807 gates on hPrev == NULL); show =
 * SW_SHOWNORMAL, as the reference launch used. The INI cache loads from
 * localStorage before WinMain (the high-score read happens during the
 * first game over; boot itself only needs the cache ready). */
int main(void)
{
    shim_ini_load();
    return WinMain((HINSTANCE)1, NULL, (LPSTR)"", 1);
}
