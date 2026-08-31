/* fontcap.c — pixel-exact capture of the font the ski32 rebuild renders with.
 *
 * FONT PATH (Task 19 step 1; evidence: decompile/ghidra + decompile/NOTES.md
 * + empirical stock-object enumeration under the reference's wine 9.0):
 *   - ski32.exe's IAT has no font-creating API (no CreateFont among the 96
 *     imports — shim/API.md cross-check).
 *   - Both class records (FUN_004052d0.c — SkiMain 0x405350, SkiStatus
 *     0x405480) never write a useful hFont: the original leaves the word
 *     stack-garbage, the rebuild zero-initializes the record (src/ski_win.c
 *     ski_create_windows) — either way the class font does not select a real
 *     font. A fresh window DC under wine 9.0 carries the GDI "System" font
 *     (vgasys.fon, 16px 1-bit VGA) — which is NOT what the game's text uses.
 *   - wproc_status_create (FUN_00406a70.c) does
 *       c664 = SelectObject(c6cc, GetStockObject(10))
 *     Real Win32 stock 10 is BLACK_BRUSH (a brush) and NOTES.md line 779
 *     mislabels it "OEM_FIXED_FONT". But the reference runs under wine 9.0,
 *     and wine 9.0's GetStockObject(10) returns a FONT: face "" (empty
 *     name), lfHeight 12, weight 400, tmHeight 12, tmAscent 9, tmDescent 3,
 *     tmAveCharWidth 7, tmMaxCharWidth 13 — a 12px MONOSPACE TTF (every
 *     ASCII advance = 7). Selecting it replaces the status DC's font.
 *     (Measured by selecting every stock object 0..25 into a DC under
 *     WINEPREFIX=~/.wine-ski, wine-9.0 9.0~repack-4build3: 0-5/9/20-25 are
 *     NULL or non-fonts, 10/11/12/16/17 each install a font.)
 *   - Every text draw (labels + values via ski_text_draw, 0x401e20) and
 *     every extent/metrics query (0x406c50, GetTextMetricsA) runs on the
 *     status window's DCs — the BeginPaint DC and the c6cc GetDC, the same
 *     DC object — after that SelectObject. The game therefore renders ALL
 *     its text with wine 9.0's stock-10 font (subpixel-AA TTF on Xvfb —
 *     the reference frames show ClearType-style blue/red edge fringes).
 *   - Panel geometry cross-check (monospace 7px): label_w = 6ch*7 = 42
 *     ("Speed:" is longest), value_w = 11ch*7 = 77 (" 0:00:00.00" sizing
 *     string), panel w = 42 + 77 + 4 = 123 and h = 4*12 + 4 = 52 — exactly
 *     the reference status window (evidence/m0-status-window.png, and the
 *     T18 rebuild run 2026-08-31 on the house Xvfb :99).
 *   - CONTEXT MATTERS: wine 9.0's X11 driver renders a WS_CHILD window
 *     into the PARENT's X window, and that path produces a 2/255 blue-
 *     channel subpixel fringe at glyph edges that a top-level window's
 *     own DC does not (verified: the same "Time:" at (2,2) renders B=255
 *     in a top-level DC and B=253 in the child region; the reference
 *     panel carries the B=253 fringe). The capture therefore reproduces
 *     the game's context exactly: a WS_CHILD sheet window painting in
 *     WM_PAINT, read out through the parent window's DC (BitBlt) — the
 *     same pixels an external X11 import of the parent sees.
 *
 * LAYOUT: the sheet is an 8x12 grid of cells, one per printable ASCII code
 * 32..126 (row-major). Each cell slot is CELLW x CELLH with CELLW =
 * MARGIN + maxAdvance + RIGHTGUARD (12) and CELLH = tmHeight. The wide
 * slot is deliberate: the reference's subpixel-AA fringes are NOT confined
 * to the 8px crop the advance implies — a glyph's left subpixel fringe
 * lands 1px LEFT of its own pen (inside the previous char's last columns;
 * verified: "Time:" shows B=253 at the 'i' col 6 caused by the following
 * 'm' drawn later, while "Ti e:"/"Tie:" leave it white), and the right
 * fringe runs past the advance. With 19px of isolation on each side no
 * slot carries a neighbor's ink, so fontcap_parse.py can measure each
 * glyph's true extent (min..max non-white column, relative to the pen)
 * and crop a common-width strip [L..R] per char. The shim draws each
 * char's strip at (x + L) in left-to-right order, exactly like GDI's
 * per-glyph composition: a later glyph's strip overwrites the previous
 * glyph's overhanging fringe. All cells are drawn at integer (x, y) in
 * the child DC, like every game TextOutA call (x = 2 or 44,
 * y = 2 + 12*k).
 *
 * OUTPUTS (CWD):
 *   fontcap.txt — "fontcap 1" magic, "face" line (identity of the selected
 *                 stock-10 font), sheet W H, cell MARGIN CELLW CELLH,
 *                 17 metrics ints (TEXTMETRICS field order), one
 *                 "extent CODE ADVANCE" per char.
 *   fontcap.bmp — 24bpp bottom-up BMP of the sheet (BI_RGB, stride 4-aligned),
 *                 read out from the PARENT DC over the child's region.
 *
 * Build: i686-w64-mingw32-gcc fontcap.c -o fontcap.exe -lgdi32 -luser32
 * Run:   WINEPREFIX=$HOME/.wine-ski xvfb-run -a wine ./fontcap.exe
 *        (ephemeral display; font metrics AND the subpixel fringe were
 *        verified byte-identical between the house Xvfb :99 and xvfb-run's
 *        default screen, so the display choice does not affect the data.)
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIRST  32
#define LAST   126
#define NCHARS (LAST - FIRST + 1)
#define COLS   8
#define ROWS   12
#define MARGIN 2
#define RIGHTGUARD 12

static HWND g_parent;
static HWND g_child;
static int g_measuring; /* set before CreateWindowEx(child) */
static int g_done;

/* measured in the child's WM_CREATE (the game's status_create sequence) */
static LOGFONTA g_lf;
static TEXTMETRICA g_tm;
static int g_adv[NCHARS];
static int g_cellw, g_cellh, g_W, g_H;

static LRESULT CALLBACK wpt(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_CREATE && g_measuring) {
        /* wproc_status_create's exact query sequence on the child DC.
         * (g_child is not assigned until CreateWindowEx returns, so the
         * pre-set flag, not the handle, identifies this window.) */
        g_measuring = 0;
        g_child = h;
        HDC dc = GetDC(h);
        HGDIOBJ stock10 = GetStockObject(10);
        if (!stock10 || !SelectObject(dc, stock10))
            return -1;
        HFONT cur = (HFONT)GetCurrentObject(dc, OBJ_FONT);
        if (!cur || !GetObjectA(cur, sizeof(g_lf), &g_lf) ||
            !GetTextMetricsA(dc, &g_tm) || g_tm.tmHeight <= 0)
            return -1;
        int max_adv = 0;
        for (int k = 0; k < NCHARS; k++) {
            char ch = (char)(FIRST + k);
            SIZE sz;
            if (!GetTextExtentPoint32A(dc, &ch, 1, &sz))
                return -1;
            g_adv[k] = sz.cx;
            if (sz.cx > max_adv)
                max_adv = sz.cx;
        }
        ReleaseDC(h, dc);
        g_cellw = MARGIN + max_adv + RIGHTGUARD;
        g_cellh = g_tm.tmHeight;
        g_W = COLS * g_cellw;
        g_H = ROWS * g_cellh;
        return 0;
    }
    if (m == WM_PAINT && h == g_child && !g_done) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT all = { 0, 0, g_W, g_H };
        FillRect(dc, &all, (HBRUSH)GetStockObject(WHITE_BRUSH));
        SelectObject(dc, GetStockObject(10));
        for (int k = 0; k < NCHARS; k++) {
            char ch = (char)(FIRST + k);
            TextOutA(dc, (k % COLS) * g_cellw + MARGIN, (k / COLS) * g_cellh,
                     &ch, 1);
        }
        EndPaint(h, &ps);
        g_done = 1;
        /* Read out through the PARENT DC over the child region: the child
         * has no separate X window (wine draws it into the parent's X
         * window), and only the composited pixels carry the reference's
         * subpixel fringe. */
        HDC pdc = GetDC(g_parent);
        HDC mdc = CreateCompatibleDC(pdc);
        HBITMAP mbm = CreateCompatibleBitmap(pdc, g_W, g_H);
        HGDIOBJ old = SelectObject(mdc, mbm);
        int ok = BitBlt(mdc, 0, 0, g_W, g_H, pdc, 0, 0, SRCCOPY);
        BITMAPINFO bi;
        ZeroMemory(&bi, sizeof bi);
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = g_W;
        bi.bmiHeader.biHeight = g_H; /* bottom-up */
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 24;
        int stride = (g_W * 3 + 3) & ~3;
        unsigned char *px = malloc((size_t)stride * g_H);
        if (ok && px &&
            GetDIBits(mdc, mbm, 0, (unsigned)g_H, px, &bi, DIB_RGB_COLORS) == g_H) {
            FILE *fo = fopen("fontcap.txt", "w");
            FILE *fb = fopen("fontcap.bmp", "wb");
            if (fo && fb) {
                fprintf(fo, "fontcap 1\n");
                fprintf(fo, "face %.31s %d %d %d\n", g_lf.lfFaceName,
                        (int)g_lf.lfHeight, (int)g_lf.lfWidth,
                        (int)g_lf.lfWeight);
                fprintf(fo, "sheet %d %d\n", g_W, g_H);
                fprintf(fo, "cell %d %d %d\n", MARGIN, g_cellw, g_cellh);
                fprintf(fo, "metrics %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        (int)g_tm.tmHeight, (int)g_tm.tmAscent, (int)g_tm.tmDescent,
                        (int)g_tm.tmInternalLeading, (int)g_tm.tmExternalLeading,
                        (int)g_tm.tmAveCharWidth, (int)g_tm.tmMaxCharWidth,
                        (int)g_tm.tmWeight, (int)g_tm.tmOverhang,
                        (int)g_tm.tmDigitizedAspectX, (int)g_tm.tmFirstChar,
                        (int)g_tm.tmLastChar, (int)g_tm.tmDefaultChar,
                        (int)g_tm.tmBreakChar, (int)g_tm.tmItalic,
                        (int)g_tm.tmUnderlined, (int)g_tm.tmStruckOut);
                for (int k = 0; k < NCHARS; k++)
                    fprintf(fo, "extent %d %d\n", FIRST + k, g_adv[k]);
                fclose(fo);
                int datasz = stride * g_H;
                unsigned char fh[14];
                int v;
                memcpy(fh, "BM", 2);
                v = 14 + 40 + datasz;
                memcpy(fh + 2, &v, 4);
                v = 0;
                memcpy(fh + 6, &v, 4);
                v = 14 + 40;
                memcpy(fh + 10, &v, 4);
                fwrite(fh, 1, 14, fb);
                fwrite(&bi, 1, 40, fb); /* bottom-up: the rows exactly as
                                           GetDIBits returned them (an extra
                                           flip mirrors the sheet — verified) */
                fwrite(px, 1, datasz, fb);
                fclose(fb);
            } else {
                if (fo)
                    fclose(fo);
                if (fb)
                    fclose(fb);
                ok = 0;
            }
        } else {
            ok = 0;
        }
        free(px);
        SelectObject(mdc, old);
        DeleteObject(mbm);
        DeleteDC(mdc);
        ReleaseDC(g_parent, pdc);
        if (!ok) {
            fprintf(stderr, "sheet readout failed\n");
            PostQuitMessage(1);
        } else {
            PostQuitMessage(0);
        }
        return 0;
    }
    return DefWindowProcA(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s)
{
    (void)p; (void)c; (void)s;
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof wc);
    wc.cbSize = sizeof wc;
    wc.lpfnWndProc = wpt;
    wc.hInstance = i;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "fontcap";
    if (!RegisterClassEx(&wc)) {
        fprintf(stderr, "RegisterClassEx failed\n");
        return 1;
    }
    g_parent = CreateWindowEx(0, "fontcap", "fontcap", WS_VISIBLE,
                              0, 0, 400, 300, 0, 0, i, 0);
    if (!g_parent) {
        fprintf(stderr, "CreateWindowEx(parent) failed\n");
        return 1;
    }
    /* Child created 0x0; its WM_CREATE measures the font (synchronous),
     * then it is grown to the sheet — the game's status window follows the
     * same create-small/MoveWindow-late pattern (status_reposition). */
    g_measuring = 1;
    g_child = CreateWindowEx(0, "fontcap", "", WS_CHILD | WS_VISIBLE,
                             0, 0, 1, 1, g_parent, 0, i, 0);
    if (!g_child) {
        fprintf(stderr, "CreateWindowEx(child) failed\n");
        return 1;
    }
    if (g_W <= 0) {
        fprintf(stderr, "font measurement failed (WM_CREATE)\n");
        return 1;
    }
    /* parent client must cover the sheet (400x300 window -> client is
     * smaller by the frame; the measured sheet is 120x96, far inside) */
    if (g_W > 390 || g_H > 290) {
        MoveWindow(g_parent, 0, 0, g_W + 20, g_H + 40, 1);
    }
    MoveWindow(g_child, 0, 0, g_W, g_H, 1);
    InvalidateRect(g_child, 0, 1);
    MSG m;
    while (GetMessage(&m, 0, 0, 0)) {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }
    return (int)m.wParam;
}
