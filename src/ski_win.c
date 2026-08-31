/* Reconstructed SkiFree 1.04 — window + message layer (Task 10).
 *
 * Transcribed 1:1 from the decompilation (decompile/ghidra/FUN_*.c) with
 * every player-field access re-verified against the raw disassembly where
 * the decompiler misread offsets (NOTES.md "Input" section records the
 * final verified keymap; the L/R steer tables below come from 0x40a258).
 *
 * Source map:
 *   WinMain            0x4047e0
 *   ski_tick_cb        0x4047c0  (40 ms callback timer)
 *   ski_create_windows 0x4052d0
 *   ski_wproc_main     0x405800  (jump tables 0x4059c4/0x4059e0)
 *   wm_create_main     0x405a40
 *   wm_destroy_main    0x405ec0
 *   wm_size_main       0x405fa0
 *   wm_paint_main      0x4060b0
 *   ski_key_down       0x406170  (first switch 0x4063a8/0x4063bc,
 *                                 second switch 0x406424/0x40644c)
 *   ski_char_key       0x406780  (table 0x40686c, jump table 0x40684c)
 *   ski_mouse_aim      0x406550
 *   ski_click_action   0x4066d0  (jump table 0x406764)
 *   ski_wproc_status   0x4068d0
 *   status_create      0x406a70
 *   status_destroy     0x406c80
 *   status_reposition  0x406890
 *   status_paint       0x406970
 *   ski_pause_toggle   0x405760
 *   ski_pause          0x4057c0
 *   ski_resume         0x404ad0
 *   ski_pause_auto     0x405a10
 *   ski_game_reset     0x404970
 *   ski_restart        0x406500
 */
#include <stdio.h>
#include "ski_game.h"

#if SKI_HARNESS
extern int g_ski_tick; /* defined in ski_core.c (harness counter) */
#endif

#define ENT16(e, off) (*(const int16_t *)((const char *)(e) + (off)))
#define ENT16W(e, off) (*(int16_t *)((char *)(e) + (off)))
#define ENT32(e, off) (*(const uint32_t *)((const char *)(e) + (off)))
#define SKI_ASSERT_FILE "V:\\hack\\ski32\\ski2.c" /* .data 0x40c090 */

/* Steer tables @0x40a258 (stride 8B: {left, right}), dumped from the PE.
 * Left/Right keys: newframe = L[frame] / R[frame]; steer is applied only
 * when the target frame is 7 (left) or 8 (right). */
const ski_steer_pair_t ski_steer_table[22] = {
    {0x01, 0x04}, {0x02, 0x00}, {0x03, 0x01}, {0x07, 0x02}, /* 0..3 */
    {0x00, 0x05}, {0x04, 0x06}, {0x05, 0x08}, {0x03, 0x02}, /* 4..7 */
    {0x05, 0x06}, {0x09, 0x02}, {0x05, 0x0a}, {0x03, 0x06}, /* 8..b */
    {0x03, 0x06}, {0x0e, 0x0f}, {0x10, 0x0d}, {0x0d, 0x10}, /* c..f */
    {0x0f, 0x0e}, {0x0e, 0x0f}, {0x14, 0x15}, {0x14, 0x15}, /* 10..13 */
    {0x10, 0x0d}, {0x0d, 0x10},                               /* 14..15 */
};

/* ---------------- timer (0x4047c0) ---------------- */

static void ski_key_down(uint32_t vk); /* fwd (defined below) */

#if SKI_HARNESS
/* T13/T14 differential alignment: fire the start key at the reference
 * original's exact KP_1 boundary. In the captured original run the
 * WM_KEYDOWN landed between timer callbacks 467 and 468: the last
 * fr=3 sample holds A_466 (c16c after menu tick 466 = 0x12e83c69),
 * tick 467 is a final MENU tick (4 RNG calls -> 0x69780dfd), and the
 * first fr=1 sample holds that 0x69780dfd with sp/st/py still 0
 * (verified: the original's first descent tick then ramps sp 0->1 with
 * st 0->0, second tick sp 1->2 st 0->0, third sp 2->3 st 0->-1 —
 * exactly ski_anim_update rows). SKI_ALIGN_C16C must be set to
 * 0x69780dfd: it is both the hook's fire condition and the
 * keydown-time injection value (a no-op at this boundary). A file-tail
 * xdotool fire cannot reliably land inside one 40 ms window, so the
 * harness synthesizes the keydown itself: at tick-callback entry, when
 * c16c == SKI_ALIGN_C16C and the player sits at frame 3 / mode 0,
 * call ski_key_down(0x61) before ski_tick() so THIS tick is the first
 * descent tick — sample-identical to the original. One-shot (the same
 * (c16c, frame) pair recurs in the post-F2 menu walk; the flag keeps
 * it from refiring). */
static void ski_harness_maybe_fire(void)
{
    static int fired = 0;
    static int have_target = 0;
    static uint32_t target = 0;
    if (fired)
        return;
    if (!have_target) {
        const char *a = getenv("SKI_ALIGN_C16C");
        have_target = 1;
        if (a == NULL)
            return;
        target = (uint32_t)strtoul(a, NULL, 0);
        if (target == 0)
            fired = 1; /* explicitly disabled */
        return;
    }
    if (g_c72c == NULL || g_c16c != target)
        return;
    if (ENT32(g_c72c, ENT_FRAME) != 3 || ENT16(g_c72c, ENT_MODE) != 0)
        return;
    fired = 1;
    ski_key_down(0x61);
}
#endif

static LRESULT CALLBACK ski_tick_cb(HWND hwnd, UINT id, UINT msg, DWORD time)
{
    (void)hwnd; (void)id; (void)msg; (void)time;
    if (g_c67c != 0) {
#if SKI_HARNESS
        ski_harness_maybe_fire();
#endif
        ski_tick();
    }
    return 1;
}

/* ---------------- pause / resume (0x4057c0/0x404ad0/0x405a10) ---------------- */

void ski_pause(void)
{
    if (g_c6c8 != NULL && g_c6d0 != 0) {
        g_c6d0 = 0;
        KillTimer(g_c6c8, SKI_TIMER_ID);
        g_c600 = g_c698; /* pause timestamp (style-run accounting, 0x404ad0) */
    }
}

void ski_resume(void)
{
    if (g_c6c8 != NULL && g_c6d0 == 0 && g_c650 == 0) {
        g_c6d0 = 1;
        g_c698 = GetTickCount();
        if (g_c95c != 0 || g_c958 != 0)
            g_c948 = g_c948 + (g_c698 - g_c600);
        SetTimer(g_c6c8, SKI_TIMER_ID, g_c678 & 0xffff, (TIMERPROC)g_c940);
    }
}

void ski_pause_auto(void)
{
#if SKI_HARNESS
    /* T14 parity: the instrumented original is patched at 0x405a17
     * (je -> 2 nop) so resume is driven ONLY by !c770 (minimize) —
     * the wineserver foreground/activation state under Xvfb is racy
     * and would intermittently freeze either side of the diff.
     * Deactivation (c694) no longer pauses; minimize still does.
     * Production (flag off) keeps the faithful c694 && !c770. */
    if (g_c770 == 0) {
#else
    if (g_c694 != 0 && g_c770 == 0) {
#endif
        g_c67c = 1;
        ski_resume();
        return;
    }
    g_c67c = 0;
    ski_pause();
}

void ski_pause_toggle(void)
{
    g_c650 = g_c6d0;
    if (g_c6d0 != 0) {
        ski_pause();
        SetWindowTextA(g_c6c8, ski_str_cache(STR_PAUSED)); /* id 2 */
        InvalidateRect(g_c6c8, NULL, 0);
        return;
    }
    SetWindowTextA(g_c6c8, ski_str_cache(STR_TITLE)); /* id 1 */
    ski_resume();
}

/* ---------------- game reset (0x404970) ---------------- */

int ski_game_reset(void)
{
    DWORD t = GetTickCount();
#if SKI_DETERMINISTIC
    /* T8 seed freeze: the original is patched at 0x404971 to move
     * 0x00123456 into eax instead of calling GetTickCount. The rebuild
     * mirrors that here (see harness/seed.json). */
    t = SKI_SEED_CONSTANT;
#endif
    g_c698 = t;
    ski_rand_seed(t);
    ski_level_init(); /* 0x404a00 */
    g_c64c = NULL;
    g_c72c = NULL;
    g_c6fc = 0;
    g_c702 = 0; /* 0x404a70 */
    g_c670 = 0;
    g_c5f2 = 0;
    g_c640 = 0;
    g_c5d8 = 0;
    g_c714 = 0;
    g_c6a8 = 0;
    g_c964 = 0;
    g_c95c = 0;
    g_c960 = 0;
    g_c958 = 0;
    g_c944 = 0;
    g_c678 = SKI_TIMER_MS;
    g_c610 = 1;
    return 1; /* original eax after reset is non-zero (level_init loop leftover) */
}

/* ---------------- restart (0x406500) ---------------- */

void ski_restart(void)
{
    if (ski_game_reset() != 0) {
        if (g_c650 != 0)
            ski_pause_toggle(); /* F2 while paused: restart and unpause */
        InvalidateRect(g_c6c8, NULL, 1);
        if (ski_game_start() != 0) {
            UpdateWindow(g_c6c8);
            return;
        }
    }
    DestroyWindow(g_c6c8);
}

/* ---------------- input (0x406170 / 0x406780 / 0x406550 / 0x4066d0) ---------------- */

static void ski_key_down(uint32_t vk)
{
#if SKI_HARNESS
    /* T13/T14 differential alignment (harness-only): on the first
     * start-key press (KP_1 / End), overwrite the RNG state with the
     * reference original run's exact keydown-time c16c
     * (SKI_ALIGN_C16C=0x...; for the captured run 0x69780dfd = A_467,
     * after menu tick 467). The start transition (ski_set_frame 3->1)
     * makes no rand calls, so keydown-time injection is exactly
     * descent-tick-0 state. With the deterministic menu this value is
     * already the natural c16c at the hook's fire boundary, so the
     * injection is belt-and-suspenders (covers fire-latency jitter of
     * the xdotool fallback path). */
    static int align_done = 0;
    if (!align_done && (vk == 0x23 || vk == 0x61)) {
        align_done = 1;
        const char *a = getenv("SKI_ALIGN_C16C");
        if (a != NULL) {
            uint32_t v = (uint32_t)strtoul(a, NULL, 0);
            if (v != 0)
                g_c16c = v;
        }
    }
#endif
    /* First switch (0x4063a8, idx table 0x4063bc, byte-verified):
     * VK_RETURN 0x0d -> Enter gate; VK_ESCAPE 0x1b -> minimize;
     * VK_F2 0x71 -> restart; VK_F3 0x72 -> pause toggle;
     * every other VK falls through to the second switch. */
    switch (vk) {
    case 0x0d: /* VK_RETURN */
        if (g_c72c != NULL)
            return; /* player alive: Enter does nothing */
        /* fall through */
    case 0x71: /* VK_F2 */
        ski_restart();
        return;
    case 0x1b: /* VK_ESCAPE */
        ShowWindow(g_c6c8, 6 /* SW_MINIMIZE */);
        return;
    case 0x72: /* VK_F3 */
        ski_pause_toggle();
        return;
    default:
        break;
    }

    if (g_c72c == NULL)
        return;
    uint32_t frame = ENT32(g_c72c, ENT_FRAME);
    uint16_t mode = ENT16(g_c72c, ENT_MODE);
    if (frame == 0xb || frame == 0x11)
        return; /* frozen frames: no state change (tail compares equal) */

    /* Second switch (0x406424, idx table 0x40644c; re-verified against
     * raw table bytes 2026-08-30 — the 2026-08-29 "byte-verify" had two
     * misreads, both fixed now):
     * PageUp(0x21)/Numpad9(0x69) -> 6 (idx @0x406494 = 0 -> jt[0] =
     * 0x406320, the SAME case body as PageUp); PageDown(0x22)/Numpad3 -> 4;
     * End(0x23)/Numpad1 -> 1; Home(0x24)/Numpad7 -> 3 (all mode-0 only);
     * Left(0x25)/Numpad4 -> L[] steer; Up(0x26)/Numpad8(0x68) -> up-table
     * (0x4064bc, frame 3..0x13); Right(0x27)/Numpad6 -> R[] steer;
     * Down(0x28)/Numpad2 -> mode 0 ? frame 0 : down-table (0x406498,
     * frame 0xd..0x15); '-'(0x2d)/Insert/Numpad0 -> crouch (mode 0);
     * Numpad5 and all else no-op.
     * The 0xd/0x12/0x13 crouch cycle: UP advances 0xd->0x12->0x13->0xd,
     * DOWN walks it backwards 0xd->0x13->0x12->0xd (down-table bytes:
     * 0x406498=0x406293(0x13), 0x4064ac=0x4062ce(0xd),
     * 0x4064b0=0x4062c4(0x12)). */
    uint32_t new_frame = frame;
    switch (vk) {
    case 0x21: case 0x69: /* VK_PRIOR / VK_NUMPAD9: facing 6, mode 0 only
     * (0x69 shares PageUp's case body 0x406320 — idx byte @0x406494 = 0) */
        if (mode == 0)
            new_frame = 6;
        break;
    case 0x22: case 0x63: /* VK_NEXT / VK_NUMPAD3 */
        if (mode == 0)
            new_frame = 4;
        break;
    case 0x23: case 0x61: /* VK_END / VK_NUMPAD1 */
        if (mode == 0)
            new_frame = 1;
        break;
    case 0x24: case 0x67: /* VK_HOME / VK_NUMPAD7 */
        if (mode == 0)
            new_frame = 3;
        break;
    case 0x25: case 0x64: /* VK_LEFT / VK_NUMPAD4 */
        if (frame > 0x15)
            ski_assert_fail(SKI_ASSERT_FILE, 0xf63);
        new_frame = ski_steer_table[frame].left;
        if (new_frame == 7) {
            short steer = ENT16(g_c72c, ENT_STEER) - 8;
            if (steer < -7)
                steer = -8;
            ENT16W(g_c72c, ENT_STEER) = steer;
        }
        break;
    case 0x26: case 0x68: /* VK_UP / VK_NUMPAD8 */
        switch (frame) {
        case 3: case 7: case 0xc:
            if (ENT16(g_c72c, ENT_SPEED) == 0) {
                new_frame = 9;
                ENT16W(g_c72c, ENT_SPEED) = -4;
            }
            break;
        case 6: case 8:
            if (ENT16(g_c72c, ENT_SPEED) == 0) {
                new_frame = 0xa;
                ENT16W(g_c72c, ENT_SPEED) = -4;
            }
            break;
        case 0xd: new_frame = 0x12; break;
        case 0xe: new_frame = 0x14; break;
        case 0xf: new_frame = 0x15; break;
        case 0x12: new_frame = 0x13; break;
        case 0x13: new_frame = 0xd; break;
        }
        break;
    case 0x27: case 0x66: /* VK_RIGHT / VK_NUMPAD6 */
        if (frame > 0x15)
            ski_assert_fail(SKI_ASSERT_FILE, 0xf6b);
        new_frame = ski_steer_table[frame].right;
        if (new_frame == 8) {
            short steer = ENT16(g_c72c, ENT_STEER) + 8;
            if (steer > 8)
                steer = 8;
            ENT16W(g_c72c, ENT_STEER) = steer;
        }
        break;
    case 0x28: case 0x62: /* VK_DOWN / VK_NUMPAD2 */
        if (mode == 0) {
            new_frame = 0;
            break;
        }
        switch (frame) {
        case 0xd: new_frame = 0x13; break;   /* 0x406293 */
        case 0x12: new_frame = 0xd; break;   /* 0x4062ce */
        case 0x13: new_frame = 0x12; break;  /* 0x4062c4 */
        case 0x14: new_frame = 0xe; break;   /* 0x40629d */
        case 0x15: new_frame = 0xf; break;   /* 0x4062a7 */
        }
        break;
    case 0x2d: case 0x60: /* VK_INSERT / VK_NUMPAD0 */
        if (mode == 0) {
            ENT16W(g_c72c, ENT_CROUCH) = 2;
            new_frame = 0xd;
            short speed = ENT16(g_c72c, ENT_SPEED);
            if (speed > 4)
                ENT16W(g_c72c, ENT_SPEED) = speed - 4;
        }
        break;
    default:
        break;
    }

    /* Tail (0x40636a/0x40636f): apply the frame change, one redraw while
     * the input-redraw latch is set. */
    if (new_frame != frame) {
        ski_set_frame(g_c72c, new_frame);
        if (g_c610 != 0) {
            ski_render(g_c63c, &g_c6b0);
            g_c610 = 0;
        }
    }
}

static void ski_char_key(uint32_t ch)
{
    /* Table 0x40686c / jump table 0x40684c (byte-verified): X x-2, Y y-2,
     * x x+2, y y+2 (teleport, 2 units of 1/16 m), f turbo toggle,
     * r render, t manual tick; everything else no-op. */
    switch (ch) {
    case 0x58: /* 'X' */
        if (g_c72c != NULL)
            ski_teleport(g_c72c, ENT16(g_c72c, ENT_X) - 2, ENT16(g_c72c, ENT_Y),
                         ENT16(g_c72c, ENT_MODE));
        return;
    case 0x59: /* 'Y' */
        if (g_c72c != NULL)
            ski_teleport(g_c72c, ENT16(g_c72c, ENT_X), ENT16(g_c72c, ENT_Y) - 2,
                         ENT16(g_c72c, ENT_MODE));
        break;
    case 0x66: /* 'f' */
        g_c670 = (g_c670 == 0);
        return;
    case 0x72: /* 'r' */
        ski_render(g_c63c, &g_c6b0);
        return;
    case 0x74: /* 't' */
        ski_tick();
        return;
    case 0x78: /* 'x' */
        if (g_c72c != NULL)
            ski_teleport(g_c72c, ENT16(g_c72c, ENT_X) + 2, ENT16(g_c72c, ENT_Y),
                         ENT16(g_c72c, ENT_MODE));
        return;
    case 0x79: /* 'y' */
        if (g_c72c != NULL)
            ski_teleport(g_c72c, ENT16(g_c72c, ENT_X), ENT16(g_c72c, ENT_Y) + 2,
                         ENT16(g_c72c, ENT_MODE));
    }
}

static void ski_mouse_aim(short x, short y)
{
    if (g_c760 != 0 &&
        (x != g_c700 || y != g_c70c) &&
        g_c72c != NULL &&
        ENT32(g_c72c, ENT_FRAME) != 0xb &&
        ENT32(g_c72c, ENT_FRAME) != 0x11) {
        /* Disasm 0x406587-0x40659a: ECX = mouseX(c700) - c704.lo (center X),
         * EDX = mouseY(c70c) - c5fc.lo (center Y). No axis swap. */
        short adx = x - g_c704;
        short ady = y - g_c5fc;
        uint32_t f;
        if (ENT16(g_c72c, ENT_MODE) == 0)
            f = ski_aim_facing(adx, ady); /* 0x4065e0 */
        else
            f = ski_aim_crouch(adx, ady); /* 0x406670 (zero-extended byte) */
        ski_set_frame(g_c72c, f);
    }
    g_c700 = x;
    g_c70c = y;
    g_c760 = 1;
}

static void ski_click_action(void)
{
    /* WM_LBUTTONDOWN / WM_RBUTTONDOWN share this path (0x4066d0). */
    if (g_c72c == NULL) {
        ski_restart();
        return;
    }
    uint32_t frame = ENT32(g_c72c, ENT_FRAME);
    uint32_t new_frame = frame;
    if (frame != 0xb) {
        if (ENT16(g_c72c, ENT_MODE) == 0) {
            ENT16W(g_c72c, ENT_CROUCH) = 4; /* jump flag */
            new_frame = 0xd;
        } else if (frame != 0x11) {
            switch (frame) {
            case 0xd: new_frame = 0x12; break;
            case 0xe: new_frame = 0x14; break;
            case 0xf: new_frame = 0x15; break;
            case 0x12: new_frame = 0x13; break;
            case 0x13: new_frame = 0xd; break;
            default: break; /* 0x10/0x11: no change */
            }
        }
    }
    if (new_frame != frame) {
        ski_set_frame(g_c72c, new_frame);
        if (g_c610 != 0) {
            ski_render(g_c63c, &g_c6b0);
            g_c610 = 0;
        }
    }
}

/* ---------------- main window handlers (0x405a40/5ec0/5fa0/60b0) ---------------- */

static int wm_create_main(HWND h)
{
    g_c63c = GetDC(h);
    if (g_c63c == NULL)
        return 0;
    g_c710 = NULL;
    g_c6a4 = NULL;
    g_c730 = NULL;
    g_c6ec = NULL;
    g_c5ec = NULL;
    g_c620 = NULL;
    g_c6d4 = NULL;
    g_c644 = NULL;
    g_c75c = NULL;
    g_c614 = NULL;
    if (ski_load_bitmaps(g_c63c) == 0) {
        ski_fatal_msg("Whoa, like, can't load bitmaps!  Yer outa memory, duuude!");
        return 0;
    }
    return 1;
}

static void wm_destroy_main(HWND h)
{
    ReleaseDC(h, g_c63c);
    ski_pause();
    if (g_c620 != NULL) {
        HGDIOBJ old = SelectObject(g_c710, g_c620);
        DeleteObject(old);
    }
    if (g_c644 != NULL) {
        HGDIOBJ old = SelectObject(g_c730, g_c644);
        DeleteObject(old);
    }
    if (g_c6d4 != NULL) {
        HGDIOBJ old = SelectObject(g_c6a4, g_c6d4);
        DeleteObject(old);
    }
    if (g_c75c != NULL) {
        HGDIOBJ old = SelectObject(g_c6ec, g_c75c);
        DeleteObject(old);
    }
    if (g_c614 != NULL) {
        HGDIOBJ old = SelectObject(g_c5ec, g_c614);
        DeleteObject(old);
    }
    if (g_c710 != NULL)
        DeleteDC(g_c710);
    if (g_c730 != NULL)
        DeleteDC(g_c730);
    if (g_c6a4 != NULL)
        DeleteDC(g_c6a4);
    if (g_c6ec != NULL)
        DeleteDC(g_c6ec);
    if (g_c5ec != NULL)
        DeleteDC(g_c5ec);
}

static void wm_size_main(HWND h)
{
    g_c760 = 0;
    GetClientRect(h, &g_c6b0);
    /* 0x405fa0-0x405feb: param1 = (left+right)/2 -> c704 (center X);
     * param2 = (top+bottom)/3 (0x405fcc imul 0x55555556 high part) ->
     * c5fc (vertical anchor, 1/3 down, NOT the center). T12 render
     * exposed the old /2: scene shifted ~123px down vs the original. */
    ski_size_hook((short)((g_c6b0.right + g_c6b0.left) / 2),
                  (short)((g_c6b0.bottom + g_c6b0.top) / 3)); /* 0x406060 */
    g_c684 = g_c6b0.top - 0x78;
    g_c6d8 = (uint16_t)(g_c6b0.bottom - g_c6b0.top);
    g_c688 = g_c6b0.right + 0x78;
    g_c68c = g_c6b0.bottom + 0x78;
    g_c680 = g_c6b0.left - 0x78;
    g_c5f0 = (uint16_t)(g_c6b0.right - g_c6b0.left);
    g_c748 = (uint32_t)((g_c68c - g_c680) * (g_c688 - g_c680));
}

static void wm_paint_main(HWND h)
{
    PAINTSTRUCT ps;
    BeginPaint(h, &ps);
    FillRect(ps.hdc, &ps.rcPaint, g_c69c);
    ski_paint_scene(ps.hdc, &ps.rcPaint); /* 0x406100: cull + ski_render */
    EndPaint(h, &ps);
}

static void status_reposition(void); /* 0x406890 (defined below) */

/* ---------------- main WndProc (0x405800) ---------------- */

#if SKI_HARNESS
static FILE *ski_dbg = NULL;
static void ski_trace(const char *what, UINT msg, WPARAM wp, LPARAM lp)
{
    if (ski_dbg == NULL && (ski_dbg = fopen("/tmp/ski_msgs", "a")) != NULL)
        fprintf(ski_dbg, "# ski wproc trace pid=%d\n", (int)GetProcessId(NULL));
    if (ski_dbg != NULL) {
        fprintf(ski_dbg, "%s msg=%x wp=%lx lp=%lx tick=%u c67c=%u c694=%lx c770=%u c6d0=%u\n",
                what, msg, (unsigned long)wp, (unsigned long)lp, g_ski_tick,
                g_c67c, (unsigned long)g_c694, g_c770, g_c6d0);
        fflush(ski_dbg);
    }
}
#else
#define ski_trace(w, m, wp, lp)
#endif

LRESULT CALLBACK ski_wproc_main(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    ski_trace("wp", msg, wp, lp);
    if (msg < 0x25) {
        if (msg == 0x24) {
            /* Message 0x24 (36 — an unused Windows message number; NOT
             * WM_NCCREATE, which is 0x81). Transcribed verbatim (0x4058fd):
             * stores 0x140/0x12c at lParam+0x18/+0x1c and returns 0. Windows
             * never delivers message 36, so this path is dead in practice
             * (the window geometry is the plain CreateWindowExA args:
             * 768x768 at ((HORZ-768)/2, 0) -> 760x734 client at (132,30) on
             * the 1024x768 Xvfb). */
            *(uint32_t *)((char *)lp + 0x18) = 0x140;
            *(uint32_t *)((char *)lp + 0x1c) = 0x12c;
            return 0;
        }
        switch (msg) {
        case 1: /* WM_CREATE */
            if (wm_create_main(h) == 0)
                return -1;
            wm_size_main(h);
            return 0;
        case 2: /* WM_DESTROY */
            wm_destroy_main(h);
            PostQuitMessage(0);
            return 0;
        case 5: /* WM_SIZE */
            wm_size_main(h);
            if (g_c624 != NULL)
                status_reposition();
            g_c770 = (wp == 1); /* SIZE_MINIMIZED */
            ski_pause_auto();
            if (g_c67c != 0) {
                UpdateWindow(g_c6c8);
                return 0;
            }
            break;
        case 6: /* WM_ACTIVATE */
            g_c694 = wp;
            if (wp != 0)
                SetFocus(h);
            ski_pause_auto();
            return 0;
        case 0xf: /* WM_PAINT */
            wm_paint_main(h);
            return 0;
        case 0x21: /* WM_MOUSEACTIVATE */
            /* (0x4058e8): if the mouse screen-x ((short)lParam) == 1 ->
             * return 2 (MA_ACTIVATEANDEAT); else fall through -> return 0. */
            if ((short)lp == 1)
                return 2;
        }
    } else if (msg < 0x201) {
        if (msg == 0x200) { /* WM_MOUSEMOVE */
            if (g_c67c != 0) {
                ski_mouse_aim((short)lp, (short)(lp >> 0x10));
                return 0;
            }
        } else if (msg == 0x100) { /* WM_KEYDOWN (no WM_KEYUP path exists) */
            if (g_c67c != 0) {
                ski_key_down(wp);
                return 0;
            }
        } else if (msg == 0x102) { /* WM_CHAR */
            if (g_c67c != 0) {
                ski_char_key(wp);
                return 0;
            }
        } else {
            return DefWindowProcA(h, msg, wp, lp);
        }
    } else {
        if (msg == 0x201 /* WM_LBUTTONDOWN */ ||
            msg == 0x203 /* WM_RBUTTONDOWN */) {
            if (g_c67c != 0)
                ski_click_action();
        } else {
            return DefWindowProcA(h, msg, wp, lp);
        }
    }
    return 0;
}

/* ---------------- status window (0x4068d0/6a70/6c80/6890/6970) ---------------- */

static int status_create(HWND h)
{
    g_c6cc = GetDC(h);
    if (g_c6cc == NULL)
        return 0;
    g_c664 = GetStockObject(10); /* stock object 10, transcribed as-is */
    if (g_c664 != 0)
        g_c664 = SelectObject(g_c6cc, g_c664);
    TEXTMETRICA tm;
    GetTextMetricsA(g_c6cc, &tm);
    g_c668 = (uint16_t)tm.tmHeight; /* line advance (TextOut y step) */
    g_c66a = 0;
    short label_w = 0, value_w = 0;
    const UINT ids_l[4] = { STR_TIME, STR_DIST, STR_SPEED, STR_STYLE };
    for (int i = 0; i < 4; i++) {
        const char *s = ski_str_cache(ids_l[i]);
        ski_text_extent(g_c6cc, &label_w, s, (int)lstrlenA(s));
    }
    const UINT ids_v[4] = { STR_TIME0, STR_DIST0, STR_SPEED0, STR_SCORE0 };
    for (int i = 0; i < 4; i++) {
        const char *s = ski_str_cache(ids_v[i]);
        ski_text_extent(g_c6cc, &value_w, s, (int)lstrlenA(s));
    }
    g_c66e = label_w;                        /* value column x = label_w + 2 */
    g_c66a = (uint16_t)(g_c668 * 4);         /* panel height base */
    g_c66c = (uint16_t)(label_w + value_w);  /* panel width base */
    return 1;
}

static void status_destroy(HWND h)
{
    if (h != g_c624)
        ski_assert_fail(SKI_ASSERT_FILE, 0x1123);
    if (g_c664 != 0)
        SelectObject(g_c6cc, g_c664);
    ReleaseDC(h, g_c6cc);
}

static void status_reposition(void)
{
    int w = (int)g_c66c + 4;
    MoveWindow(g_c624, g_c6b0.right - w, g_c6b0.top, w, (int)g_c66a + 4, 1);
}

static void status_paint(HWND h)
{
    PAINTSTRUCT ps;
    BeginPaint(h, &ps);
    /* stock 4: under wine (the reference) a solid BLACK brush — FrameRect
     * draws the 1px black ring around the panel (verified in original
     * frames: (0,0,0) at panel rows 0/51, cols 0/122). Real-GDI "out of
     * range -> NULL" was the disproven T16 note. */
    HBRUSH hbr = (HBRUSH)GetStockObject(4);
    FrameRect(ps.hdc, &g_c778, hbr);
    /* y starts at 2: the prologue (0x40697d) writes DWORD 0x00000002 into the
     * y slot and x=2 is pushed per call (0x4069c2); Ghidra rendered that
     * single DWORD as short[2] {2,0} — the misread shipped T12 and left the
     * labels 2px above the values (invisible to M2: the diff masks the
     * panel). Verified against original pixels (T at row 4 -> pen row 2)
     * and a 0/6050px panel simulation (M3 T19 follow-up). */
    short cursor[2] = { 2, 2 }; /* x, y */
    const UINT ids[4] = { STR_TIME, STR_DIST, STR_SPEED, STR_STYLE };
    for (int i = 0; i < 4; i++) {
        const char *s = ski_str_cache(ids[i]);
        ski_text_draw(ps.hdc, s, 2, cursor, (int)lstrlenA(s));
    }
    ski_status_draw_values(ps.hdc); /* 0x401b80 (T12); stamps c5dc = c698 */
    EndPaint(h, &ps);
}

LRESULT CALLBACK ski_wproc_status(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case 1: /* WM_CREATE */
        if (status_create(h) == 0)
            return -1;
        /* fall through: measure client, then default */
    case 5: /* WM_SIZE */
        GetClientRect(h, &g_c778);
        break;
    case 2: /* WM_DESTROY */
        status_destroy(h);
        return 0;
    case 0xf: /* WM_CLOSE */
        status_paint(h);
        return 0;
    default:
        break;
    }
    return DefWindowProcA(h, msg, wp, lp);
}

/* ---------------- create windows (0x4052d0) ---------------- */

static int ski_create_windows(HINSTANCE hInstance, HINSTANCE hPrev, int show)
{
    HDC hdc = GetDC(NULL);
    if (hdc == NULL)
        return 0;
    g_c6a0 = (uint16_t)GetDeviceCaps(hdc, 8);  /* HORZRES */
    g_c74c = (uint16_t)GetDeviceCaps(hdc, 10); /* VERTRES */
    ReleaseDC(NULL, hdc);

    g_c61c = hInstance;
    g_c69c = (HBRUSH)GetStockObject(0); /* stock 0: out of range -> NULL
                                         * under real GDI; FillRect no-ops */
    g_c6c8 = NULL;
    g_c624 = NULL;
    g_c6d0 = 0;
    g_c770 = 1; /* starts "minimized": game begins paused */
    g_c694 = 0;
    g_c67c = 0;
    g_c704 = 0;
    g_c5fc = 0;

    g_c6c8 = FindWindowA("SkiMain", NULL);
    if (g_c6c8 != NULL) { /* single-instance guard */
        SetWindowPos(g_c6c8, NULL, 0, 0, 0, 0, 3 /* SWP_NOSIZE|SWP_NOMOVE */);
        if (IsIconic(g_c6c8))
            OpenIcon(g_c6c8);
        g_c6c8 = NULL;
        return 0;
    }

    g_c940 = (void *)ski_tick_cb;
    if (g_c794 == 0 && ski_sound_init()) {
        ski_sound_load(1, &ski_sound_1);
        ski_sound_load(2, &ski_sound_2);
        ski_sound_load(3, &ski_sound_3);
        ski_sound_load(4, &ski_sound_4);
        ski_sound_load(5, &ski_sound_5);
        ski_sound_load(6, &ski_sound_6);
        ski_sound_load(9, &ski_sound_7); /* id 9 -> slot 7 (original order) */
        ski_sound_load(7, &ski_sound_8);
        ski_sound_load(8, &ski_sound_9);
    }

    if (hPrev == NULL) {
        WNDCLASSA wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.style = 0x2023; /* CS_VVREDRAW|CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS */
        wc.lpfnWndProc = ski_wproc_main;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIconA(hInstance, "iconSki");
        wc.hCursor = LoadCursorA(NULL, (LPCSTR)0x7f00 /* IDC_ARROW */);
        wc.hbrBackground = g_c69c;
        wc.lpszClassName = "SkiMain";
        if (RegisterClassA(&wc) == 0)
            return 0;
        wc.lpfnWndProc = ski_wproc_status;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursorA(NULL, (LPCSTR)0x7f00);
        wc.lpszClassName = "SkiStatus";
        wc.hbrBackground = g_c69c;
        if (RegisterClassA(&wc) == 0)
            return 0;
    }

    short nWidth = (short)g_c6a0;
    if ((short)g_c74c <= nWidth)
        nWidth = (short)g_c74c; /* nWidth = min(HORZRES, VERTRES) */
    short nHeight = (short)g_c74c;
    int x = ((int)nWidth ? ((int)g_c6a0 - nWidth) / 2 : 0);
    const char *title = ski_str_cache(STR_TITLE);
    g_c6c8 = CreateWindowExA(0, "SkiMain", title, 0x2cf0000, x, 0,
                             (int)nWidth, (int)nHeight, NULL, NULL,
                             hInstance, NULL);
    if (g_c6c8 != NULL) {
        g_c624 = CreateWindowExA(0, "SkiStatus", g_c788,
                                 0x40000000 /* WS_CHILD */, 0, 0, 0, 0,
                                 g_c6c8, NULL, hInstance, NULL);
        if (g_c624 != NULL) {
            ShowWindow(g_c6c8, show);
            UpdateWindow(g_c6c8);
            ShowWindow(g_c624, 1 /* SW_SHOW */);
            UpdateWindow(g_c624);
            return 1;
        }
        DestroyWindow(g_c6c8);
        return 0;
    }
    return 0;
}

/* ---------------- WinMain (0x4047e0) ---------------- */

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmd, int show)
{
    if (lstrcmpiA(cmd, "nosound") == 0)
        g_c794 = 1;
    if (ski_init_mem() == 0)
        return 0;
    if (ski_game_reset() == 0)
        return 0;
    if (ski_create_windows(hInstance, hPrev, show) == 0)
        return 0;
    if (ski_game_start() == 0) {
        DestroyWindow(g_c6c8);
        ski_cleanup();
        return 0;
    }
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    ski_cleanup();
    return (int)msg.wParam;
}
