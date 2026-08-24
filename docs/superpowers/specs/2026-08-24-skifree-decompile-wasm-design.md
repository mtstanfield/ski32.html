# SkiFree 32-bit: Full Decompilation + Perfect WASM Port

Date: 2026-08-24
Status: approved design (approach A)

## Goal

1. Produce a **full, annotated decompilation** of `ski32.exe` — SkiFree 1.04 32-bit
   (the 2005 build Chris Pirih compiled from his 1993-era source) — as a
   readable C source tree with a complete function map and reconstructed data
   model. This is a deliverable in its own right.
2. Use that decompilation to build a **pixel-perfect, behavior-perfect WASM
   port** playable in a desktop browser.

## Decisions (from brainstorming)

- **Rights**: no contact with the author; project stays **private** (personal
  use, never hosted publicly). Decompile the EXE only; no source is available
  to us (the author holds the 1.03 source, `ski2.c`, but it is unpublished).
- **Fidelity bar**: **max fidelity** — identical game logic, physics, RNG
  sequences, frame timing, sprites, animations, UI layout, sound, and high
  scores — verified by **frame-by-frame pixel diff** against the original
  running in Wine, with a frozen RNG seed and scripted inputs.
- **Approach**: **A — recompile the decompilation.** Reconstructed C compiles
  unmodified for both mingw-w64 (native, Wine) and emscripten (WASM). The only
  new code is the Win32/GDI platform shim. Clean-room TS rewrite (B) and
  run-the-PE-as-is (C, the exebrowser.com route) were rejected.

## Reference binary facts (verified 2026-08-24)

- `ski32.exe`: PE32 GUI, Intel 386, 118,784 bytes, built 2005-10-02,
  MSVC-era toolchain (Rich header; MSVC6-era codegen expected).
- Sections: `.text` 0x86ac (34.5 KB machine code), `.rdata` 0x10f0,
  `.data` 0xcdc, `.rsrc` 0xf8c0. Entry point 0x406d83. Relocations/symbols
  stripped.
- Assert path in `.rdata`: `V:\hack\ski32\ski2.c` → the game logic is
  (almost certainly) a single C source file; the rest of `.text` is MSVC CRT.
- **Imports** (app-level):
  - USER32: RegisterClassA, CreateWindowExA, GetMessageA, TranslateMessage,
    DispatchMessageA, PostQuitMessage, ShowWindow, UpdateWindow, SetTimer,
    KillTimer, BeginPaint, EndPaint, GetDC, ReleaseDC, GetClientRect,
    MoveWindow, SetWindowPos, InvalidateRect, SetWindowTextA, SetFocus,
    FindWindowA, IsIconic, LoadCursorA, LoadIconA, OpenIcon, FrameRect,
    FillRect, MessageBoxA, LoadStringA, wsprintfA, DestroyWindow,
    DefWindowProcA.
  - GDI32: BitBlt, PatBlt, CreateCompatibleBitmap, CreateCompatibleDC,
    CreateBitmap, SelectObject, DeleteObject, DeleteDC, GetObjectA,
    GetStockObject, TextOutA, GetTextExtentPoint32A, GetTextMetricsA,
    GetDeviceCaps.
  - WINMM: sndPlaySoundA.
  - KERNEL32 (app-level): GetPrivateProfileStringA,
    WritePrivateProfileStringA, GetTickCount, GetModuleFileNameA, GetVersion.
- **Resources**: 89 RT_BITMAP sprites (IDs 1–89; 8bpp or lower, GDI bitmaps
  with BITMAPINFOHEADER + palette), 6 RT_ICON, 2 RT_STRING groups (UTF-16),
  2 RT_GROUP_ICON (`ICONSKI`, `ICONSKI2`). No dialogs, no menus, no sound
  resources.
- **String table** (UTF-16): `SkiFree` (title), `Ski Paused ... Press F3 to
  continue`, labels `Time:`, `Dist:`, `Speed:`, `Style:`, format strings
  (`%2u:%2.2u:%2.2u.%2.2u`, `%5.2dm`, `%5.2dm/s`, `%7ld`, `00:00:00.00`,
  ` 0000m`, ` 0000m/s`, `0000000`), `High Scores`, and high-score list
  markers ` <-- that's you!` / ` <-- try again!`.
- **INI**: `entpack.ini` (the 1991 Entertainment Pack's shared INI) — high
  scores and/or settings persist here.
- **Window classes**: `SkiMain` (game), `SkiStatus` (the floating
  score/status window), `button` (role unknown — M1 answers).
- **Sound mystery**: `sndPlaySoundA` + strings `WAVE`, `nosound`, but no
  sound resources → M1 determines what is actually played (system sounds?
  disabled by default?).

## Architecture

One source of truth: reconstructed C in `src/`, consumed by three runtimes —
the annotated decompilation (reference), the native rebuild (mingw-w64, runs
in Wine), and the WASM port (emcc + shim). Game logic is byte-identical
across runtimes; only the platform layer differs.

```
skifree-wasm/
├── original/     ski32.zip, ski32.exe (reference, checked in)
├── decompile/    Ghidra output, annotated C + NOTES.md: function map
│                 (name, address range, role), reconstructed structs,
│                 resource/API/data-model notes — the decompilation deliverable
├── src/          reconstructed compilable C; every file cites the Ghidra
│                 address range it was reconstructed from
├── shim/         Win32/GDI/WinMM/INI emulation — linked into emscripten build only
├── harness/      seed-freeze patcher, input scripts, frame capture,
│                 content-hash aligner, pixel differ, reports
├── web/          index.html, boot.js, captured bitmap font, audio assets;
│                 build output (ski.js/wasm) lands here; static-hosted app
└── CMakeLists.txt  two builds: native (mingw-w64) and web (emcmake)
```

Build requirements: emsdk (emcc), CMake, mingw-w64 (i686) cross toolchain,
Python 3, Wine 9.0 (Ubuntu 24.04 universe; 32-bit support via `wine32:i386`
multiarch or the new WoW64 — decided at setup), Java (Ghidra headless),
Ghidra (fetched at setup).

## Pipeline

### M1 — Decompile (Ghidra headless)

- Run Ghidra headless analysis on `ski32.exe`; export decompiled C for every
  function in `.text` outside the MSVC CRT (CRT identified by symbol-free
  signature matching / address ranges; it is not ported, only skipped and
  documented).
- Annotate from binary evidence: rename functions and variables using
  imports, the 89 bitmap IDs, the UTF-16 string table, INI keys, window
  classes, and data flow; reconstruct the key structures (game state,
  object lists, window records); write the function map and data-model notes
  into `decompile/NOTES.md`.
- M1 must answer the three port-critical questions:
  1. **RNG**: algorithm and seed source (time-seeded vs fixed).
  2. **Timing model**: fixed-step per timer tick vs `GetTickCount` dt-based;
     the timer period (SetTimer interval).
  3. **Sound**: exact `sndPlaySoundA` call(s) — what plays, when, and how
     `nosound`/`WAVE` factor in.
- Exit criterion: every non-CRT function named, categorized (init / message
  loop / game update / render / UI / score / audio), and cross-referenced to
  its imports and resources; the three answers written down with evidence.

### M2 — Rebuild native + prove behavioral equality

- Reconstruct `src/` from `decompile/` until it compiles cleanly with
  mingw-w64 → `ski-rebuild.exe`. Reconstructed code keeps the original's
  control flow and arithmetic; only decompiler artifacts (wrong types,
  redundant loads, opaque stack slots) are fixed.
- Compile native with `-mfpmath=387` to match MSVC6 x87 floating point.
- Run `ski32.exe` and `ski-rebuild.exe` side by side in Wine (xvfb) under the
  verification harness:
  - **Seed freeze**: locate the RNG seed write in the original; hex-patch it
    to a constant (patch lives in `harness/`, applied to a copy); the same
    constant is used in `src/` under a `DETERMINISTIC` build flag, active in
    both the native and web builds (M3's WASM-vs-rebuild diff needs it too).
  - **Scripted inputs**: JSON key-event scripts (tick, VK code, down/up)
    replayed into both.
  - **Frame alignment + diff**: capture frames per tick from the rebuild
    (in-process framebuffer dump); capture the original via X11 at high rate,
    dedupe by content hash, align by content (robust to timer jitter when
    physics is fixed-step — confirmed by M1's timing answer), pixel-diff
    every aligned frame.
- Target: **100% pixel match** across a suite of scripts covering: menu,
  all three game modes, mode switches, collisions, jumps/stunts, the Abominable
  Snow Monster chase, game over, high-score screen, pause (F3), and the
  `SkiStatus` window layout.
- Exit criterion: zero-pixel-diff reports for the full script suite.

### M3 — WASM port

- Write `shim/` implementing exactly the imported API surface over
  emscripten:
  - **Windows**: `SkiMain` + `SkiStatus` (+ `button` windows if M1 says so),
    drawn at their original positions on one canvas; move/position events
    honored as in the original.
  - **GDI**: 32-bit RGBA framebuffer per window; BitBlt (SRCCOPY + any other
    rop the code uses), PatBlt, CreateBitmap/CreateCompatibleBitmap with
    exact 8bpp→RGB palette expansion, TextOut/GetTextExtentPoint32A/
    GetTextMetricsA via the captured bitmap font (below), BeginPaint/EndPaint
    → double-buffered canvas blit.
  - **Timers**: `SetTimer`/`KillTimer` → rAF-backed tick scheduler emitting
    ticks at the game's period.
  - **INI**: `entpack.ini` → localStorage, preserving the exact INI text.
  - **Sound**: `sndPlaySoundA` → WebAudio, per M1's findings (if it plays
    Windows system sounds, use the authentic WAVs from Wine's own resources).
- **Font**: capture the exact glyphs by rendering the original's on-screen
  text under Wine (all characters used: digits, `:`, `.`, `/`, `m`, `s`,
  space, the string-table text), bake into a bitmap font with matching
  advance widths; the shim's text path draws from it → text pixels match,
  eliminating the one planned deviation.
- Build `src/` + `shim/` with emcc (single-threaded; no pthreads, no
  asyncify, no COOP/COEP).
- WASM-vs-rebuild frame diff in headless Chrome (rAF tick counting,
  no-throttle flags, swiftshader or real GPU — same harness recipes as the
  OBOE port).
- Exit criterion: zero-pixel-diff WASM-vs-rebuild for the M2 script suite
  (WASM-vs-original follows by transitivity).

### M4 — Ship

- `web/` app: keyboard 1:1 mapping (final VK list from M1: steering keys,
  crouch, F3 pause, mode keys), integer 1×/2×/3× canvas scaling, high scores
  persisting in localStorage, click-to-focus + key handling.
- Full verification matrix re-run end to end; write README (layout, build,
  run, provenance, the three M1 answers, verification results); commit.

## Verification design (summary)

- Staged: (1) rebuild vs original, both in Wine — validates the
  decompilation; (2) WASM vs rebuild — validates the shim; (3) WASM vs
  original follows transitively.
- Determinism: frozen RNG seed (original hex-patched, rebuild via flag) +
  scripted inputs.
- Alignment: content-hash frame matching (works regardless of wall-clock
  jitter once M1 confirms fixed-step physics).
- Sound: verified by construction (same assets, same trigger calls) +
  manual listen on the M4 build; automated audio capture is out of scope.

## Risks & mitigations (decided)

| Risk | Mitigation |
|---|---|
| x87 80-bit FP intermediates in MSVC6 code | native rebuild uses `-mfpmath=387`; M1 inventories FP ops in game code; if divergences appear, locate and match the exact expressions |
| Wine timing jitter breaks frame alignment | content-hash alignment; fixed-step physics (expected) makes frame N a pure function of (seed, inputs) |
| Font pixels differ | bitmap font captured from the original's own rendering under Wine |
| Ghidra misreconstructs a tricky function (inlines, unions, x87) | cross-check against behavior via the M2 diff; `decompile/` keeps raw Ghidra output alongside annotated C |
| 32-bit Wine setup friction on this host | `wine32:i386` multiarch or new WoW64; setup task in the plan with a fallback note |
| `button` class / status-window interactivity more complex than expected | M1 documents it; shim implements whatever it is — scope is bounded by the import list |

## Out of scope

- Any contact with the author / licensing for public release (private project).
- Public hosting, deployment, CI.
- Touch/mobile input; anything the original doesn't implement (no multiplayer,
  no editors — 1.04 has none).
- Automated audio waveform verification (manual listen only).
- Decompiling the MSVC CRT (documented and skipped; not ported).

## Acceptance criteria (project complete when)

1. `decompile/` contains annotated C for 100% of non-CRT code with a
   complete function map and data-model notes; the three M1 questions are
   answered with evidence.
2. `ski-rebuild.exe` (from `src/`, mingw-w64) produces zero-pixel-diff frames
   vs `ski32.exe` in Wine across the full scripted suite (M2 exit).
3. The WASM build in a desktop browser produces zero-pixel-diff frames vs
   `ski-rebuild.exe` across the same suite (M3 exit).
4. The game is playable in the browser: all modes, monster chase, pause,
   high scores persisting across reloads, sound (if the original has any).
