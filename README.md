This project was used as a sort of benchmark for Qwen 3.8 27B. All work was performed by Qwen 3.8 27B using oh-my-pi (https://github.com/can1357/oh-my-pi).

The model was able to:
* dissemble the original ski32.exe
* create a full decompilation, using Ghidra and the disassembly
* use this new decompiled source to build a WASM port.

6x the file size for portability...there are certainly additional optimizations that could likely be made, but as a benchmark this was sufficient.

Playable here - https://mtstanfield.github.io/ski32.html

---

# SkiFree 1.04 — decompilation + pixel-perfect WASM port

A full annotated decompilation of **SkiFree 1.04 32-bit** (`original/ski32.exe`),
and a browser port compiled from the same reconstructed C, verified
frame-by-frame to be **pixel-identical** to the original.

## Layout

```
skifree-wasm/
├── original/     ski32.zip, ski32.exe (reference, checked in)
├── decompile/    Ghidra output + NOTES.md: the function map, data model,
│   ghidra/      annotated C (FUN_*.c) — the decompilation deliverable
├── src/          reconstructed C (ski_win.c, ski_core.c); every block cites
│                 the Ghidra address it was transcribed from
├── shim/         Win32/GDI/WinMM/INI emulation — linked into the WASM build only
├── harness/      seed-freeze + stub patchers, input scripts, frame capture,
│                 content-hash differ, scenario suite
├── web/          index.html, boot.js, font.json, ski.js/ski.wasm (build output)
└── CMakeLists.txt  native (mingw-w64) and web (emscripten) builds
```

One source of truth: `src/` compiles **unmodified** for three runtimes —
the native rebuild (mingw-w64, runs in Wine with real GDI) and the WASM port
(emscripten + the `shim/`). Game logic is byte-identical across runtimes; only
the platform layer differs.

## Build

Prerequisites (pinned in `harness/TOOLCHAIN.md`): CMake ≥ 3.22,
`i686-w64-mingw32-gcc` (mingw-w64 i386), emsdk (emcc), Wine 9.0 (32-bit),
Python 3.12 + Pillow (`harness/.venv`), Node ≥ 22, Java 21 + Ghidra 12.1.3
(decompilation only).

**Native rebuild** (the M2 reference; runs in Wine):
```bash
rm -rf build-native-harness && mkdir build-native-harness && cd build-native-harness
cmake -DCMAKE_BUILD_TYPE=Release -DSKI_HARNESS=ON \
      -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
      -DCMAKE_RC_COMPILER=i686-w64-mingw32-windres ..
make -j8 ski            # -> build-native-harness/ski.exe  (zero -Wall warnings)
```

**Web (WASM) port** (the shipping app):
```bash
source ~/.emsdk/emsdk_env.sh
emcmake cmake -B build-web -DSKI_DETERMINISTIC=ON -DSKI_HARNESS=OFF
cmake --build build-web -j
cp build-web/ski.js build-web/ski.wasm web/     # web/ is the served app
```
The `-DSKI_HARNESS=ON` variant (with a per-tick frame seal) is the WASM side of
the diff harness; the shipping app is `-DSKI_HARNESS=OFF`.

**Host unit tests** (native, ASan/UBSan):
```bash
cmake -B build-test && cmake --build build-test -j && ctest --test-dir build-test --output-on-failure
```

## Run

```bash
python3 -m http.server 8080 --directory web     # -> http://localhost:8080/
```
Open the URL in a desktop browser (hard-refresh after a rebuild). The canvas is
the reference 1024×768 screen; the main window client sits at (132,30). It
auto-scales to `min(100vw, 1024px)` with nearest-neighbour (crisp) rendering.

**Controls** (1:1 with the original — arrows *or* the original numpad):
- Steer: ← → (or Numpad4/6) · Crouch: `−`/Insert (or Numpad0) · Face: ↑ ↓ (or Numpad8/2), Numpad1/3/7/9
- **F2** restart · **F3** pause · **Enter** advance (post-crash score screen)
- **Mouse**: move to steer (the skier faces the cursor relative to center); click to jump (with a player) or restart (without)
- Letters (debug, as in the original): **F** = turbo (2× movement), x/X/y/Y = teleport, r = render, t = manual tick

High scores persist in `localStorage` (backed to the INI key `entpack.ini`),
surviving reloads, exactly as the original persists to `entpack.ini`.

## Decompilation

Ghidra **12.1.3** headless (`-processor "x86:LE:32:default"`) decompiled all of
`.text`. **168 functions**: **106 GAME** (the ski2.c logic — renamed, one row
each in `decompile/NOTES.md`) and **62 CRT** (MSVC6 runtime — documented, not
ported). The annotated C is `decompile/ghidra/FUN_*.c`; the data model (80-byte
entity struct, gate descriptors, level layout, sprite tables) is in
`decompile/NOTES.md`, verified against `objdump -d` where Ghidra's decompiler
and the disassembly disagree.

**The three port-critical M1 answers:**
1. **RNG** — the statically-linked MSVC CRT LCG, inlined into the image:
   `c16c = c16c*0x343fd + 0x269ec3; return (c16c>>16) & 0x7fff;` (15-bit).
   Seeded from `GetTickCount()` on every reset/restart; each `game_start` draws
   3 RNG values per gate × 39 gates (117 consumptions) in a fixed order.
   Frozen for the diff (original hex-patched, rebuild via flag).
2. **Timing** — one `SetTimer` (id 0x29a, **40 ms**) drives a fixed-step tick;
   input is **edge-triggered WM_KEYDOWN** (no key state, no WM_KEYUP — a held
   key arrives as repeated keydowns).
3. **Sound** — 9 `FindResourceA(..., "WAVE")` calls, but the PE has **no
   RT_WAVE node**, so all return NULL and the game is **silent in every
   configuration**. The port omits audio (faithful).

## Verification

Three-stage, content-addressed, **tolerance 0**:
1. **M2** — native rebuild vs original, both in Wine. Validates the
   decompilation (reconstructed C == the original binary).
2. **M3** — WASM vs native rebuild, in headless Chrome. Validates the shim
   (the Win32/GDI emulation).
3. **WASM vs original** follows by transitivity.

A frozen RNG seed + scripted per-tick inputs make frame N a pure function of
(seed, inputs), so the three runtimes are aligned tick-for-tick (shift 0). The
differ (`harness/diff.py`) pixel-compares the 760×734 scene, masking only the
status-panel rect (620,0)–(760,60) — that region is verified separately (see
Known deltas).

Re-run the full matrix:
```bash
# instrumented original (seed-freeze + tick hook + pause bypass):
python3 harness/stub_patch.py /tmp/orig_t15.exe
# M2: rebuild vs original (wine, Xvfb :99) — per scenario:
bash harness/run_scenario.sh <scen> $PWD/build-native-harness/ski.exe harness/frames/rebuild_<scen>
bash harness/run_scenario.sh <scen> /tmp/orig_t15.exe harness/frames/orig_<scen>
harness/.venv/bin/python harness/diff.py harness/frames/orig_<scen> harness/frames/rebuild_<scen>
# M3: wasm vs rebuild (headless Chrome):
bash harness/run_scenario_wasm.sh <scen>
```
Scenarios: `s01_menu s02_start s03_steering s04_crouch s05_modes
s06_longrun s07_monster s08_pause_scores`. The committed run is recorded in
`evidence/final-verification.txt`.

| Scenario            | M2 rebuild vs original | M3 wasm vs rebuild |
|---|---|---|
| s01_menu            | 303 frames, 0px | 301 frames, 0px |
| s02_start           | 403 frames, 0px | 401 frames, 0px |
| s03_steering        | 808 frames, 0px | 801 frames, 0px |
| s04_crouch          | 401 frames, 0px | 401 frames, 0px |
| s05_modes           | 808 frames, 0px | 801 frames, 0px |
| s06_longrun         | 3004 frames, 0px | 3001 frames, 0px |
| s07_monster         | 4004 frames, 0px | 4001 frames, 0px |
| s08_pause_scores    | 301 frames, 0px | 301 frames, 0px |

**16/16 PASS** — zero differing scene pixels in every frame of every scenario. By
transitivity (M2: rebuild == original; M3: wasm == rebuild), the WASM port is
pixel-identical to `ski32.exe`.

## Known deltas

- **Text font provenance.** The status/label glyphs are baked from the
  original's own on-screen rendering under Wine (all characters used: digits,
  `: . / m s`, space, the string-table text) — `web/assets/font.json`, 43,689 B,
  with the original's advance widths. This eliminates font pixel drift.
- **Sound.** None — the PE carries no WAVE resources, so `sndPlaySoundA` never
  fires (M1 answer 3). The shim's `PlaySoundA` is a no-op (faithful).
- **`GetVersion`.** Called only by the CRT PE entry (OS-version detection,
  stored to `c7b4/c7b8/c7bc`); the WASM build uses emscripten's entry, so it is
  not ported and has no game-logic effect.
- **s08_pause_scores.** F3 pause kills the 40 ms timer at tick 301; both sides
  stop there (a live-game property, not a bug). The differ compares the frame
  overlap (the whole of the stopped run).
- **Status panel.** The 8-scenario scene diff masks the panel rect (620,0)–(760,60).
  The panel itself (Time/Dist/Speed/Style) is drawn to the status child window's
  DC outside `WM_PAINT` (327 ms throttle); the shim emulates wine's
  `WM_ERASEBKGND` + invalidate-on-own-DC-draw so it clears and redraws crisp
  every paint (verified 0px vs the original, unmasked, M4).
- **Determinism.** The diff freezes the RNG seed (original hex-patched, rebuild
  via `-DSKI_DETERMINISTIC`). A live game reseeds on `GetTickCount()` at each
  F2/restart — faithful to the original, but a live run is not reproducible.
