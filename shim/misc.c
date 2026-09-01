/* Shim misc — the remaining T16-probe symbols (Task 20): sprites, the
 * string table, stock objects, FillRect/FrameRect, wsprintf, the resource
 * no-ops, LocalAlloc, the lstr* trio, INI, PlaySoundA, and MessageBoxA.
 *
 * Every symbol below was traced against its src/ call sites on 2026-09-01
 * (HEAD 8a98221); the citations are in the per-symbol notes. The
 * MessageBoxA modal design (pump keeps running + answer hook + per-site
 * epilogue) is documented in shim/API.md and in the MessageBoxA note
 * below.
 */
#include <emscripten.h>
#include <emscripten/em_js.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "surface.h"
#include "sprites.inc"
#include "win.h"
#include "win32.h"

/* ---- sprites ----------------------------------------------------------- */
/* LoadBitmapA — ski_load_bitmap (ski_core.c:220-222), called for every id
 * 1..0x59 twice: the sizing pass (ski_core.c:233-262, GetObjectA w/h +
 * DeleteObject) and the blit pass (ski_core.c:288-340, SelectObject +
 * SRCCOPY image strip + NOTSRCCOPY mask strip + DeleteObject). Each call
 * gets a fresh bitmap: the game deletes the one it was handed.
 *
 * The resources are 4bpp indexed DIBs in the PE; the embedded pixels
 * (sprites.inc) are their palette expansion — exactly what the
 * reference's wine 9.0 produces after LoadBitmapA (wine converts the
 * loaded 4bpp DIB to a 32bpp device bitmap; T20 probe, see the
 * harness/embed_sprites.py header and surface.h). The NOTSRCCOPY 4bpp->1
 * mask pass is the "expanded color != white" rule (T17 + T20 probes),
 * which the 32bpp storage implements exactly. */
void *LoadBitmapA(HINSTANCE inst, LPCSTR name)
{
    (void)inst;
    unsigned id = (unsigned)(uintptr_t)name; /* MAKEINTRESOURCEA */
    for (int i = 0; i < (int)(sizeof g_sprites / sizeof g_sprites[0]); i++)
        if (g_sprites[i].id == id)
            return shim_bmp_from_rgb(g_sprites[i].w, g_sprites[i].h,
                                     g_sprites[i].px);
    return NULL;
}

/* ---- stock objects ------------------------------------------------------ */
/* GetStockObject — the raw indices the game passes (transcribed from the
 * original; API.md row): 0 (ski_win.c:775, class hbrBackground + the
 * FillRect brush g_c69c), 4 (ski_win.c:733, the status-panel FrameRect
 * brush), 10 (ski_win.c:686, SelectObject'd into the status DC as the
 * text font). Measured wine 9.0 behavior (T19 review, 66993e1 — the
 * reference ran on wine, so wine's table is the contract, not real
 * Win32's): 0 -> WHITE brush (windows erase white; the FillRect at
 * ski_win.c:570 fills the main client white), 1 -> 0xc0c0c0 brush,
 * 2..7 -> BLACK brush (4 draws the 1px black status-panel ring visible
 * in the M2 reference, evidence/m0-original-gameplay.png), 8 -> WHITE
 * brush, 10 -> the 12px MONOSPACE font (captured in shim/font.inc;
 * SelectObject accepts it as a current object, text.c renders from it).
 * Brushes are opaque {color} structs so FillRect/FrameRect can read the
 * color back by identity; the font is a tag struct (text.c's g_font is
 * the source; the handle only has to be non-NULL and stable). */
typedef struct { uint32_t color; } ShimBrush;
static ShimBrush g_brush_white  = { 0x00FFFFFFu };
static ShimBrush g_brush_ltgray = { 0x00C0C0C0u };
static ShimBrush g_brush_black  = { 0u };
static struct { int is_font; } g_stock_font = { 1 };

static const ShimBrush *as_brush(HBRUSH b)
{
    if (b == (HBRUSH)&g_brush_white)
        return &g_brush_white;
    if (b == (HBRUSH)&g_brush_ltgray)
        return &g_brush_ltgray;
    if (b == (HBRUSH)&g_brush_black)
        return &g_brush_black;
    return NULL;
}

HBRUSH GetStockObject(int idx)
{
    switch (idx) {
    case 0:
        return (HBRUSH)&g_brush_white;
    case 1:
        return (HBRUSH)&g_brush_ltgray;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return (HBRUSH)&g_brush_black;
    case 8:
        return (HBRUSH)&g_brush_white;
    case 10:
        return (HBRUSH)&g_stock_font;
    default: /* 9, 11..: not used by the game; unmeasured under wine */
        return NULL;
    }
}

/* FillRect — single call: ski_win.c:570 (main WM_PAINT) with g_c69c =
 * GetStockObject(0) = the wine WHITE brush -> fills the client white.
 * A NULL brush must no-op (API.md). Return value ignored by the game. */
BOOL FillRect(HDC dc, const RECT *rc, HBRUSH brush)
{
    const ShimBrush *b;
    if (!rc)
        return FALSE;
    b = as_brush(brush);
    if (!b)
        return TRUE; /* NULL (or non-stock) brush: no-op */
    shim_dc_fill(dc, rc->left, rc->top, rc->right - rc->left,
                 rc->bottom - rc->top, b->color);
    return TRUE;
}

/* FrameRect — single call: ski_win.c:729-734 (status WM_PAINT) with
 * GetStockObject(4) = the wine BLACK brush. T19 review (66993e1)
 * corrected the plan's "standard 2-line highlight/shadow 3D edge" to the
 * MEASURED reference: the panel ring is exactly 1px (246 dark px in the
 * original frame = 2*123 + 2*50 = top + bottom + left + right of a
 * 123x51 box). So: a 1px border in the brush color. NULL brush: no-op. */
BOOL FrameRect(HDC dc, const RECT *rc, HBRUSH brush)
{
    const ShimBrush *b;
    int l, t, w, h;
    if (!rc)
        return FALSE;
    b = as_brush(brush);
    if (!b)
        return TRUE; /* NULL (or non-stock) brush: no-op */
    l = rc->left;
    t = rc->top;
    w = rc->right - rc->left;
    h = rc->bottom - rc->top;
    if (w <= 0 || h <= 0)
        return FALSE;
    shim_dc_fill(dc, l, t, w, 1, b->color);           /* top */
    shim_dc_fill(dc, l, t + h - 1, w, 1, b->color);   /* bottom */
    if (h > 2) {
        shim_dc_fill(dc, l, t + 1, 1, h - 2, b->color);            /* left */
        shim_dc_fill(dc, l + w - 1, t + 1, 1, h - 2, b->color);    /* right */
    }
    return TRUE;
}

/* ---- string table ------------------------------------------------------- */
/* LoadStringA — every string in the game flows through ski_str_cache
 * (ski_core.c:122-135), which calls LoadStringA(g_c61c, id, buf, 0xff)
 * with the raw STR_* id (skidef.h:19-35) and copies n chars.
 *
 * The original PE's string store is two groups (resources.json "strings",
 * M1 extraction): group 1 has 16 entries, 1-BASED with a dummy "" at
 * [0] (ids 1..15: "SkiFree".."High Scores"), and group 2 has 2 entries
 * (the score-panel suffixes). The game reaches both groups ONLY through
 * LoadStringA — the suffixes included: ski_str_cache(STR_SUFFIX_YOU=16)
 * and ski_str_cache(STR_SUFFIX_TRY=17) (ski_core.c:2801/2810). Find-
 * ResourceA is called only with type "WAVE" (ski_sound_load,
 * ski_core.c:174) — never for strings — so the "group 2 via
 * FindResource/LockResource" reading of the plan does not occur in the
 * rebuilt sources; the trace above is the contract.
 *
 * The T19 strprobe run against the ORIGINAL under wine (src/resources.rc
 * header) verified the flat map this shim must reproduce: ids 1..17
 * resolve to the 17 strings below in order, id 0 and 18+ fail (0), no
 * empty entry. The plan's "index = id - 1" would have mapped id 1 onto
 * the group-1 dummy "" and shifted everything else; the array is 1-based
 * with the dummy at [0], exactly mirroring group 1. A standard
 * STRINGTABLE with ids 1..17 (resources.rc) gives the same mapping,
 * which is why the i686 rebuild matched the original.
 *
 * Return value: chars loaded (Win32), 0 on failure; ski_str_cache uses
 * it as the copy length (n + 1 bytes are LocalAlloc'd). */
static const char *const g_str[18] = {
    "",                              /* 0: dummy — id 0 fails */
    "SkiFree",                       /* 1  STR_TITLE */
    "Ski Paused ... Press F3 to continue", /* 2 STR_PAUSED */
    "Time:",                         /* 3  STR_TIME */
    "Dist:",                         /* 4  STR_DIST */
    "Speed:",                        /* 5  STR_SPEED */
    "Style:",                        /* 6  STR_STYLE */
    "00:00:00.00",                   /* 7  STR_TIME0 */
    " 0000m",                        /* 8  STR_DIST0 */
    " 0000m/s",                      /* 9  STR_SPEED0 */
    "0000000",                       /* 10 STR_SCORE0 */
    "%2u:%2.2u:%2.2u.%2.2u",         /* 11 STR_FMT_TIME */
    "%5.2dm",                        /* 12 STR_FMT_DIST */
    "%5.2dm/s",                      /* 13 STR_FMT_SPEED */
    "%7ld",                          /* 14 STR_FMT_SCORE */
    "High Scores",                   /* 15 STR_HIGHSCORE */
    " <-- that's you!",              /* 16 STR_SUFFIX_YOU (group 2, id 1) */
    " <-- try again!",               /* 17 STR_SUFFIX_TRY (group 2, id 2) */
};

int LoadStringA(HINSTANCE inst, UINT id, LPSTR buf, int max)
{
    (void)inst;
    if (!buf || max <= 0 || id < 1 || id > 17)
        return 0; /* id 0 and 18+ fail, as the original under wine */
    const char *s = g_str[id];
    int n = 0;
    while (s[n] && n < max - 1) {
        buf[n] = s[n];
        n++;
    }
    buf[n] = '\0';
    return n;
}

/* ---- wsprintf ------------------------------------------------------------ */
/* wsprintfA — traced formats (all sites): "%s line %u" (ski_core.c:150),
 * "frame_%06u_main.ppm" + "P6\n%d %d\n255\n" (harness-only, 2573/2579),
 * "%ld " (2788), "%9ld" (2796/2807), "%s" (2801/2810), "\n\n" (2805),
 * "%5.2dm" (3155), "%5.2dm/s" (3158), "%7ld" (3161),
 * "%2u:%2.2u:%2.2u.%2.2u" (3177). Every conversion in use is a plain C
 * printf conversion — nothing wsprintf-specific (%ls, %I64d, ...) — so
 * vsnprintf is exact; the 0x4000 bound is far above every caller's
 * worst case (largest real output: the 10-entry score line, ~110 chars
 * into buf[0x100]). Returns the char count written; the high-score
 * writer advances `p` by it (2788/2801/2805/2810). */
int wsprintfA(LPSTR buf, LPCSTR fmt, ...)
{
    va_list ap;
    int n;
    if (!buf || !fmt)
        return 0;
    va_start(ap, fmt);
    n = vsnprintf(buf, 0x4000, fmt, ap);
    va_end(ap);
    return n < 0 ? 0 : n;
}

/* ---- kernel32 misc --------------------------------------------------------- */
/* GetProcessId — harness-only (the T16 probe's +2): ski_trace's header
 * line (ski_win.c:584, SKI_HARNESS) reads it once; the wproc trace is
 * diagnostics, never diffed. Any fixed pid is faithful. */
DWORD GetProcessId(HANDLE proc)
{
    (void)proc;
    return 1;
}

/* LocalAlloc — the game's pools (ski_init_mem, ski_core.c:106-117:
 * 0x50/0x5a0/8000/0x2400 bytes) plus the string cache (ski_core.c:129,
 * n+1). The process never imports LocalFree — the pools live for the
 * process lifetime (NOTES; ski_cleanup frees sounds via FreeResource,
 * not the pools). calloc: zeroed like GMEM_ZEROINIT; the game memsets
 * the pools itself anyway (113). */
HLOCAL LocalAlloc(UINT flags, DWORD bytes)
{
    (void)flags;
    return (HLOCAL)calloc(1, bytes ? bytes : 1);
}

BOOL FreeLibrary(HMODULE mod)
{
    (void)mod;
    return TRUE; /* the sound module handle g_c78c is always 0 (static
                    import; API.md) — no module is ever opened */
}

/* FindResourceA — called ONLY with (g_c61c, MAKEINTRESOURCEA(id), "WAVE")
 * (ski_sound_load, ski_core.c:174, sound ids 1..9). The original PE has
 * no WAVE-type resource node, so the reference's wine returns NULL and
 * the game is silent by construction (NOTES M1#3; the M2 reference runs
 * silent). The web build has no resource database at all: NULL for every
 * (name, type) — which keeps the LoadResource/LockResource chain below
 * unreachable, exactly as in the reference. */
HRSRC FindResourceA(HMODULE mod, LPCSTR name, LPCSTR type)
{
    (void)mod; (void)name; (void)type;
    return NULL;
}

/* LoadResource / LockResource / FreeResource — reachable only if
 * FindResourceA returned non-NULL (ski_core.c:177-179, 192), which it
 * never does here (no WAVE node); defensive identity/no-op, matching
 * the API.md rows: LoadResource returns its input, LockResource a
 * pointer to the data, FreeResource a no-op. */
HGLOBAL LoadResource(HMODULE mod, HRSRC res)
{
    (void)mod;
    return res;
}

void *LockResource(HGLOBAL res)
{
    return res;
}

void FreeResource(HGLOBAL res)
{
    (void)res;
}

LPSTR lstrlenA(LPCSTR s)
{
    return (LPSTR)(s ? strlen(s) : 0);
}

LPSTR lstrcpyA(LPSTR dst, LPCSTR src)
{
    return dst ? strcpy(dst, src) : dst;
}

static int ci_eq(const char *a, size_t alen, const char *b)
{
    for (size_t i = 0; i < alen; i++) {
        char x = a[i], y = b[i];
        if (y == '\0')
            return 0;
        if (x >= 'A' && x <= 'Z')
            x += 0x20;
        if (y >= 'A' && y <= 'Z')
            y += 0x20;
        if (x != y)
            return 0;
    }
    return b[alen] == '\0';
}

int lstrcmpiA(LPCSTR a, LPCSTR b)
{
    if (!a || !b)
        return 0;
    /* single use: the WinMain "nosound" gate (ski_win.c:869) compares a
     * lowercase command line against a lowercase literal — a full
     * case-insensitive strcmp keeps the semantics exact. */
    size_t i = 0;
    while (a[i] || b[i]) {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z')
            x += 0x20;
        if (y >= 'A' && y <= 'Z')
            y += 0x20;
        if (x != y)
            return x < y ? -1 : 1;
        i++;
    }
    return 0;
}

/* ---- winmm ----------------------------------------------------------------- */
/* PlaySoundA — M1 decision: silent by default. The game takes our
 * address into g_c790 (ski_sound_init, ski_core.c:162-167) and calls it
 * through the pointer: stop (NULL, NULL, 0) in ski_cleanup (203, runs
 * at exit) and play (ptr, NULL, 0x8000) at 414 — unreachable: no WAVE
 * resource exists (FindResourceA -> NULL, 174). Log to the console;
 * never block, never allocate audio. (The original imports the WINMM
 * alias sndPlaySoundA; the shim exports the standard name the rebuild
 * calls — API.md.) */
/* EM_JS bodies called from C receive RAW POINTERS (no auto-marshal —
 * only the generated Module wrappers convert), so convert on the JS
 * side with UTF8ToString (the EM_JS body runs in runtime scope).
 * NULL -> empty, as the Win32 callers may pass (stop call, ski_core.c:203). */
EM_JS(void, console_c, (const char *s),
      { console.log(s ? UTF8ToString(s) : ""); })

BOOL PlaySoundA(LPCSTR name, HMODULE mod, DWORD flags)
{
    (void)mod; (void)flags;
    char line[160];
    snprintf(line, sizeof line, "[ski] PlaySoundA(%s, 0x%lx)",
             name ? name : "(stop)", (unsigned long)flags);
    console_c(line);
    return TRUE;
}

/* ---- INI (entpack.ini) ------------------------------------------------------ */
/* GetPrivateProfileStringA / WritePrivateProfileStringA — the high-score
 * table. Call sites (traced 2026-09-01): read (ski_core.c:2744)
 * GetPrivateProfileStringA("Ski", panel, g_c788, buf, 0x100,
 * "entpack.ini"); write (ski_core.c:2789) WritePrivateProfileStringA
 * ("Ski", panel, buf, "entpack.ini"); panel is one of the .data blobs
 * "SS"/"FS"/"GS" (g_c0d8/g_c0f4/g_c0f8, ski_core.c:1612-1615).
 *
 * Values are space-separated int lists ("%ld " per entry, 2788; up to
 * 10 entries). The reader (2746-2767) tokenizes on 0x20 and atoi's each
 * token, so the trailing space is a separator, not data — which is
 * exactly how wine 9.0 handles it (probed 2026-09-01, /tmp/t21fix):
 *   GetPrivateProfileStringA: a FOUND value comes back with leading AND
 *     trailing blanks (space and tab) stripped, interior blanks kept
 *     (write " 111 " -> get "111"; "222 333 " -> "222 333"). The DEFAULT
 *     is trimmed at the END only ("  DEFVAL  " -> "  DEFVAL", n=8).
 *   WritePrivateProfileStringA: the value's LEADING blanks are trimmed
 *     before writing (" 777 " -> line `SS=777 `), the trailing blank is
 *     kept in the file; a replace rewrites the line with the ORIGINAL
 *     key spelling from the file (write key "ss" over an existing "SS"
 *     line keeps `SS=` in the file); a pure replace never disturbs any
 *     other line.
 *
 * Serialization mirrors the reference exactly: the wine 9.0-written
 * profile (~/.wine-ski/drive_c/windows/entpack.ini) is
 *     [Ski]\r\nSS=-88328 \r\n
 * — `key=value\r\n` (no spaces around '=', value as above) and
 * `[section]\r\n`. A write rewrites only the target line, preserving
 * every other line byte-for-byte including its newline; a missing key
 * is inserted at the end of its section and a missing section is
 * appended at end of file.
 *
 * Known deviation (documented): wine 9.0's file writer has an
 * off-by-one — when a write ADDS a new line, the trailing space of the
 * first value line that has one is eaten (probed, /tmp/t21fix
 * keycase3-5.exe; pure replaces never eat). The game's tokenizer
 * (2746-2767) splits on 0x20 and skips leading blanks, so the eaten
 * space is functionally invisible to the game; the shim does not
 * reproduce the writer bug.
 *
 * g_c788 is the default: a zeroed .data buffer (ski_core.c:75) -> "" —
 * a missing key yields an empty string (empty table). Classic INI
 * semantics: [section] lines, key=value (value = rest of line after the
 * first '='), case-insensitive sections and keys, first occurrence
 * wins, default returned when missing.
 *
 * The file lives in localStorage key "entpack.ini"; the C side caches
 * the full text — loaded once from the shim's main (win.c) before
 * WinMain, re-saved on every Write*. The game has exactly one INI; the
 * fname argument is always "entpack.ini".
 *
 * The JS bridge uses the C-buffer pattern: EM_JS on emcc 6.0.6 does NOT
 * marshal strings — a JS string returned as 'const char*' arrives as
 * NULL and a 'const char*' argument arrives in JS as a raw pointer
 * NUMBER (T20-review repros, /tmp/t20review/emjs/t{,2}.c). Only ints
 * cross the bridge: C owns the bytes; JS copies via stringToUTF8 /
 * UTF8ToString. */
EM_JS(int, ini_js_load, (char *buf, int cap),
      {
        const s = localStorage.getItem('entpack.ini') || "";
        /* UTF-8 byte length without writing: lengthBytesUTF8 is NOT
         * in EM_JS scope on emcc 6.0.6 (T20-fix web probe) and
         * stringToUTF8(s, 0, 0) returns 0, not the needed length —
         * TextEncoder is exact for arbitrary UTF-8. */
        const len = new TextEncoder().encode(s).length;
        if (cap > 0)
          stringToUTF8(s, buf, cap);
        return len;
      })
EM_JS(void, ini_js_save, (const char *text),
      { localStorage.setItem('entpack.ini', text ? UTF8ToString(text) : ""); })

#define INI_CAP 8192
static char g_ini[INI_CAP];

void shim_ini_load(void)
{
    ini_js_load(g_ini, INI_CAP);
    g_ini[INI_CAP - 1] = '\0';
}

/* Locate `key` inside [section] in the cached text. Returns a pointer
 * into g_ini at the value start (after the '=') with *vlen set, or NULL. */
static const char *ini_find(const char *section, const char *key, size_t *vlen)
{
    const char *p = g_ini;
    int in = 0;
    while (*p) {
        const char *line = p;
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        const char *next = eol ? eol + 1 : p + len;
        while (len && (line[0] == ' ' || line[0] == '\t')) {
            line++;
            len--;
        }
        if (len == 0) {
            p = next;
            continue;
        }
        if (line[0] == '[') {
            /* line points at the '[': compare the name between the
             * brackets — comparing "[Ski" against "Ski" can never
             * match (T20 review). */
            size_t slen = strcspn(line + 1, "]");
            in = ci_eq(line + 1, slen, section);
            p = next;
            continue;
        }
        if (in) {
            const char *eq = memchr(line, '=', len);
            if (eq) {
                size_t klen = (size_t)(eq - line);
                while (klen && (line[klen - 1] == ' ' || line[klen - 1] == '\t'))
                    klen--;
                if (ci_eq(line, klen, key)) {
                    const char *v = eq + 1;
                    size_t vl = (size_t)((eol ? eol : p + len) - v);
                    if (vl && v[vl - 1] == '\r')
                        vl--;
                    *vlen = vl;
                    return v;
                }
            }
        }
        p = next;
    }
    return NULL;
}

static int ini_ws(int c)
{
    return c == ' ' || c == '\t';
}

int GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR def,
                             LPSTR buf, int size, LPCSTR fname)
{
    (void)fname;
    size_t vlen = 0;
    const char *v, *end;
    int found;
    if (!section || !key || !buf || size <= 0)
        return 0;
    v = ini_find(section, key, &vlen);
    found = v != NULL;
    if (!v) {
        v = def ? def : "";
        vlen = v ? strlen(v) : 0;
    }
    end = v + vlen;
    while (vlen && end[-1] == '\r')
        end--;
    if (found) {
        /* wine 9.0: a stored value is trimmed at BOTH ends, interior
         * blanks kept (T20-fix probe, see the INI note above). */
        while (v < end && ini_ws((unsigned char)*v))
            v++;
    }
    while (v < end && ini_ws((unsigned char)end[-1]))
        end--;
    size_t n = (size_t)(end - v);
    if (n > (size_t)size - 1)
        n = (size_t)size - 1;
    memcpy(buf, v, n);
    buf[n] = '\0';
    return (int)n;
}

/* Rebuild the file so `key` in [section] holds `val` (val == NULL
 * deletes the key — the game never passes NULL). Wine-exact
 * serialization (see the INI note): the rewritten line is
 * `key=value\r\n`; every other line is preserved byte-for-byte
 * INCLUDING its newline (the old pass copied up to eol exclusive and
 * dropped every newline it touched — T20 review). A missing key is
 * inserted at the end of its section; a missing section is appended at
 * end of file. */
int WritePrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR val,
                               LPCSTR fname)
{
    static char tmp[2 * INI_CAP];
    (void)fname;
    size_t n = 0;
    int in = 0, saw = 0, done = 0;
    const char *p = g_ini;
    if (!section || !key)
        return 0;
    /* wine 9.0 (T20-fix probe): leading blanks in the value are
     * trimmed before the line is written; a replace rewrites the line
     * with the ORIGINAL key spelling from the file, not the argument's. */
    const char *wval = val;
    while (wval && ini_ws((unsigned char)*wval))
        wval++;
#define PUT(s)                                                                     \
    do {                                                                           \
        const char *q = (s);                                                       \
        while (*q && n + 1 < sizeof tmp)                                           \
            tmp[n++] = *q++;                                                       \
    } while (0)
    /* copy the line verbatim, newline included (eol == NULL: final
     * unterminated line — copy to its end) */
#define PUT_LINE(l, l2)                                                            \
    do {                                                                           \
        const char *q = (l);                                                       \
        const char *stop = (eol) ? (eol) + 1 : (l) + (l2);                         \
        while (q < stop && n + 1 < sizeof tmp)                                     \
            tmp[n++] = *q++;                                                       \
    } while (0)
    /* the wine line shape: no spaces around '=', CRLF; (k, klen) are
     * the key bytes to emit — the original file spelling on a replace,
     * the argument for an insertion */
#define PUT_KEYLINE(k, klen)                                                       \
    do {                                                                           \
        size_t ki;                                                                 \
        for (ki = 0; ki < (size_t)(klen); ki++)                                    \
            if (n + 1 < sizeof tmp)                                                \
                tmp[n++] = (k)[ki];                                                \
        PUT("=");                                                                  \
        PUT(wval);                                                                 \
        PUT("\r\n");                                                               \
    } while (0)
    while (*p) {
        const char *line = p;
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        const char *next = eol ? eol + 1 : p + len;
        const char *trim = line;
        size_t tlen = len;
        while (tlen && (trim[0] == ' ' || trim[0] == '\t')) {
            trim++;
            tlen--;
        }
        if (trim[0] == '[' && tlen > 0) {
            /* trim points at the '[': compare the name between the
             * brackets (the old code compared "[Ski" against "Ski" —
             * T20 review). */
            size_t slen = strcspn(trim + 1, "]");
            int is_ours = ci_eq(trim + 1, slen, section);
            if (in && !done && val && !is_ours) {
                PUT_KEYLINE(key, (int)strlen(key)); /* end of our section:
                                  insert before the next header */
                done = 1;
            }
            in = is_ours;
            saw |= in;
            PUT_LINE(line, len);
            p = next;
            continue;
        }
        if (tlen > 0 && in && !done) {
            const char *eq = memchr(trim, '=', tlen);
            if (eq) {
                size_t klen = (size_t)(eq - trim);
                while (klen && (trim[klen - 1] == ' ' || trim[klen - 1] == '\t'))
                    klen--;
                if (ci_eq(trim, klen, key)) {
                    done = 1;
                    if (val) {
                        /* the ORIGINAL key spelling from the file —
                         * wine keeps it on a replace (T20-fix probe) */
                        PUT_KEYLINE(trim, (int)klen);
                    } /* val == NULL: the line is dropped */
                    p = next;
                    continue;
                }
            }
        }
        PUT_LINE(line, len);
        p = next;
    }
    if (in && !done && val) {
        PUT_KEYLINE(key, (int)strlen(key)); /* our section is last:
                                                 append at its end */
    } else if (!saw && val) {
        if (n && tmp[n - 1] != '\n')
            PUT("\r\n");
        PUT("[");
        PUT(section);
        PUT("]\r\n");
        PUT_KEYLINE(key, (int)strlen(key));
    }
    tmp[n] = '\0';
    snprintf(g_ini, sizeof g_ini, "%s", tmp);
    ini_js_save(g_ini);
    return 1;
}

/* Test hook (T20-review verification): exercise the INI cache + the
 * localStorage bridge without a full game run — the bridge is EM_JS,
 * so the host unit test (which stubs it) cannot see the real thing.
 * Fixed "Ski" section, game-shaped keys/values (ski_core.c:2724-2789);
 * ints only cross the JS boundary (no string marshaling on emcc 6.0.6):
 *   0: WritePrivateProfileStringA("Ski","SS","-88328 ","entpack.ini")
 *   1: WritePrivateProfileStringA("Ski","FS","12 34 ","entpack.ini")
 *   2: GetPrivateProfileStringA("Ski","SS","",...) into the result
 *      buffer, returns the length (wine trims the trailing space: 6)
 *   3: same for "FS" (interior space kept: "12 34", 5)
 * The result buffer is read back with UTF8ToString(ski_test_ini_result()). */
static char g_ini_probe[0x100];

EMSCRIPTEN_KEEPALIVE
int ski_test_ini(int op)
{
    switch (op) {
    case 0:
        return WritePrivateProfileStringA("Ski", "SS", "-88328 ",
                                          "entpack.ini");
    case 1:
        return WritePrivateProfileStringA("Ski", "FS", "12 34 ",
                                          "entpack.ini");
    case 2:
    case 3:
        return GetPrivateProfileStringA("Ski", op == 2 ? "SS" : "FS", "",
                                        g_ini_probe, sizeof g_ini_probe,
                                        "entpack.ini");
    }
    return -1;
}

EMSCRIPTEN_KEEPALIVE
const char *ski_test_ini_result(void) { return g_ini_probe; }

/* ---- MessageBoxA ----------------------------------------------------------- */
/* MessageBoxA — the three call sites (traced 2026-09-01):
 *
 *  1. ski_core.c:141 (assert box): (NULL, msg, "Assertion Failed",
 *     0x31 = MB_ICONHAND|MB_YESNO). The ONLY site that reads the return
 *     value: `if (r == 2) DestroyWindow(g_c6c8);` (142). 2 is the
 *     IDCANCEL literal (Win32 IDCANCEL == 2; the decompile comment says
 *     IDNO but the compared value is 2). r == 1 (IDOK) -> no destroy.
 *     The caller's epilogue after the box: ski_pause_toggle()
 *     (ski_assert_fail, ski_core.c:152).
 *  2. ski_core.c:158 (fatal box): (NULL, msg, STR_TITLE, 0x30 =
 *     MB_ICONERROR|MB_OK). Return value ignored; both callers'
 *     post-box code is `return 0` (ski_core.c:117, ski_win.c:506).
 *  3. ski_core.c:2812 (high-score box): (g_c6c8, buf, STR_HIGHSCORE,
 *     0). Return value ignored; the box is ski_score_show's last
 *     statement and every caller's post-box code is `return;`
 *     (ski_core.c:1638/1679/1715).
 *
 * Design (full write-up in the win.c modal note + shim/API.md): wine
 * 9.0 (the contract) keeps the main window's callback timer firing
 * AND its WM_PAINTs delivering while a modal box is up — the world
 * keeps ticking and painting behind it (T20-review controlled probe,
 * /tmp/t20review/modal_probe2.log: ticks 6..145 + paints 16..144 all
 * with box_up=1). The box cannot block in C (emscripten_sleep was
 * rejected empirically — see the win.c note), so the yield is
 * pump-level: shim_modal_raise (win.c) records the box + its site and
 * returns IDOK immediately, the rAF pump KEEPS DISPATCHING (timer +
 * paint; input is dropped — the dialog owns it) while the box is up,
 * and the harness closes it via the EMSCRIPTEN_KEEPALIVE hook
 * ski_messagebox_answer(r), which replays the one statement the
 * immediate return skipped (assert site: `if (r == 2)
 * DestroyWindow(g_c6c8)` — ski_core.c:142; score/fatal: none). The
 * game's own post-box code — the verified-faithful per-call-site
 * epilogues (ski_core.c:141-152: the assert site's
 * ski_pause_toggle(); the score/fatal sites' plain returns) — runs at
 * raise time, unmodified. A box raised outside the pump (boot-time
 * fatal path, before the rAF loop starts) returns IDOK synchronously
 * with no modal state. */
int MessageBoxA(HWND owner, LPCSTR text, LPCSTR caption, UINT type)
{
    char line[600];
    snprintf(line, sizeof line,
             "[ski] MessageBoxA owner=%s type=0x%02lx caption=\"%s\" text=\"%s\"",
             owner ? "main" : "NULL", (unsigned long)type,
             caption ? caption : "", text ? text : "");
    console_c(line);
    return shim_modal_raise(owner, text, caption, type);
}
