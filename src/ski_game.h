/* Reconstructed SkiFree 1.04 (32-bit) — global and function interface.
 *
 * Layout:
 *   src/ski_win.c   window + message layer (Task 10): WinMain 0x4047e0,
 *                   game_create_windows 0x4052d0, wproc_main 0x405800,
 *                   input handlers, status window, pause/resume/restart
 *                   lifecycle, 40 ms callback timer.
 *   src/ski_core.c  globals + game core (Task 11: tick/physics/entities/
 *                   spawn/RNG) + render/text/sound (Task 12). Stubs marked
 *                   "T11"/"T12" below are filled in by those tasks; leaves
 *                   needed by the window layer (string cache, fatal/assert
 *                   boxes, sound path, bitmap loader, text helpers) are
 *                   implemented now.
 */
#ifndef SKI_GAME_H
#define SKI_GAME_H

#include "skidef.h"

/* ================= .data globals (defined in ski_core.c) =================
 * Names carry the original VAs (ImageBase 0x400000). Word-pair globals are
 * split into the u16 fields the game actually touches. */

extern uint32_t g_c16c; /* 0x40c16c RNG LCG state */
extern uint32_t g_c5d8; /* 0x40c5d8 (T11) */
extern uint32_t g_c5dc; /* 0x40c5dc last status-paint time (c698 stamp) */
extern uint32_t g_c5f4; /* 0x40c5f4 last tick delta ms (T11 sets; display only) */
extern void    *g_c5f8; /* 0x40c5f8 sprite column table (LocalAlloc 0x5a0) */
extern uint16_t g_c5fc; /* 0x40c5fc window-center y (lo16) */
extern uint32_t g_c600; /* 0x40c600 pause timestamp (ms) */
extern uint16_t g_c640; /* 0x40c640 camera x offset (lo16) */
extern void    *g_c64c; /* 0x40c64c player entity ptr (set by ski_game_start) */
extern uint16_t g_c5f0, g_c5f2; /* 0x40c5f0 client width (lo16) + hi */
extern uint32_t g_c610; /* 0x40c610 input-redraw latch */
extern void    *g_c618; /* 0x40c618 active entity list head */
extern HBITMAP  g_c614; /* 0x40c614 canvas bitmap (selected in c5ec) */
extern HWND     g_c624; /* 0x40c624 status window (WS_CHILD) */
extern HBITMAP  g_c620; /* 0x40c620 small image bitmap (selected in c710) */
extern HDC      g_c63c; /* 0x40c63c main window DC */
extern uint16_t g_c6a0, g_c6a2; /* 0x40c6a0 HORZRES (lo16) + hi */
extern HDC      g_c6a4; /* 0x40c6a4 small mask DC */
extern uint32_t g_c6a8; /* 0x40c6a8 score */
extern uint16_t g_c668, g_c66a; /* 0x40c668 tmHeight, 0x40c66a tmHeight*4 */
extern uint16_t g_c66c, g_c66e; /* 0x40c66c label_w+value_w, 0x40c66e label_w */
extern HGDIOBJ  g_c664; /* 0x40c664 status DC selection */
extern uint32_t g_c670; /* 0x40c670 turbo flag ('f' key) */
extern void    *g_c674; /* 0x40c674 string cache ptr array (LocalAlloc 0x50) */
extern uint32_t g_c678; /* 0x40c678 timer period ms (init SKI_TIMER_MS) */
extern uint32_t g_c67c; /* 0x40c67c game active (tick + input gate) */
extern int32_t  g_c680, g_c684; /* 0x40c680/84 world extent left/top (WM_SIZE) */
extern int32_t  g_c688, g_c68c; /* 0x40c688/8c world extent right/bottom */
extern uint16_t g_c690, g_c6e8; /* 0x40c690/e8 canvas w/h (aligned+0x40) */
extern uint32_t g_c694; /* 0x40c694 window active (WM_ACTIVATE wParam) */
extern HBRUSH   g_c69c; /* 0x40c69c NULL_BRUSH (GetStockObject(0)) */
extern uint32_t g_c698; /* 0x40c698 last tick time ms / RNG seed source */
extern HINSTANCE g_c61c; /* 0x40c61c hInstance */
extern RECT     g_c6b0; /* 0x40c6b0 main client rect {l,t,r,b} */
extern HWND     g_c6c8; /* 0x40c6c8 main window */
extern HDC      g_c6cc; /* 0x40c6cc status window DC */
extern uint32_t g_c650; /* 0x40c650 paused latch (ski_pause_toggle) */
extern uint32_t g_c6d0; /* 0x40c6d0 timer armed */
extern HBITMAP  g_c6d4; /* 0x40c6d4 small mask bitmap (selected in c6a4) */
extern uint16_t g_c6d8, g_c6da; /* 0x40c6d8 client height (lo16) + hi */
extern HDC      g_c6ec; /* 0x40c6ec big mask DC */
extern uint32_t g_c6fc; /* 0x40c6fc on-screen area budget (T11) */
extern uint16_t g_c700, g_c70c; /* 0x40c700/70c last mouse x/y */
extern uint32_t g_c702; /* 0x40c702 gate slot count (T11) */
extern uint16_t g_c704, g_c706; /* 0x40c704 window-center x (lo16) + hi */
extern uint32_t g_c708; /* 0x40c708 style timing (T11) */
extern uint32_t g_c714; /* 0x40c714 spawn cursor X (T11) */
extern HDC      g_c710; /* 0x40c710 small image DC */
extern void    *g_c72c; /* 0x40c72c player ref (input gate; NULL = dead) */
extern HDC      g_c730; /* 0x40c730 big image DC */
extern void    *g_c744; /* 0x40c744 entity freelist head (T11) */
extern uint16_t g_c74c, g_c74e; /* 0x40c74c VERTRES (lo16) + hi */
extern uint32_t g_c748; /* 0x40c748 extended world area (WM_SIZE) */
extern void    *g_c758; /* 0x40c758 gate pool (LocalAlloc 0x2400) */
extern uint32_t g_c760; /* 0x40c760 mouse-seen flag */
extern HBITMAP  g_c75c; /* 0x40c75c big mask bitmap (selected in c6ec) */
extern uint32_t g_c770; /* 0x40c770 minimized flag (init 1 = start paused) */
extern RECT     g_c778; /* 0x40c778 status client rect */
extern char     g_c788[16]; /* 0x40c788 zeroed status caption buffer */
extern HMODULE  g_c78c; /* 0x40c78c sound DLL module (always 0: static import) */
extern void    *g_c790; /* 0x40c790 PlaySoundA function pointer */
extern uint32_t g_c794; /* 0x40c794 nosound command-line flag */
extern void    *g_c940; /* 0x40c940 timer callback ptr (ski_tick_cb) */
extern void    *g_c94c, *g_c950; /* 0x40c94c/950 style gate cursors (T11) */
extern uint32_t g_c944, g_c948; /* 0x40c944/948 style accumulators */
extern uint32_t g_c954, g_c958, g_c95c; /* style flags (T12 status draw) */
extern uint16_t g_c960, g_c962, g_c964, g_c968; /* style words (0x40c960..) */
extern HBITMAP  g_c644; /* 0x40c644 big image bitmap (selected in c730) */
extern HDC      g_c5ec; /* 0x40c5ec scratch DC */
extern void    *g_c648; /* 0x40c648 entity pool (LocalAlloc 0x2000 = 100x80B) */

/* 9 sound resources (silent by construction: FindResourceA(type "WAVE")
 * returns NULL — the PE has no such resource node; see NOTES M1#3).
 * Pairs: [0]=HGLOBAL, [1]=LockResource ptr. */
typedef struct { HGLOBAL h; LPVOID p; } ski_sound_t;
extern ski_sound_t ski_sound_1; /* @0x40c6c0 */
extern ski_sound_t ski_sound_2; /* @0x40c768 */
extern ski_sound_t ski_sound_3; /* @0x40c5d0 */
extern ski_sound_t ski_sound_4; /* @0x40c718 */
extern ski_sound_t ski_sound_5; /* @0x40c750 */
extern ski_sound_t ski_sound_6; /* @0x40c628 */
extern ski_sound_t ski_sound_7; /* @0x40c6f0 (resource id 9) */
extern ski_sound_t ski_sound_8; /* @0x40c6e0 (resource id 7) */
extern ski_sound_t ski_sound_9; /* @0x40c608 (resource id 8) */

/* ================= Window layer (src/ski_win.c) ================= */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmd, int show);
LRESULT CALLBACK ski_wproc_main(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ski_wproc_status(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* --- lifecycle (0x404970 / 0x405760 / 0x4057c0 / 0x404ad0 / 0x405a10 / 0x406500) --- */
int ski_game_reset(void); /* 0x404970 seed + level_init + counters */
void ski_pause(void); /* 0x4057c0 KillTimer + pause timestamp */
void ski_resume(void); /* 0x404ad0 SetTimer + style-run accounting */
void ski_pause_auto(void); /* 0x405a10 activate/minimize driven */
void ski_pause_toggle(void); /* 0x405760 F3: latch + caption + repaint */
void ski_restart(void); /* 0x406500 F2: reset + start + repaint */

/* ================= Game core (src/ski_core.c) =================
 * T11 = game logic (filled in by Task 11); T12 = render/text/sound (Task 12);
 * leaves marked T10 are implemented now. */

/* --- T10 leaves (window-layer dependencies, fully known) --- */
char *ski_str_cache(UINT id); /* 0x401cf0 LoadStringA + LocalAlloc cache */
void ski_fatal_msg(const char *msg); /* 0x404950 MessageBox, MB_ICONERROR|MB_OK */
void ski_assert_fail(const char *file, unsigned line); /* 0x401240 */
void ski_assert_box(const char *caption, const char *msg); /* 0x401270 */
int ski_sound_init(void); /* 0x405620 c790 = PlaySoundA; returns c790 != NULL */
int ski_sound_load(UINT id, ski_sound_t *dst); /* 0x405640 FindResourceA("WAVE") */
void ski_sound_free(ski_sound_t *dst); /* 0x405730 */
void ski_cleanup(void); /* 0x4056a0 exit path (sound stop + free pairs) */
HBITMAP ski_load_bitmap(UINT id); /* 0x405ea0 LoadBitmapA */
int ski_load_bitmaps(HDC hdc); /* 0x405ab0 build DCs + c5f8 column table */
void ski_text_draw(HDC hdc, const char *s, short x, short *y, int len); /* 0x401e20 */
void ski_text_extent(HDC hdc, short *maxw, const char *s, int len); /* 0x406c50 */

/* --- T11 game core (stubs until Task 11) --- */
int ski_init_mem(void); /* 0x4048c0 LocalAlloc pools (T10: implemented) */
void ski_level_init(void); /* 0x404a00 freelist build (T11) */
int ski_game_start(void); /* 0x404a80 spawn player + start (T11) */
void ski_tick(void); /* 0x401000 one fixed-step (T11) */
void *ski_entity_alloc(int type, uint32_t frame); /* 0x4020d0 (T11) */
void *ski_teleport(void *ent, short x, short y, short mode); /* 0x402390 (T11) */
void *ski_set_frame(void *ent, uint32_t frame); /* 0x402120 (T11) */
void *ski_group_head(void *ent); /* 0x402220 (T11) */
void ski_size_hook(short cx, short cy); /* 0x406060 (T11) */
uint32_t ski_aim_facing(short dx, short dy); /* 0x4065e0 (T11) */
uint32_t ski_aim_crouch(short dx, short dy); /* 0x406670 (T11) */
void ski_start_benches(void); /* 0x4051e0 (T11) */
void ski_start_decor(void); /* 0x404b50 level layout (T11, 117 rand calls) */
int ski_rand(void); /* 0x406cda MSVC CRT LCG (T11) */
void ski_rand_seed(uint32_t seed); /* 0x406cd0 (T11) */

/* --- T12 render/status (stubs until Task 12) --- */
void ski_render(HDC hdc, const RECT *rc); /* 0x401060 scene render */
void ski_paint_scene(HDC hdc, const RECT *rc); /* 0x406100 cull + render */
void ski_status_draw_values(HDC hdc); /* 0x401b80 panel values (T12) */
void ski_fmt_time(uint32_t ms, char *buf); /* 0x401d70 "%2u:%2.2u:%2.2u.%2.2u" (id 11) */

#endif /* SKI_GAME_H */
