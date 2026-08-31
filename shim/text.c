/* GDI text: the captured pixel-exact font (Task 19).
 *
 * Font path (full evidence chain in harness/fontcap/fontcap.c header): the
 * game creates no font (no CreateFont in the 96-entry IAT) and the class
 * records select none, so a fresh status DC would carry wine's 16px "System"
 * VGA font. wproc_status_create (FUN_00406a70.c) instead does
 *     g_c664 = SelectObject(g_c6cc, GetStockObject(10));
 * and under the reference's wine 9.0 stock index 10 resolves to a 12px
 * MONOSPACE TTF (empty face name, 7px advance, tmHeight 12, ascent 9,
 * descent 3) — not the BLACK_BRUSH that real Win32 gives for 10 (types.h's
 * stock table and API.md line 108 state the real-Win32 identity; the
 * reference frames were produced under wine, so the shim follows wine).
 * The panel geometry cross-check pins it: 6ch*7 + 11ch*7 + 4 = 123 = the
 * reference panel width, 4*12 + 4 = 52 = the reference panel height.
 * Every TextOutA / GetTextExtentPoint32A / GetTextMetricsA call in the game
 * runs on the status DC (labels at x=2, values at label_w+2, ski_text_draw
 * 0x401e20 / ski_status_draw_values 0x401b80 / ski_text_extent 0x406c50),
 * so the glyphs + advances captured under the same wine 9.0 + prefix
 * (harness/fontcap -> font.inc / web/assets/font.json) are exactly what the
 * reference rendered.
 *
 * Rendering: (x, y) is the GDI pen origin. Each char is a captured STRIP
 * (font.inc): columns [strip_l .. strip_l+strip_w-1] relative to the pen,
 * row 0 = text top (baseline at tmAscent). The strip is wider than the
 * advance because the reference's subpixel-AA fringe is not advance-
 * confined: a glyph's left fringe lands 1px LEFT of its pen (inside the
 * previous char's last column — verified in the reference panel, where
 * the 'm' of "Time:" tints the 'i' col 6 to B=253 and the 'm' of "   00m"
 * tints the preceding '0' col 6 to R~192-247/B=253), while ink ends at/
 * inside advance+1. The pen advances by the captured per-char advance
 * (NOT tmAveCharWidth — ski_text_extent sums those same advances for the
 * panel width) and chars are composed left-to-right, so a later glyph's
 * left fringe overwrites the previous glyph's overhang exactly like GDI.
 *
 * Compositing is GDI's subpixel blend, dst_c = dst_c * cap_c / 255 (per
 * channel, rounded): the captured pixels encode the per-subchannel
 * coverage of the black text on the white capture background (cap_c =
 * 255 - 255*coverage), and the game never calls SetTextColor (not in the
 * IAT) so the source color is black — blending over any existing dst is
 * dst*(1-coverage) = dst*cap/255. Verified against the reference:
 * 'm'-fringe (255,255,253) over the '0' edge (206,251,255) yields the
 * reference's (206,251,253). Pure-white strip pixels are skipped (they
 * leave dst untouched). Codes outside 32..126 have no glyph and zero
 * advance (the game only prints ASCII: wsprintf-formatted digits/labels
 * and the ski_str_cache strings).
 *
 * GetDeviceCaps: the reference screen was 1024x768 (evidence/
 * m0-geometry.txt); ski_win.c:772-773 stores indices 8/10 into
 * c6a0/c74c for the 768x768 window sizing — HORZRES(8) -> 1024,
 * VERTRES(10) -> 768, anything else -> 0.
 */
#include <stddef.h>
#include "win32.h"
#include "surface.h"
#include "font.inc"

BOOL TextOutA(HDC dc, int x, int y, LPCSTR s, int cnt)
{
    uint8_t *surf;
    int W, H;
    if (dc == NULL || s == NULL)
        return FALSE;
    shim_dc_px(dc, (const uint8_t **)&surf);
    if (surf == NULL)
        return TRUE; /* DC with no own surface: nothing to draw on */
    shim_dc_size(dc, &W, &H);
    for (int i = 0; i < cnt; i++) {
        int code = (unsigned char)s[i];
        if (code < FONT_FIRST || code > FONT_LAST)
            continue; /* no glyph, no advance */
        const unsigned char *strip = g_font.px[code - FONT_FIRST];
        for (int cy = 0; cy < FONT_STRIP_H; cy++) {
            int dy = y + cy;
            if (dy < 0 || dy >= H)
                continue;
            uint8_t *row = surf + (size_t)dy * W * 4;
            const unsigned char *src = strip + (size_t)cy * FONT_STRIP_W * 3;
            for (int cx = 0; cx < FONT_STRIP_W; cx++) {
                int dx = x + g_font.strip_l + cx;
                if (dx < 0 || dx >= W)
                    continue;
                const unsigned char *p = src + (size_t)cx * 3;
                if (p[0] == 0xFF && p[1] == 0xFF && p[2] == 0xFF)
                    continue; /* pure background: dst untouched */
                uint8_t *q = row + (size_t)dx * 4;
                q[0] = (uint8_t)(((uint32_t)q[0] * p[0] + 127) / 255);
                q[1] = (uint8_t)(((uint32_t)q[1] * p[1] + 127) / 255);
                q[2] = (uint8_t)(((uint32_t)q[2] * p[2] + 127) / 255);
            }
        }
        x += g_font.adv[code - FONT_FIRST];
    }
    return TRUE;
}

BOOL GetTextExtentPoint32A(HDC dc, LPCSTR s, int cnt, SIZE *sz)
{
    (void)dc;
    if (sz == NULL)
        return FALSE;
    int w = 0;
    for (int i = 0; i < cnt; i++) {
        int code = (unsigned char)s[i];
        if (code >= FONT_FIRST && code <= FONT_LAST)
            w += g_font.adv[code - FONT_FIRST];
    }
    sz->cx = w;
    sz->cy = g_font.tm.tmHeight;
    return TRUE;
}

BOOL GetTextMetricsA(HDC dc, TEXTMETRICS *tm)
{
    (void)dc;
    if (tm == NULL)
        return FALSE;
    *tm = g_font.tm; /* captured wine-9.0 stock-10 metrics (font.inc) */
    return TRUE;
}

int GetDeviceCaps(HDC dc, int index)
{
    (void)dc;
    switch (index) {
    case HORZRES: /* 8 */
        return 1024;
    case VERTRES: /* 10 */
        return 768;
    default:
        return 0;
    }
}
