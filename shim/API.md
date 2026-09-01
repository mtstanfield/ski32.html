# Shim API — authoritative symbol list (Task 16)

Re-probed 2026-08-31 (emcc 6.0.6, after the 16 M2 commits that touched `src/`
since the 2026-08-29 probe; the symbol set is **unchanged**). The shim
(Tasks 17–20) implements **exactly** this list — nothing more, nothing less
(plus the `EMSCRIPTEN_KEEPALIVE` debug hooks, which live in `canvas.c` and are
not in this list).

## Probe (reproducible)

```sh
source ~/.emsdk/emsdk_env.sh          # Emscripten 6.0.6
mkdir -p /tmp/skiprobe && cd /tmp/skiprobe
# main_wrap.c (probe-only, not in the repo):
#   #include <windows.h>
#   int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#   int main(void) { return WinMain((HINSTANCE)1, 0, 0, 1); }

# Core build — 60 undefined symbols:
emcc -O2 -I <repo>/shim -DSKI_DETERMINISTIC=1 -DSKI_HARNESS=0 \
     -Werror=implicit-function-declaration \
     <repo>/src/*.c main_wrap.c -o ski.js -Wl,--error-limit=0 2>&1 | tee emcc-errors.txt

# Harness build — 62 = core 60 + CreateDIBSection + GetProcessId:
emcc -O2 -I <repo>/shim -DSKI_DETERMINISTIC=1 -DSKI_HARNESS=1 \
     -Werror=implicit-function-declaration -include stdlib.h \
     -DVK_LEFT=0x25 -DVK_RIGHT=0x27 -DVK_UP=0x26 -DVK_DOWN=0x28 \
     -DVK_NUMPAD0=0x60 -DVK_NUMPAD1=0x61 -DVK_NUMPAD3=0x63 \
     -DVK_NUMPAD7=0x67 -DVK_NUMPAD9=0x69 -DVK_F2=0x71 -DVK_F3=0x72 \
     -DVK_RETURN=0x0d -DBI_RGB=0 \
     <repo>/src/*.c main_wrap.c -o ski_h.js -Wl,--error-limit=0 2>&1 | tee emcc-errors-harness.txt
```

- `main_wrap.c` supplies `main` (the game's entry is `WinMain`); `-I shim`
  resolves the game's `#include <windows.h>` to `shim/windows.h` so the
  unmodified sources compile.
- `-Werror=implicit-function-declaration` proves every Win32 call in `src/`
  is declared in `shim/win32.h`, so the linker's undefined list is complete:
  no symbol can hide behind an implicit declaration.
- The harness build's `VK_*`/`BI_RGB`/`<stdlib.h>` defines are probe-side
  stand-ins for what the shim must provide (see Build notes, last item).

## Cross-check against the original PE

`ski32.exe`'s import directory (parsed from `original/ski32.exe`; matches the
IAT documented in `harness/stub/orig_stub.asm`) has **96 imports in 4 DLLs**:
KERNEL32 48, USER32 33, GDI32 14, WINMM 1.

- All 60 core probe symbols appear in that IAT **except** `PlaySoundA`:
  the original imports the legacy WINMM alias `sndPlaySoundA` (same function);
  the rebuild calls the standard name `PlaySoundA`, so the shim exports that.
- The remaining IAT entries (CRT glue: `Heap*`, `MultiByteToWideChar`,
  `LCMapStringA`, `GetEnvironmentStrings*`, `RtlUnwind`, …) are satisfied by
  emscripten's system libraries — they do not appear in the probe.
- Grouping below follows the original IAT's DLLs.

## user32 — windows/messages (33)

| symbol | what it must do |
|---|---|
| `RegisterClassA` | store class by name → WndProc + bg brush; both `SkiMain` (style 0x2023) and `SkiStatus` register (ski_win.c:807–826); called only when hPrev==NULL; return nonzero atom |
| `CreateWindowExA` | create a window record (client dc w×h); main: style 0x2cf0000, w = min(HORZRES,VERTRES), h = VERTRES, x = (HORZRES−w)/2 (ski_win.c:834); status: WS_CHILD 0x4000000, 0×0 (ski_win.c:838). The rAF main loop is NOT started here — see `ski_start_pump` (Debug hooks): `emscripten_set_main_loop(..., simulate_infinite_loop=1)` inside main() throws 'unwind' and abandons the rest of boot (T20) |
| `FindWindowA` | look up a window by class name; the single-instance guard probes "SkiMain" (ski_win.c:785) — return NULL on the web (fresh boot) |
| `MoveWindow` | update x/y/w/h; the WM_SIZE handler resizes the offscreen canvas |
| `SetWindowPos` | MoveWindow semantics per flag bits; only used with 0x3 = SWP_NOSIZE\|SWP_NOMOVE (ski_win.c:787) |
| `SetWindowTextA` | update the title (cosmetic); the pause state sets it to STR_PAUSED id 2 (ski_win.c:167) |
| `ShowWindow` | track visible/minimized state; SW_MINIMIZE (6, ski_win.c:264) drives the auto-pause (c770); initial show at ski_win.c:842/844 |
| `UpdateWindow` | no-op (painting is tick-driven) |
| `SetFocus` | no-op |
| `InvalidateRect` | no-op (full repaint every tick) |
| `KillTimer` | disarm the 40 ms game timer (pause path — c6d0=0) |
| `SetTimer` | arm the 40 ms callback timer (id 0x29a, period c678&0xffff, proc c940 — see skidef.h) |
| `BeginPaint` | fill PAINTSTRUCT: hdc = window dc, rcPaint = full client rect, fErase = 1 |
| `EndPaint` | flush the window framebuffer to the canvas |
| `GetDC` | return the window's DC; `GetDC(NULL)` = the *screen* DC (ski_win.c:769) |
| `ReleaseDC` | no-op, return 1 |
| `GetClientRect` | {0,0,w,h} |
| `DefWindowProcA` | no-op, return 0 (fall-through for undriven messages) |
| `PostQuitMessage` | set the quit flag (loop exit) |
| `GetMessageA` | always returns 0 (WM_QUIT): WinMain's loop exits immediately and the rAF pump (`ski_mainloop`, started by `ski_start_pump`) does ALL dispatching — timers, paints, injected input (T18 design, boot-verified T20) |
| `TranslateMessage` | no-op |
| `DispatchMessageA` | call the window's WndProc(msg, wp, lp) |
| `DestroyWindow` | mark window dead (WM_DESTROY path; also triggered by IDNO on the assert box, ski_core.c:143) |
| `IsIconic` | return minimized state (0 or 1) — ski_win.c:788 |
| `LoadIconA` | non-NULL dummy; class icon "iconSki" (ski_win.c:813) |
| `LoadCursorA` | non-NULL dummy; IDC_ARROW (0x7f00) for both classes (ski_win.c:814, 821) |
| `OpenIcon` | identity restore (single-instance guard, ski_win.c:789) |
| `MessageBoxA` | **Deterministic modal in the pump** (T20). Three call sites: assert box 0x31 MB_ICONHAND\|MB_YESNO (the only site that reads the return: `if (r == 2) DestroyWindow(g_c6c8)` — ski_core.c:141-143; 2 is the IDCANCEL literal the original compares, the IDNO macro is never used), fatal box 0x30 MB_ICONERROR\|MB_OK (ski_core.c:158, return ignored), high-score modal type 0, owner = main window (ski_core.c:2812, return ignored). Design: the pump is single-threaded (rAF) so the box cannot block in C. When the game raises a box, `shim_modal_raise` (win.c) records the box (type/text/caption/site) + logs it to the console, then longjmps to `ski_mainloop`'s setjmp (armed once per rAF frame; the buffer is only targeted while live — a `g_in_pump` guard). The pump then SUSPENDS the game — no ticks, no messages, no paint — exactly matching the M2 s08 evidence (original and rebuild both 0px while a box is up). The harness answers via `ski_messagebox_answer(r)` (1 = IDOK, 2 = IDCANCEL); on the next pump frame the site epilogue runs (assert: `if (r == 2) DestroyWindow(g_c6c8); ski_pause_toggle()` — fatal/score: none) and the game resumes. The one-frame deferral of the epilogue (vs synchronous Win32) is invisible: the box blocks every tick and the epilogue touches only window/pause state. A box raised outside the pump (boot-time paths, before the rAF loop starts) falls through and returns IDOK synchronously — the callers' epilogues are then plain returns. JS visibility: `ski_messagebox_get/type/text/caption` (win.c KEEPALIVE hooks) |
| `LoadStringA` | id → exact UI string from the .data table (ids 1..17, skidef.h; NOTES "String table"); copy ≤ max−1 chars, return length. The table is **1-based with a dummy "" at [0]** (id 1 = "SkiFree", id 3 = "Time:" — matches the M1-extracted resources.json group 1 and the T19 pixel-verified reference); id 0 and out-of-range fail (return 0). The group-2 strings (" <-- that's you!" / " <-- try again!") are NOT reachable via LoadStringA — the game reads them through ski_str_cache from its own .data table (skidef.h STR_SUFFIX_*), and the FindResourceA path is WAVE-only (below) |
| `LoadBitmapA` | id → the pre-decoded sprite `HBITMAP` (ids 1..0x59; see Sprite model) |
| `wsprintfA` | `vsnprintf` — every format used in src/ is snprintf-compatible (`%s line %u`, `%5.2dm`, `%5.2dm/s`, `%7ld`, `%9ld`, `%2u:%2.2u:%2.2u.%2.2u`, `frame_%06u_main.ppm`, `P6\n%d %d\n255\n` — traced T20; no format needs native handling) |
| `FillRect` | fill rect with the brush color; the single call (ski_win.c:570, main paint) uses g_c69c = GetStockObject(0) = the wine WHITE brush (stock 0 is non-NULL under the reference's wine 9.0) → fills the main client white; a NULL brush must always no-op |
| `FrameRect` | 1-px edge with the brush; the single call (ski_win.c:729-730) uses GetStockObject(4) = the wine BLACK brush → draws the 1-px black ring around the 123×52 status panel that IS visible in the reference (evidence/m0-original-gameplay.png); a NULL brush must always no-op |

## gdi32 — surfaces/text (14)

| symbol | what it must do |
|---|---|
| `CreateCompatibleDC` | new offscreen DC (sprite strips, canvas, status) |
| `DeleteDC` | free the DC |
| `CreateCompatibleBitmap` | new w×h bitmap at the reference DC's depth (window DC → 32bpp in the shim) |
| `CreateBitmap` | new w×h bitmap from (w, h, planes, bpp, bits); called **only** with bpp=1, bits=NULL — the 1bpp mask strips (ski_core.c:275, 296) |
| `SelectObject` | swap the DC's current object (bitmap); return the previous |
| `DeleteObject` | free bitmap |
| `GetObjectA` | fill the 24-byte `BITMAP` prefix (bmType..bmBitsPixel — width/height/planes/bitcount); called with cnt=0x18 (sprite-load sizing pass) |
| `BitBlt` | copy src rect → dst rect with ROP; the exact ROP set in use is below — must reproduce Win32 GDI's palette/index → color and 32bpp/1bpp conversion semantics |
| `PatBlt` | ROP fill; **only** WHITENESS 0xFF0062 is used (canvas/window clears fill white, ski_core.c:2892/2974/3001) |
| `GetStockObject` | the indices used are 0, 4, 10 (ski_win.c:775/729/686, transcribed verbatim from the original — FUN_004052d0.c:36, FUN_00406970.c:17, FUN_00406a70.c:21). The reference runs under wine 9.0 (WINEPREFIX=~/.wine-ski), whose stock table is NOT real Win32's (real: 0/4 NULL, 10 NULL_PEN). Measured wine behavior: 0 → WHITE brush (the class hbrBackground — windows erase white), 4 → BLACK brush (the FrameRect at ski_win.c:730 draws the 1-px black status-panel ring visible in the M2 reference), 10 → a 12px MONOSPACE TTF (empty face name, lfHeight 12, weight 400, tmHeight 12, tmAscent 9, tmDescent 3, every ASCII advance 7, tmMaxCharWidth 13) — the game's text font. wproc_status_create SelectObject's it into the status DC (686-688), making it the DC's current font for every TextOutA/extent/metrics call, so return the Task-19 captured-font handle for 10 (glyphs + advances baked in shim/font.inc; see shim/text.c) and white/black brush handles for 0/4 |
| `GetTextExtentPoint32A` | width = sum of per-char advances, height = font tmHeight (pixel-exact font capture, Task 19) |
| `GetTextMetricsA` | fill TEXTMETRICS from the captured metrics |
| `GetDeviceCaps` | screen-DC caps (hdc from `GetDC(NULL)`): index 8 = HORZRES → c6a0, index 10 = VERTRES → c74c (ski_win.c:770–771); the reference run used a 1024×768 screen (harness Xvfb) → window outer 768×768, client 760×734 (evidence/m0-geometry.txt) — the shim's "screen" must match for parity |
| `TextOutA` | draw string at (x,y) with the captured pixel-exact font (Task 19), per-char advances |

### ROP set (verified against `decompile/ghidra/FUN_00401540.c` + `FUN_00405ab0.c`)

| ROP | value | call sites |
|---|---|---|
| SRCCOPY | 0xCC0020 | ski_core.c:331 (sprite→image strip), 2977 (group canvas, first draw), 2997 (group canvas→window composite), 2565 (harness tick dump) |
| NOTSRCCOPY (a.k.a. MASKPEN) | 0x330008 | ski_core.c:332 (sprite→1bpp mask strip) |
| SRCAND | 0x8800C6 | ski_core.c:2901 (group canvas OOM fallback), 2982 (group canvas) |
| SRCPAINT | 0xEE0086 | ski_core.c:2980 (group canvas mask blit) |
| WHITENESS | 0xFF0062 | PatBlt only (above) |

The inline comments at ski_core.c:2901/2977/2980/2982/2997 misnamed these
ROPs ("SRCCOPY" over 0x8800C6, "SRCPAINT" over 0xCC0020, "MERGECOPY" over
0xEE0086), and the PatBlt clears at 2892/2975/3002 said "BLACKNESS" —
0xFF0062 is WHITENESS (the real BLACKNESS is 0x000042, unused by the game):
the canvas/window clears fill WHITE. All comments now use the SDK names
(mingw wingdi.h). The **values** are transcribed from the decompilation
(decompile/ghidra/FUN_00401540.c:77,171,175,177,187) and are
authoritative. No CAPTUREBLT or PATINVERT call exists in the rebuild.

## kernel32 (12)

| symbol | what it must do |
|---|---|
| `GetTickCount` | virtual ms clock: must advance on the tick schedule, **not** wall clock (determinism: the SKI_DETERMINISTIC build derives now = c698+40 per tick, ski_core.c:2623; ski_win.c:180) |
| `LocalAlloc` | `calloc` (entity pool, string cache) |
| `FreeLibrary` | no-op, return 1; the sound-module handle g_c78c is always 0 (static import, ski_game.h:83) |
| `FindResourceA` | called **only** with type "WAVE" (9 sound ids, ski_core.c:174); must return NULL — the PE has no WAVE resource node, so the game is silent by construction |
| `LoadResource` | reachable only if `FindResourceA` returns non-NULL → unreachable; stub that returns its input (ski_core.c:177) |
| `LockResource` | unreachable for the same reason (ski_core.c:179) |
| `FreeResource` | unreachable (ski_sound_free path, ski_core.c:192); no-op |
| `GetPrivateProfileStringA` | INI read: "entpack.ini", section "Ski", 10-entry high-score panels ("SS"/"FS"/"GS"), 0x100 buffer (ski_core.c:2744). Backed by localStorage key **"entpack.ini"** via an EM_JS bridge (misc.c); the C side caches the full text (loaded once from the shim's main before WinMain). Classic INI semantics: `[section]` lines, `key=value`, case-insensitive sections+keys, `lpDefault` returned when missing, values stored/read verbatim (the reader tokenizes on 0x20 + atoi, so no trimming) |
| `WritePrivateProfileStringA` | INI write, same file/section (ski_core.c:2789): single-pass cache rebuild (replaces existing key, appends at end if new, deletes on NULL value, creates `[section]` if absent) and persists to localStorage on every call |
| `lstrlenA` | strlen |
| `lstrcpyA` | strcpy |
| `lstrcmpiA` | case-insensitive strcmp; the WinMain "nosound" gate (ski_win.c:858) |

## winmm (1)

| symbol | what it must do |
|---|---|
| `PlaySoundA` | no-op (console log). Must be a real linkable function: `ski_sound_init` takes its **address** into g_c790 (ski_core.c:165) and later calls it through the pointer — stop (NULL,NULL,0) at ski_core.c:203, play (ptr,NULL,0x8000) at ski_core.c:414 (unreachable: no WAVE resource exists). The original imports the WINMM alias `sndPlaySoundA`; the rebuild calls `PlaySoundA` — the shim exports the standard name |

## Harness-only additions (SKI_HARNESS=1, T21 WASM diff build) — +2

| symbol | DLL | what it must do |
|---|---|---|
| `CreateDIBSection` | gdi32 | w×h 32bpp DIB + out-ptr (tick-frame dump, ski_core.c:2547); NOT in the original IAT (the stub resolved it via GetProcAddress) |
| `GetProcessId` | kernel32 | return any fixed pid (wproc trace header line, ski_win.c:584) |

## Sprite model (verified against `decompile/ghidra/FUN_00405ab0.c` + `ski_load_bitmaps`)

`game_sprites_load` (0x405ab0) loads the 89 sprites via `LoadBitmapA`
(ids 1..0x59; each a 1/4/8bpp DIB — `resources.json` from the T6 extraction
carries bitcount + palette). It then builds two strip families:

- **Image strips** — `CreateCompatibleBitmap` vs the window DC (32bpp in the
  shim): small strip 0x20×h (c710, sprites with w < 0x21), big strip
  max_w×h (c730).
- **Mask strips** — `CreateBitmap(w, h, 1, 1, NULL)` (1bpp): small (c6a4),
  big (c6ec).
- **Canvas** (c5ec) — `CreateCompatibleBitmap`, aligned
  ((max_w & 0xffc0)+0x40) × ((max_h & 0xffc0)+0x40).

Each sprite is blitted into its image strip with SRCCOPY (0xCC0020) and into
its mask strip with NOTSRCCOPY (0x330008, a.k.a. MASKPEN). The shim's `BitBlt` must therefore
reproduce Win32 GDI's DIB expansion exactly: palette-index → RGB for the
32bpp image strips, and DIB → 1bpp for the mask strips under NOTSRCCOPY.
M2's 0-px frame parity against the original (real GDI on both sides) is the
acceptance gate for these conversions.

### 4bpp sprite storage + the 4→1 mask rule (T20)

The PE's 89 sprite resources are all 4bpp indexed DIBs (resources.json:
bpp=4 ×89, no 1/8/24bpp present). The T20 wine-9.0 probe
(/tmp/t20probe, real 4bpp RT_BITMAP resources via `gen_dibs.py` + LoadBitmapA,
2026-09-01) established:

- **wine converts loaded 4bpp resource DIBs to 32bpp device bitmaps**:
  `GetObjectA` on the loaded resource reports bpp=32 (and a 4bpp GetDIBits
  readback fails with error 87). So storing the sprites pre-expanded as
  32bpp RGBA (`shim_bmp_from_rgb`, surface.c; rows from the T6-extracted
  PNGs, whose palette-corrected RGB *is* the resource pixels) is exactly
  what the reference GDI had. `harness/embed_sprites.py` emits
  `shim/sprites.inc` (89 entries: id, w, h, RGB rows) and `LoadBitmapA`
  builds a 32bpp ShimBmp from it.
- **The NOTSRCCOPY 4bpp→1bpp mask rule: bit set iff the palette-EXPANDED
  color is not 0xFFFFFF** — the same rule as the measured 32→1 case (T17:
  [white,red,black,blue] → bits 0 1 1 1). Discriminated by palette shapes:
  pal [red,white,green,black] px idx [0,1,2,3] → bits **1 0 1 1** (rejects
  index-based rules) and pal [white,white,red,black] → bits **0 0 1 1**
  (rejects index-of-white). The index is never compared; the expanded RGB
  is. surface.c's 32→1 NOTSRCCOPY (px != WHITE) applies unchanged to the
  expanded sprite pixels, so the game's mask-strip pass (ski_core.c:332)
  needs no 4bpp-specific code.

## Debug hooks (Task 18/20/21, NOT in the symbol list)

`EMSCRIPTEN_KEEPALIVE`: `ski_key_event(vk, down)`, `ski_click(x, y)`,
`ski_set_input(bytes, n)` (per-tick input word, same 2-byte layout as
`harness/gen_input.py`), `ski_tick_get()`, `ski_window_png(n)` (PNG
dataURL of window n's framebuffer), `ski_start_pump()` (start the rAF
pump — web/boot.js calls it once after createSki() resolves; it MUST NOT
be called from inside the C main stack, see CreateWindowExA row), and the
MessageBoxA modal hooks `ski_messagebox_get()`, `ski_messagebox_type()`,
`ski_messagebox_text()`, `ski_messagebox_caption()`,
`ski_messagebox_answer(r)` (1 = IDOK, 2 = IDCANCEL). Boot surface:
`window.__ski = createSki()` (MODULARIZE, EXPORT_NAME=createSki); the
module resolves after main() (WinMain) returns.

## Build notes

- Entry: emscripten needs `main`; the shim provides a wrapper
  `int main(void) { return WinMain(hInst, NULL, cmd, 1); }` with a
  non-NULL dummy hInst — class registration is gated on hPrev == NULL
  (ski_win.c:807), so pass hPrev = NULL to make it run.
- `#include <windows.h>` resolves to `shim/windows.h` via `-I shim`
  (the game sources are unmodified).
- `-sENVIRONMENT=web -sMODULARIZE=1 -sEXPORT_NAME=createSki`
  (already in CMakeLists), plus `-sEXPORT_KEEPALIVE=1` and
  `-sEXPORTED_RUNTIME_METHODS=HEAPU8` (canvas.c's blit/png EM_JS reads
  `Module.HEAPU8` — without the export the first paint aborts with
  "HEAPU8 was not exported", T20).
- Boot: web/index.html (canvas 1024×768) + web/boot.js
  (`createSki().then(m => { window.__ski = m; m._ski_start_pump(); })`).
  Verified T20 in headless Chrome: module loads with no console errors
  (the sole log is the faithful `PlaySoundA((stop), 0x0)` from
  ski_cleanup), `ski_tick_get()` advances (40 ms game timer), and the
  title screen (s01) renders at the reference (132,30) geometry.
- **Harness build prerequisites (verified by the 2026-08-31 harness probe;
  currently missing from the shim, so the probe supplied them from
  the command line):**
  1. `VK_*` macros (12): LEFT 0x25, RIGHT 0x27, UP 0x26, DOWN 0x28,
     NUMPAD0 0x60, NUMPAD1 0x61, NUMPAD3 0x63, NUMPAD7 0x67, NUMPAD9 0x69,
     F2 0x71, F3 0x72, RETURN 0x0d — consumed via `ski_keys.h`
     (ski_core.c:2474, table at 2484).
  2. `BI_RGB` (0) — `BITMAPINFOHEADER.bmiCompression` in the CreateDIBSection
     path (ski_core.c:2547).
  3. `<stdlib.h>` — the harness-only ski_win.c:89/93/243 calls `getenv`/
     `strtoul`; `shim/windows.h` (or a shim include) must provide it.
