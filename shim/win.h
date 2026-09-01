/* Internal window model shared by win.c (owner) and canvas.c (T18).
 * Not part of the game-visible shim API (win32.h is that). */
#ifndef SHIM_WIN_H
#define SHIM_WIN_H
#include "win32.h"

#define SKI_WIN_MAX 8
#define SKI_MQ_CAP  64

typedef struct ShimWin {
    int used;            /* slot in use (creation-order index basis) */
    int dead;            /* DestroyWindow'd */
    int child;           /* WS_CHILD (coords relative to parent client) */
    int visible;         /* ShowWindow'd */
    int minimized;       /* SW_MINIMIZE */
    int dirty;           /* full-client WM_PAINT pending (low priority) */
    const char *cls;
    const char *title;
    int x, y, w, h;      /* outer (top-level) or parent-client (child) */
    int cw, ch;          /* client size (GetClientRect) */
    int cx, cy;          /* client offset from (x,y): 4,30 top-level; 0,0 child */
    WNDPROC proc;
    HDC dc;              /* client DC (stable handle; see mkwin in win.c) */
    struct ShimWin *parent;
} ShimWin;

ShimWin *shim_window(int n);      /* nth live window, creation order */
int      shim_window_count(void);
void     shim_post(HWND h, unsigned msg, unsigned long wp, long lp);
int      shim_ticks_fired(void);  /* game WM_TIMER dispatches (== g_ski_tick) */
void     canvas_flush(void);      /* canvas.c: upload window surfaces */

/* MessageBoxA modal (Task 20; design in the misc.c MessageBoxA note and
 * the API.md MessageBoxA row). shim_modal_raise records the pending box
 * and, while the pump is live, longjmps to ski_mainloop's setjmp — it
 * does NOT return in that case; out-of-pump (boot-time) it returns and
 * the caller continues with IDOK. While a box is up the pump suspends
 * the game (no timers/messages/paint); ski_messagebox_answer(r) resolves
 * it and the per-site epilogue runs on the next pump frame. */
void         shim_modal_raise(HWND owner, const char *text, const char *caption,
                              UINT type);
int          shim_modal_pending(void); /* 1 while the box is up (unanswered) */
int          shim_modal_type(void);    /* UINT type of the raised box */
const char  *shim_modal_text(void);
const char  *shim_modal_caption(void);
void         shim_modal_answer(int r); /* IDOK 1 / IDCANCEL 2 (or any id) */
void         shim_ini_load(void);      /* misc.c: entpack.ini <- localStorage */

#endif /* SHIM_WIN_H */
