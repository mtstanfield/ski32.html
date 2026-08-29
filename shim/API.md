# Shim API — authoritative symbol list (Task 16)

Captured 2026-08-29: `emcc -O2 -I shim src/*.c <main wrapper>
-DSKI_DETERMINISTIC=1 -DSKI_HARNESS=0 -Wl,--error-limit=0` → wasm-ld
`undefined symbol` list, deduplicated. The shim (Tasks 17–20) implements
**exactly** this list (plus the `EMSCRIPTEN_KEEPALIVE` debug hooks, which
live in `canvas.c` and are not in this list).

Two symbols appear additionally when `SKI_HARNESS=1` (the T21 WASM
diff build): `CreateDIBSection` (ski_core.c:2476, tick-frame DIB) and
`GetProcessId` (ski_win.c:573, wproc trace line) — declared in
`win32.h`, implemented in `misc.c`.

## user32 — windows/messages (29)

| symbol | what it must do |
|---|---|
| `RegisterClassA` | store class by name → WndProc (both `SkiMain` and `SkiStatus` register; return nonzero atom) |
| `CreateWindowExA` | create a window record (dc sized w×h, class bg fill) + start the main loop once the main window is created |
| `FindWindowA` | look up a window by class name |
| `MoveWindow` | update x/y/w/h (the WM_SIZE handler resizes the offscreen DC) |
| `SetWindowPos` | same as MoveWindow per the flag bits (SWP_NOSIZE/NOMOVE/NOZORDER) |
| `SetWindowTextA` | update the title (cosmetic only) |
| `ShowWindow` | track visible/minimized state; SW_MINIMIZE drives the original's auto-pause (c770) — the message path must still deliver WM_SIZE-like semantics |
| `UpdateWindow` | no-op (painting is tick-driven) |
| `SetFocus` | no-op |
| `InvalidateRect` | no-op |
| `KillTimer` | disarm the game timer (pause path — c6d0=0) |
| `SetTimer` | arm the 40 ms game timer (id 0x29a, period c678&0xffff) |
| `BeginPaint` | fill PAINTSTRUCT: hdc = window dc, rcPaint = full client rect, fErase = 1 |
| `EndPaint` | flush the window framebuffer to the canvas |
| `GetDC` | return the window's DC |
| `ReleaseDC` | no-op, return 1 |
| `GetClientRect` | 0,0,w,h |
| `DefWindowProcA` | no-op, return 0 |
| `PostQuitMessage` | set the quit flag (loop exit) |
| `GetMessageA` | pop one queued message (timers/keys/clicks) into the MSG; 0 on empty+quit |
| `TranslateMessage` | no-op |
| `DispatchMessageA` | call the window's WndProc(msg, wp, lp) |
| `DestroyWindow` | mark window dead (WM_DESTROY path during quit) |
| `IsIconic` | return minimized state (0 or 1) |
| `LoadIconA` | return a non-NULL dummy |
| `LoadCursorA` | return a non-NULL dummy |
| `OpenIcon` | identity |
| `MessageBoxA` | high-score modal + "Ski Paused" box: console log + JS alert with OK |
| `LoadStringA` | id → exact UI string from the .data table (group 1 ids 1..15, group 2 16..17 — see NOTES "String table"; copy ≤ max-1 chars, return length) |
| `LoadBitmapA` | name/id → the pre-decoded sprite `HBITMAP` (see gdi32 below) |
| `wsprintfA` | `vsnprintf` — all formats used are snprintf-compatible (`%2u %2.2u %5.2d %7ld`) |

## gdi32 — surfaces/text (20)

| symbol | what it must do |
|---|---|
| `CreateCompatibleDC` | new offscreen DC (sprite strips + status) |
| `DeleteDC` | free the DC |
| `CreateCompatibleBitmap` | new w×h 32bpp bitmap (transparent/zero) |
| `CreateBitmap` | new w×h bitmap from raw bits (the 8bpp sprite strips: DIB bits via `CreateBitmap`? — the game builds the strips from `LoadBitmapA` resources; verify the caller in ski_core.c before implementing: `CreateBitmap` is used at ski_core.c bitmap-load for the raw resource bytes) |
| `SelectObject` | swap the DC's current object (bitmap); return the previous |
| `DeleteObject` | free bitmap |
| `GetObjectA` | fill `BITMAP` (bmWidth/bmHeight/bmBitsPixel/…) for a bitmap |
| `BitBlt` | copy src rect → dst rect with ROP (SRCCOPY = straight copy; BLACKNESS/WHITENESS = fill; CAPTUREBLT = ignore) |
| `PatBlt` | fill with ROP (PATINVERT = xor 0xFFFFFFFF; BLACKNESS/WHITENESS = fill) — verify the exact rop set against ski_core.c call sites |
| `GetStockObject` | WHITE_BRUSH/BLACK_BRUSH/NULL_BRUSH → brush handle (color only matters for FillRect/FrameRect) |
| `FillRect` | fill rect with the brush color |
| `FrameRect` | 1-px 3D edge (raised/etched per NOTES T12 usage — the status panel frame) |
| `TextOutA` | draw string at (x,y) with the captured pixel-exact font (Task 19), per-char advances |
| `GetTextExtentPoint32A` | width = sum of per-char advances, height = font tmHeight |
| `GetTextMetricsA` | fill TEXTMETRICS from the captured metrics |
| `GetDeviceCaps` | HORZRES=760, VERTRES=734 (client size — the game stores these in c6a0/c74c) |
| `CreateDIBSection` | **[harness only]** w×h 32bpp DIB + out-ptr (tick-frame dump) |

## kernel32 (10)

| symbol | what it must do |
|---|---|
| `GetTickCount` | virtual ms clock: advances with the rAF-driven tick schedule (NOT wall clock — determinism: the SKI_DETERMINISTIC rebuild already uses c698+40 per tick; the shim's clock must match: return the tick-derived value) |
| `LocalAlloc` | `calloc` (entity pool, string cache) |
| `FreeLibrary` | no-op, return 1 (called on the resource module handle) |
| `FindResourceA` | (inst, name, type) → resource handle for the 89 bitmaps (type RT_BITMAP) — resolve from the embedded sprite table by id |
| `LoadResource` | resource handle → global handle (identity into the sprite table) |
| `LockResource` | return the sprite's pixel base (or the decoded `ShimBmp*` as the HGLOBAL — see below) |
| `FreeResource` | no-op |
| `GetPrivateProfileStringA` | INI read (localStorage-backed string, classic INI semantics, case-insensitive) |
| `WritePrivateProfileStringA` | INI write (persist to localStorage on every call) |
| `lstrlenA` / `lstrcpyA` / `lstrcmpiA` | strlen / strcpy / strcasecmp (return int) |

## winmm (1)

| symbol | what it must do |
|---|---|
| `PlaySoundA` | M1 answer: no RT_WAVE in the PE — the named sounds come from the *system* (Wine's system sounds). The game's `nosound` gate (c794) + the rebuild's silent-by-construction behavior ⇒ implement as a no-op (console log). See NOTES M1 sound answer before adding real audio. |

## Sprite model (LoadBitmapA + the resource triplet)

The original loads the 89 sprites as RT_BITMAP resources (1/4/8bpp DIBs)
via FindResourceA/LoadResource/LockResource, then builds 8bpp
palette-based sprite strips. The shim must therefore model a
**palette-bearing bitmap**: `ShimBmp { w, h, bpp, pal[256×RGB], px[] }`
where px is indexed (1/4/8bpp) — the BitBlt path expands indices through
the palette. `harness/embed_sprites.py` (Task 20) decodes
`web/assets/sprites/bmp_NNN.png` (Pillow) into static RGB/palette arrays
+ a table keyed by resource id; `LoadBitmapA`/`FindResourceA` resolve
against it. The palette is part of the DIB (verified in T6 extraction —
`resources.json` carries bitcount + palette).

## Debug hooks (Task 18/21, NOT in the symbol list)

`EMSCRIPTEN_KEEPALIVE`: `ski_key_event(vk, down)`, `ski_click(x, y)`,
`ski_set_input(bytes, n)` (per-tick input word, same 2-byte layout as
`harness/gen_input.py`), `ski_tick_get()`, `ski_window_png(n)` (PNG
dataURL of window n's framebuffer), `window.__ski = createSki()` boot.

## Build notes

- Entry: emscripten needs `main`; the shim provides a wrapper
  `int main(void) { return WinMain(0, 0, 0, 1); }` (WinMain's HINSTANCE
  args are unused by the game except for the NULL-hPrev re-register
  guard — pass a non-NULL dummy for hInstance so the class registration
  runs, and hPrev = NULL).
- `-sENVIRONMENT=web -sMODULARIZE=1 -sEXPORT_NAME=createSki`
  (already in CMakeLists).
- `#include <windows.h>` resolves to `shim/windows.h` via `-I shim`
  (the game sources are unmodified).
