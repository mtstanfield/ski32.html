# SkiFree Decompilation + WASM Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Full annotated decompilation of `original/ski32.exe` (SkiFree 1.04 32-bit) and a pixel-perfect WASM port playable in a desktop browser, verified by staged frame-by-frame pixel diffing.

**Architecture:** One source of truth — reconstructed C in `src/` — compiled unmodified for mingw-w64 (native, runs in Wine) and emscripten (WASM, behind a Win32/GDI shim in `shim/`). Ghidra headless produces the annotated decompilation in `decompile/`; a harness in `harness/` freezes the RNG seed, replays scripted inputs, captures frames from all three runtimes, aligns them by content hash, and pixel-diffs them.

**Tech Stack:** Ghidra headless (Java 17+), mingw-w64 i686, Wine 9.0 + xvfb + xdotool + ImageMagick, Emscripten (emsdk), CMake, Python 3.12 + Pillow, Node ≥22 (built-in WebSocket for CDP), C (C99).

**Spec:** `docs/superpowers/specs/2026-08-24-skifree-decompile-wasm-design.md` — read it first.

**Ground rules for every task:**
- All paths relative to the repo root (`skifree-wasm/`).
- Commit after every task (exact commands given). Never commit frame dumps (`harness/frames/` is gitignored).
- The three "M1 answers" (RNG, timing, sound — Task 7) gate Tasks 8, 11, 12, 18. If an answer differs from a plan assumption, follow the answer and record the delta in `decompile/NOTES.md`; do not silently diverge.
- Pixel diffs pass at **0 differing pixels**. If a diff is nonzero, diagnose the capture pipeline first (capture is a common suspect), then the code. Never raise tolerance without recording why in `evidence/`.
- Prefix shell commands with `rtk` per project convention.

---

## M0 — Environment (Tasks 1–3)

### Task 1: Toolchain setup

**Files:** Modify `.gitignore`; create `harness/`.

- [ ] **Step 1: Verify emsdk.** Run: `source ~/.emsdk/emsdk_env.sh && emcc -v`. Expected: emcc banner (the OBOE port used emcc 6.x here). If `~/.emsdk` is missing: clone `https://github.com/emscripten-core/emsdk` into `~/.emsdk`, then `~/.emsdk/emsdk install latest && ~/.emsdk/emsdk activate latest`.
- [ ] **Step 2: Install system packages** (pty shell; sudo):
```bash
sudo dpkg --add-architecture i386 && sudo apt-get update
sudo apt-get install -y wine wine64 wine32:i386 gcc-mingw-w64-i686 xvfb xdotool x11-apps imagemagick python3-venv
```
- [ ] **Step 3: Verify.** Run: `i686-w64-mingw32-gcc --version | head -1 && wine --version && xvfb-run -a echo xvfb-ok && node --version`. Expected: all four print; Node ≥ v22 (Task 19's CDP script uses built-in `WebSocket`).
- [ ] **Step 4: Python venv.** Run: `python3 -m venv harness/.venv && harness/.venv/bin/pip install pillow && harness/.venv/bin/python -c "import PIL; print(PIL.__version__)"`.
- [ ] **Step 5: Commit.** Append `harness/frames/` and `evidence/*.tmp` to `.gitignore`, then:
```bash
git add .gitignore && git commit -m "chore: env prep — gitignore harness frames"
```

### Task 2: Ghidra install + import smoke test

**Files:** create `harness/TOOLCHAIN.md`.

- [ ] **Step 1: Download + unpack.**
```bash
mkdir -p ~/tools && cd ~/tools
curl -fsSL -o ghidra.zip https://github.com/NationalSecurityAgency/ghidra/releases/latest/download/ghidra_11.4_public_20250718.zip
unzip -q ghidra.zip && ls -d ghidra_11.4*
```
(If the pinned URL 404s, take the latest `ghidra_*_public_*.zip` asset from `https://github.com/NationalSecurityAgency/ghidra/releases/latest`.)
- [ ] **Step 2: Headless import smoke test.**
```bash
GHIDRA=$(ls -d ~/tools/ghidra_*/ghidraRun)
$GHIDRA/analyzeHeadless ~/tools/ghidra-project ski32-smoke -import original/ski32.exe -processor "x86:IA32:default,little" -overwrite
```
Expected: report with no `ERROR` lines; project dir `~/tools/ghidra-project/ski32-smoke` exists (throwaway; the real project is Task 4's).
- [ ] **Step 3: Commit pins.** Write `harness/TOOLCHAIN.md` with the exact versions (Ghidra, emcc, wine, mingw, node), then:
```bash
git add harness/TOOLCHAIN.md && git commit -m "chore: record toolchain pins"
```

### Task 3: Run the original in Wine, capture evidence

**Files:** create `harness/run_original.sh`; evidence `evidence/m0-original-menu.png`, `evidence/m0-status-window.png`, `evidence/m0-geometry.txt`.

- [ ] **Step 1: Write `harness/run_original.sh`:**
```bash
#!/usr/bin/env bash
# Launch ski32.exe under xvfb+Wine; print window IDs; keep X alive.
set -e
export WINEPREFIX=${WINEPREFIX:-$HOME/.wine-ski}
xvfb-run -a -s "-screen 0 1024x768x24" wine "$@" original/ski32.exe &
XPID=$!
for i in $(seq 1 45); do
  MAIN=$(xdotool search --name "SkiFree" 2>/dev/null | head -1 || true)
  [ -n "$MAIN" ] && break
  sleep 2
done
[ -z "$MAIN" ] && { echo "WINDOW NOT FOUND" >&2; exit 1; }
echo "MAIN=$MAIN STATUS=$(xdotool search --name 'SkiFree' | tail -1)"
wait $XPID
```
`chmod +x harness/run_original.sh`
- [ ] **Step 2: Launch + screenshot** (pty shell; first run does `wineboot`, allow ~60s):
```bash
nohup bash harness/run_original.sh > /tmp/ski-wine.log 2>&1 &
sleep 20
MAIN=$(xdotool search --name SkiFree | head -1); sleep 5
import -window "$MAIN" evidence/m0-original-menu.png
import -window "$(xdotool search --name SkiFree | tail -1)" evidence/m0-status-window.png
```
Expected: `evidence/m0-original-menu.png` shows the real title/menu screen. **No screenshot = do not proceed** — the whole pipeline builds on this. If the window never appears, read `/tmp/ski-wine.log`, fix wine setup (`wineboot -u` under the same xvfb) and retry.
- [ ] **Step 3: Record geometry:** `xdotool getwindowgeometry $(xdotool search --name SkiFree | head -1) | tee evidence/m0-geometry.txt` and the same for the second window appended. These pixel sizes become the port's canvas dimensions (Task 16).
- [ ] **Step 4: Kill + commit:** `pkill -f ski32.exe || true; git add harness/run_original.sh evidence/ && git commit -m "M0: original runs under Wine; menu + status window evidence"`.

---

## M1 — Decompile (Tasks 4–8)

### Task 4: Ghidra headless decompilation dump

**Files:** create `harness/ghidra/DumpDecompiled.java`; generated `decompile/ghidra/`.

- [ ] **Step 1: Write the postScript** — `harness/ghidra/DumpDecompiled.java`:
```java
// Ghidra headless postScript: decompile every function, dump C + function list.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompResults;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.address.Address;
import ghidra.util.task.TaskMonitor;
import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;

public class DumpDecompiled extends ghidra.script.GhidraScript {
    @Override
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        TaskMonitor monitor = new TaskMonitor() {};
        String outDir = System.getenv("GH_DUMP_DIR");
        if (outDir == null) outDir = "decompile/ghidra";
        File dir = new File(outDir);
        dir.mkdirs();
        PrintWriter json = new PrintWriter(new FileWriter(dir + "/function-map.json"));
        json.print("[\n");
        boolean first = true;
        for (Function f : fm.getFunctions(true)) {
            Address entry = f.getEntryPoint();
            String tag = String.format("%08x", entry.getOffset());
            DecompResults res = dec.decompile(f, 60, monitor);
            String c = (res != null && res.decompileCompleted())
                    ? res.getDecompiledFunction().getC() : "// DECOMPILE FAILED";
            PrintWriter w = new PrintWriter(new FileWriter(dir + "/FUN_" + tag + ".c"));
            w.println("/* " + f.getName() + " @ 0x" + entry +
                      " size=" + f.getBody().getNumAddresses() + " */");
            w.print(c);
            w.close();
            if (!first) json.print(",\n");
            first = false;
            json.printf("{\"name\":\"%s\",\"addr\":%d,\"size\":%d,\"file\":\"FUN_%s.c\"}",
                    f.getName().replace("\"", "'"), entry.getOffset(),
                    f.getBody().getNumAddresses(), tag);
            println("decompiled " + f.getName() + " @0x" + entry);
        }
        json.print("\n]\n");
        json.close();
        dec.dispose();
        println("DONE: " + dir + "/function-map.json");
    }
}
```
- [ ] **Step 2: Run the analysis** (from repo root):
```bash
GHIDRA=$(ls -d ~/tools/ghidra_*/ghidraRun)
export GH_DUMP_DIR=$PWD/decompile/ghidra
$GHIDRA/analyzeHeadless ~/tools/ghidra-project ski32 \
  -import original/ski32.exe -processor "x86:IA32:default,little" \
  -postScriptDir harness/ghidra -postScript DumpDecompiled -overwrite
```
Expected: one `decompiled FUN_...` line per function (~300–600 incl. CRT), then `DONE: .../function-map.json`. Script compile error → fix imports in the .java (all Ghidra API classes are on the script classpath) and re-run.
- [ ] **Step 3: Sanity-check:** `ls decompile/ghidra | wc -l` matches the function-map entry count; `head -40 decompile/ghidra/FUN_00406d83.c` shows recognizable startup code. Any `DECOMPILE FAILED` file → re-run that function with a longer timeout (bump `60` to `120` in the script) before continuing.
- [ ] **Step 4: Commit:** `git add harness/ghidra/ decompile/ghidra/ && git commit -m "M1: Ghidra decompilation of all functions + function map"`.

### Task 5: Function triage + naming (CRT vs game)

**Files:** create `harness/triage.py`; modify `decompile/ghidra/function-map.json`; create `decompile/NOTES.md`, `decompile/ghidra/globals.json`.

- [ ] **Step 1: Write `harness/triage.py`:**
```python
#!/usr/bin/env python3
"""Classify functions CRT vs GAME.
GAME signal: decompiled C references data in .data (0x40c0xx), .rsrc (0x40d0xx),
or game .rdata (0x40a1xx-0x40a6xx / 0x40c0xx strings like entpack.ini).
Ghidra emits data refs as DAT_0040XXXX globals — scan for those.
"""
import json, re, pathlib
d = pathlib.Path("decompile/ghidra")
fm = json.loads((d / "function-map.json").read_text())
pat = re.compile(r"DAT_0040([a-fc-d])([0-9a-f]{2})")
for f in fm:
    c = (d / f["file"]).read_text()
    hits = {(b0 + b1) for b0, b1 in pat.findall(c)}
    f["class"] = "GAME" if hits else "CRT"
    f["dat_refs"] = sorted(hits)[:12]
(d / "function-map.json").write_text(json.dumps(fm, indent=1))
g = [f for f in fm if f["class"] == "GAME"]
print(f"GAME: {len(g)}  CRT: {len(fm) - len(g)}")
for f in g:
    print(f"  0x{f['addr']:08x} {f['name']} size={f['size']} refs={f['dat_refs']}")
```
Run: `harness/.venv/bin/python harness/triage.py`. Expected: ~60–150 GAME functions. **Hand-review the split**: every `size > 400` function marked CRT must be opened and checked — if it touches windows/skiing/graphics it is GAME; adjust the `pat` byte ranges and re-run until the split is defensible. Record final counts in NOTES.
- [ ] **Step 2: Name every GAME function.** Edit `function-map.json`: move Ghidra's name into `"ghidra_name"`, set `"name"` to a conventional name: `main_*` (startup/message loop — the function calling `GetMessage`), `wproc_*` (functions passed to `CreateWindowEx` — find via the `RegisterClassA`/`CreateWindowExA` argument data), `game_*` (simulation), `draw_*` (BitBlt/PatBlt/TextOut callers), `score_*` (entpack.ini), `snd_*` (sndPlaySoundA callers), `util_*`. Also create `decompile/ghidra/globals.json` naming at minimum: the `entpack.ini` ptr, the three window-class name ptrs (`SkiMain`/`SkiStatus`/`button`), the `SkiFree` title ptr, the `nosound`/`WAVE` ptrs, and every heavily-read global a GAME function uses (these become struct fields in Task 7). Every GAME `dat_refs` entry must be explainable by `globals.json`; unexplained → investigate now.
- [ ] **Step 3: Create `decompile/NOTES.md`:**
```markdown
# SkiFree 32-bit decompilation notes

Source: original/ski32.exe (118,784 B, PE32, 2005-10-02, assert path
V:\hack\ski32\ski2.c). Ghidra <version from harness/TOOLCHAIN.md>.

## Function map
GAME=NN CRT=NNN.
| addr | name | ghidra_name | size | role (one line) |
|------|------|-------------|------|-----------------|
(one row per GAME function)

## Sprite inventory
(Task 6 fills)

## Data model
(Task 7 fills)

## M1 answers
(Task 7 fills)
```
- [ ] **Step 4: Commit:** `git add decompile/ harness/triage.py && git commit -m "M1: function triage + full naming of game code"`.

### Task 6: Resource extraction (sprites + strings → assets)

**Files:** create `harness/extract_resources.py`; generated `web/assets/sprites/bmp_NNN.png`, `web/assets/resources.json`.

- [ ] **Step 1: Write `harness/extract_resources.py`:**
```python
#!/usr/bin/env python3
"""Extract all RT_BITMAP sprites + RT_STRING tables from ski32.exe.
Leaf offsets in this PE follow an inconsistent convention, so each candidate
is validated (BITMAPINFOHEADER size 40, sane dims/bpp, exact payload size).
"""
import json, struct, sys, pathlib
from PIL import Image

data = pathlib.Path("original/ski32.exe").read_bytes()
OUT = pathlib.Path("web/assets/sprites"); OUT.mkdir(parents=True, exist_ok=True)
base, RVA_BASE = 0xd000, 0x40d000
HDR = struct.Struct("<IIHHHH"); ENT = struct.Struct("<II")

def read_dir(off):
    _, _, _, _, nn, ni = HDR.unpack_from(data, base + off)
    out, tab = [], base + off + 16
    for _ in range(nn + ni):
        nid, off2 = ENT.unpack_from(data, tab); tab += 8
        if nid == 0xFFFFFFFF:
            n = struct.unpack_from("<H", data, base + nid)[0]
            nid = data[base+nid+2:base+nid+2+n].decode("utf-16-le")
        out.append((nid, off2 & 0x7FFFFFFF))
    return out

def leaf_data(l_off):
    rva, size, cp, res = struct.unpack_from("<IIII", data, base + l_off)
    for off in (base + rva, rva, rva - RVA_BASE + base):
        yield off, size

def valid_bitmap(off, size):
    if not (0 < off < len(data)) or off + size > len(data): return None
    hs, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", data, off)
    if hs != 40 or comp != 0 or planes != 1 or bpp not in (1, 4, 8): return None
    if not (1 <= w <= 512 and 1 <= abs(h) <= 512): return None
    rowsz = (w * abs(h) * bpp + 31) // 32 * 4
    pal = (2 ** bpp - 1) * 3
    if 40 + pal + rowsz * abs(h) != size: return None
    return (w, -h, bpp)

entries = []  # (kind, id, off, size)
for tid, t_off in read_dir(0):
    if isinstance(tid, str):
        for _, l_off in read_dir(t_off):
            for off, size in leaf_data(l_off):
                entries.append(("icon", tid, off, size)); break
    elif tid == 2:
        for nid, n_off in read_dir(t_off):
            for _, l_off in read_dir(n_off):
                for off, size in leaf_data(l_off):
                    if valid_bitmap(off, size):
                        entries.append(("bmp", nid, off, size)); break
    elif tid == 6:
        for nid, n_off in read_dir(t_off):
            for _, l_off in read_dir(n_off):
                rva, size, _, _ = struct.unpack_from("<IIII", data, base + l_off)
                for off in (rva, base + rva, rva - RVA_BASE + base):
                    if 0 < off < len(data) - size:
                        entries.append(("str", nid, off, size)); break

res = {"bitmaps": {}, "strings": {}}
for kind, nid, off, size in entries:
    if kind == "str":
        buf = data[off:off + size]
        n = struct.unpack_from("<H", buf, 0)[0]
        i, s = 2, []
        for _ in range(n):
            j = buf.index(b"\x00\x00", i)
            s.append(buf[i:j].decode("utf-16-le")); i = j + 2
        res["strings"][nid] = s
        print(f"string group {nid}: {s}")
    elif kind == "bmp":
        hs, w, h, planes, bpp, comp = struct.unpack_from("<IiiHHI", data, off)
        h = -h
        pal_n = 2 ** bpp - 1
        pal = [data[off + 40 + k * 3: off + 43 + k * 3] for k in range(pal_n)]
        rowsz = (w * h * bpp + 31) // 32 * 4
        raw = data[off + 40 + pal_n * 3: off + 40 + pal_n * 3 + rowsz * h]
        img = Image.new("RGB", (w, h)); px = img.load()
        for y in range(h):
            for x in range(w):
                if bpp == 8: idx = raw[y * rowsz + x]
                else:
                    byte = raw[y * rowsz + (x * bpp) // 8]
                    idx = (byte >> ((7 - (x * bpp) % 8) // bpp)) & (pal_n)
                px[x, y] = tuple(pal[idx])
        p = OUT / f"bmp_{nid:03d}.png"
        img.save(p)
        res["bitmaps"][nid] = {"w": w, "h": h, "bpp": bpp, "file": str(p)}
        print(f"bitmap {nid}: {w}x{h} {bpp}bpp -> {p}")

pathlib.Path("web/assets/resources.json").write_text(json.dumps(res, indent=1))
print(f"OK: {len(res['bitmaps'])} bitmaps, {len(res['strings'])} string groups")
sys.exit(0 if len(res["bitmaps"]) == 89 else 1)
```
- [ ] **Step 2: Run:** `harness/.venv/bin/python harness/extract_resources.py`. Expected: 89 `bitmap N:` lines, the two string groups matching the 18+2 strings already identified (`SkiFree`, `Ski Paused ...`, `Time:`, `Dist:`, `Speed:`, `Style:`, the formats, `High Scores`, ` <-- that's you!`, ` <-- try again!`), exit 0. If count < 89, inspect the rejected leaves (bottom-up vs top-up rows flip visuals — `img.transpose(Image.FLIP_TOP_BOTTOM)` if a sprite is upside down; re-check the payload formula) until 89/89.
- [ ] **Step 3: Inventory.** Look at the PNGs (read them), identify skier frames, trees, stumps, dogs, flags, the Abominable Snow Monster, score-panel art; add a `## Sprite inventory` section to `decompile/NOTES.md` (id → identity, dims, frame sets). This mapping drives Task 11.
- [ ] **Step 4: Commit:** `git add harness/extract_resources.py web/assets/ decompile/NOTES.md && git commit -m "M1: extract 89 sprites + string tables to web/assets"`.

### Task 7: Data model + the three M1 answers (analysis, no new code)

**Files:** modify `decompile/NOTES.md` (fill `## Data model` and `## M1 answers`).

Read the named GAME functions in `decompile/ghidra/`. Everything must be evidence-linked (address + C snippet).

- [ ] **Step 1: Data model.** Document in NOTES.md:
  - Skier state (position/velocity/airtime/mode/score/style fields, byte offsets as seen in the C).
  - Obstacle/object lists (array bounds, element layout, bitmap ID per type — tie to the Task 6 inventory).
  - Window records: what the `button`-class windows are (count, labels, handlers); `SkiMain` and `SkiStatus` contents.
  - Mode model: how many modes, switch key(s), per-mode update differences, the `Style:` label's role.
  - Monster trigger (confirm the 2000 m rule from the code) and all end conditions.
  - High-score record layout + exact `entpack.ini` section/keys (from the `GetPrivateProfileStringA`/`WritePrivateProfileStringA` call sites).
  - Exact key map (VK codes): steer left/right, crouch, pause (F3 per the string table), mode switches, high-score advance/exit, any others. **Also state the input mechanism**: per-tick `GetKeyState` sampling (record every call site + address) or `WM_KEYDOWN`/`WM_CHAR` message handling — Task 14's injection depends on this.
- [ ] **Step 2: Answer 1 — RNG.** Custom inline recurrence (record constants) or CRT `rand()` (record the `srand` argument — `GetTickCount`/`time`/constant — and the seed site's function + address). CRT `rand()` means Task 11 uses the MSVC algorithm (reference code is in Task 11).
- [ ] **Step 3: Answer 2 — Timing.** The `SetTimer` id + period (ms); whether the update consumes `GetTickCount` deltas (dt-based) or steps fixed per tick (fixed-step); record the exact path: WM_TIMER handler → update function → time reads. **Also record the "tick site": the exact address of the first instruction of the per-tick update** (Task 14 hooks it), and **the key-state global**: the address + bit layout of wherever the WndProc's WM_KEYDOWN/WM_KEYUP handling stores the held keys (the game does NOT import `GetKeyState`/`GetAsyncKeyState` — input is message-derived; Task 14 writes this global per tick). If the game has no such global (keys consumed directly from messages), say so — Task 14's fallback applies.
- [ ] **Step 4: Answer 3 — Sound.** Every `sndPlaySoundA` call site: argument (filename? resource? system sound name — resolve the `WAVE`/`nosound` string roles) and flags; whether the shipped game is silent by default; which Windows system sounds (if any) must be reproduced.
- [ ] **Step 5: Commit:** `git add decompile/NOTES.md && git commit -m "M1: data model + RNG/timing/sound answers (evidence-linked)"`.

**Exit criterion (gates Tasks 8/11/12/18):** all three answers present with evidence and the data-model bullets filled. If something can't be answered from the decompiled C, record exactly what's missing and the resolution path — never guess.

### Task 8: Determinism artifacts (seed patch + input scenarios)

**Files:** create `harness/seed.json`, `harness/seed_patch.py`, `harness/gen_seed_header.py`, `src/seed_values.h`, `harness/scenarios/s01..s08_*.json`.

- [ ] **Step 1: Write `harness/seed.json` from Task 7:**
```json
{
  "seed_constant": 12345,
  "original_patch": {
    "addr": "0x40XXXX",
    "bytes_before": "AA BB CC",
    "bytes_after": "DD DD DD",
    "note": "replaces the time/GetTickCount seed source with the constant"
  },
  "rng": "msvc_rand",
  "timing": {"settimer_ms": 33, "model": "fixed-step"},
  "keys": {
    "left": "VK_LEFT", "right": "VK_RIGHT", "crouch": "VK_SPACE",
    "pause": "VK_F3", "modes": ["VK_S"], "advance": "VK_RETURN"
  }
}
```
(`rng` is `"msvc_rand"` or `"custom:<recurrence>"`; `keys`/`timing`/`model` values from NOTES — the JSON above is the shape, not the values. `addr`/bytes from the seed site: `bytes_before` = the original bytes at that file-mapped address, `bytes_after` = the patched immediate(s); get both from the disassembly of the seed site — `objdump -d original/ski32.exe` around the address, or Ghidra's listing.)
- [ ] **Step 2: Write `harness/seed_patch.py`:**
```python
#!/usr/bin/env python3
"""Apply the seed-freeze hex patch to a COPY of ski32.exe.
This PE maps sections 1:1 (VMA - ImageBase == file offset for .text/.rdata/.data/.rsrc:
.text 0x401000->0x1000, .rdata 0x40a000->0xa000, .data 0x40c000->0xc000, .rsrc 0x40d000->0xd000).
Usage: seed_patch.py OUT_PATH
"""
import json, pathlib, sys
cfg = json.loads(pathlib.Path("harness/seed.json").read_text())
src = pathlib.Path("original/ski32.exe").read_bytes()
o = cfg["original_patch"]
off = int(o["addr"], 16) - 0x400000
before = bytes.fromhex(o["bytes_before"].replace(" ", ""))
after = bytes.fromhex(o["bytes_after"].replace(" ", ""))
assert len(before) == len(after), "patch length mismatch"
assert src[off:off + len(before)] == before, \
    f"byte mismatch at {off:#x}: got {src[off:off+len(before)].hex()}"
pathlib.Path(sys.argv[1]).write_bytes(src[:off] + after + src[off + len(before):])
print(f"patched {sys.argv[1]} at file offset {off:#x}")
```
- [ ] **Step 3: Verify determinism on the real binary.**
```bash
harness/.venv/bin/python harness/seed_patch.py /tmp/ski32-patched.exe
```
Run `/tmp/ski32-patched.exe` under Wine **twice** (fresh window each time, start a run, watch the first ~15 seconds of obstacle placement). Expected: identical obstacle fields in both runs. If they differ, the seed site/patch is wrong — re-derive from Task 7 before continuing. (Also run the unpatched original twice and confirm the fields DIFFER — proves the patch is what changed behavior.)
- [ ] **Step 4: Write `harness/gen_seed_header.py` + generate `src/seed_values.h`:**
```python
#!/usr/bin/env python3
import json, pathlib
cfg = json.loads(pathlib.Path("harness/seed.json").read_text())
pathlib.Path("src/seed_values.h").write_text(
    "/* generated by harness/gen_seed_header.py — do not edit */\n"
    f"#define SKI_SEED_CONSTANT {cfg['seed_constant']}\n"
    f"/* timing: {cfg['timing']['model']}, settimer_ms={cfg['timing']['settimer_ms']} */\n")
print("wrote src/seed_values.h")
```
Run: `harness/.venv/bin/python harness/gen_seed_header.py`.
- [ ] **Step 5: Write the 8 scenario scripts** in `harness/scenarios/`. Shape:
```json
{"name": "sNN", "ticks": 600, "events": [
  {"t": 50,  "key": "right",  "down": true},
  {"t": 150, "key": "right",  "down": false}
]}
```
`key` names from `seed.json.keys`. Scenarios:
  - `s01_menu` — ticks 300, no events (title/menu state).
  - `s02_start` — ticks 400, start/mode key down+up at t=100 (per NOTES key map).
  - `s03_steering` — ticks 800, alternate left/right every 100 ticks.
  - `s04_crouch` — ticks 400, crouch down t=100, up t=300.
  - `s05_modes` — ticks 800, press each mode key once per 200 ticks.
  - `s06_longrun` — ticks 3000, light steering (left at 200+600k, right at 500+600k) — long enough to hit obstacles and (in freestyle) pass 2000 m.
  - `s07_monster` — ticks 4000, freestyle no-crouch to trigger the monster; steer to survive.
  - `s08_pause_scores` — ticks 1200, pause down t=300 / up t=500; after the run ends, `advance` at t=900 and t=1000 (high-score entry).
  Tick counts/`t` values are first guesses — Task 13's first real run will expose mis-phased events; fix the JSON (data, not rework).
- [ ] **Step 6: Commit:** `git add harness/seed.json harness/seed_patch.py harness/gen_seed_header.py src/seed_values.h harness/scenarios/ && git commit -m "M1: seed freeze (verified on two Wine runs) + 8 input scenarios"`.

---

## M2 — Rebuild native + prove equality (Tasks 9–13)

### Task 9: CMake + native skeleton

**Files:** create `CMakeLists.txt`, `src/ski_main.c`, `src/ski_game.h`, `src/skidef.h`.

- [ ] **Step 1: Write `CMakeLists.txt`:**
```cmake
cmake_minimum_required(VERSION 3.22)
project(skifree C)
set(CMAKE_C_STANDARD 99)
option(SKI_DETERMINISTIC "frozen RNG seed (harness builds)" ON)
option(SKI_HARNESS "per-tick framebuffer dump hooks" OFF)
add_compile_definitions(
  WIN32
  SKI_DETERMINISTIC=$<IF:$<BOOL:${SKI_DETERMINISTIC}>,1,0>
  SKI_HARNESS=$<IF:$<BOOL:${SKI_HARNESS}>,1,0>
)
set(SKI_SOURCES src/ski_main.c)
if(EMSCRIPTEN)
  file(GLOB SHIM_SOURCES shim/*.c)
  list(APPEND SKI_SOURCES ${SHIM_SOURCES})
endif()
add_executable(ski ${SKI_SOURCES})
if(EMSCRIPTEN)
  target_link_options(ski PRIVATE
    -sENVIRONMENT=web
    -sINITIAL_MEMORY=16MB
    -sALLOW_MEMORY_GROWTH=1
    -sEXPORT_KEEPALIVE=1
    -sMODULARIZE=1 -sEXPORT_NAME=createSki
    -o $<TARGET_FILE_DIR:ski>/ski.js
  )
else()
  target_compile_options(ski PRIVATE -O2 -mfpmath=387 -Wall)
endif()
```
- [ ] **Step 2: Write the skeleton sources.**
`src/skidef.h`:
```c
#ifndef SKIDEF_H
#define SKIDEF_H
#include "seed_values.h"  /* Task 8 */
#endif
```
`src/ski_game.h`:
```c
/* Reconstructed SkiFree 1.04 (32-bit) game core.
 * Reconstructed from decompile/ghidra — each src file cites its source
 * Ghidra address range in a header comment.
 */
#ifndef SKI_GAME_H
#define SKI_GAME_H
#include <windows.h>
#include "skidef.h"

extern int g_ski_tick;
void ski_init(void);            /* globals, resources, windows        */
void ski_run(void);             /* message loop until WM_QUIT         */
void ski_tick(void);            /* one WM_TIMER game step             */
void ski_render_main(void);     /* draw main window (GDI)             */
void ski_render_status(void);   /* draw floating status window (GDI)  */
#endif
```
`src/ski_main.c`:
```c
#include "ski_game.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    (void)hInst; (void)hPrev; (void)cmd; (void)show;
    ski_init();
    ski_run();
    return 0;
}
```
- [ ] **Step 3: Build the skeleton.**
```bash
cmake -B build-native -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw-w64/i686/mingw-windows_i686.cmake -DSKI_DETERMINISTIC=ON -DSKI_HARNESS=OFF
cmake --build build-native -j
file build-native/ski.exe
```
Expected: `PE32 executable (GUI) Intel 80386`. (`ski_init`/`ski_run` don't exist yet — the skeleton build will fail to link; that is expected. Create `src/ski_core.c` with empty `ski_init`/`ski_run`/`ski_tick`/`ski_render_main`/`ski_render_status` bodies and `int g_ski_tick = 0;`, add it to `SKI_SOURCES`, rebuild → links. Tasks 10–12 replace the bodies.)
- [ ] **Step 4: Commit:** `git add CMakeLists.txt src/ && git commit -m "M2: CMake dual-toolchain skeleton builds native PE32"`.

### Task 10: Reconstruct windows + message loop

**Files:** create `src/ski_win.c`; modify `CMakeLists.txt` (`SKI_SOURCES`), `src/ski_core.c` (stubs).

**Source material:** the `main_*`/`wproc_*` functions from Task 5.

- [ ] **Step 1: Reconstruct `src/ski_win.c`** from the decompiled C: `RegisterClassA` ×3 (SkiMain, SkiStatus, button), `CreateWindowExA` calls (exact styles/positions/sizes — cross-check against `evidence/m0-geometry.txt`), `SetTimer` (id/period must equal `seed.json.timing.settimer_ms`), the message loop (`GetMessage`/`TranslateMessage`/`DispatchMessage`/`PostQuitMessage`), and each WndProc. File header cites the source functions + addresses, e.g.:
```c
/* Reconstructed from main_init (FUN_0040A210) + wproc_main (FUN_0040B5F0)
 * + wproc_status (FUN_0040C110) — see decompile/NOTES.md function map. */
```
Rules: keep all Win32 calls and their arguments exactly as decompiled (class names, styles, strings, timer id/period); fix only decompiler artifacts (wrong types, redundant loads, opaque stack slots). In the WM_TIMER handler, under `#ifdef SKI_HARNESS` (i.e. `#if SKI_HARNESS`), do `g_ski_tick++;` before calling the update — this is the frame index the dump hook uses.
- [ ] **Step 2:** Add `src/ski_win.c` to `SKI_SOURCES` in CMakeLists. Keep `ski_tick`/`ski_render_*` as stubs in `src/ski_core.c` (Task 11–12 fill them); the WndProcs call them at the points the original's decompiled code calls its equivalents.
- [ ] **Step 3: Build + run in Wine:**
```bash
cmake --build build-native -j
nohup bash harness/run_original.sh --rebuild build-native/ski.exe > /tmp/ski-r.log 2>&1 &
```
(`run_original.sh` passes extra args to wine after `--rebuild`… simpler: for the rebuild, launch directly — `WINEPREFIX=$HOME/.wine-ski xvfb-run -a -s "-screen 0 1024x768x24" wine build-native/ski.exe &`, then screenshot the same way as Task 3 → `evidence/m2-windows.png`.)
Expected: both windows appear with the original's sizes/titles (`xdotool getwindowname`).
- [ ] **Step 4: Kill + commit:** `pkill -f build-native/ski.exe || true; git add src/ CMakeLists.txt evidence/m2-windows.png && git commit -m "M2: windows + message loop reconstructed; matches original geometry"`.

### Task 11: Reconstruct the game core (state, physics, modes, collisions, monster)

**Files:** create/extend `src/ski_core.c` (or split `src/ski_game.c` / `src/ski_objects.c` if the reconstruction reads better — your call, cite addresses either way).

**Source material:** the `game_*` functions from Task 5.

- [ ] **Step 1: Reconstruct** every `game_*` function into `src/`, keeping control flow and arithmetic exactly. Critical fidelity rules:
  - **RNG**: if `seed.json.rng == "msvc_rand"`, replace every `rand()`/`srand()` call with these exact MSVC-CRT equivalents (the mingw glibc `rand()` has a different sequence — using it breaks determinism vs the original):
```c
/* MSVC CRT rand: 32-bit LCG, returns (state>>16) & 0x7fff */
static unsigned int ms_rand_state;
int  ski_srand(unsigned int seed) { ms_rand_state = seed; return 0; }
int  ski_rand(void) { ms_rand_state = ms_rand_state * 214013u + 2531011u;
                      return (int)((ms_rand_state >> 16) & 0x7fffu); }
```
    Seed it from `#if SKI_DETERMINISTIC → ski_srand(SKI_SEED_CONSTANT) #else → the original seed expression (per NOTES answer 1) #endif`. If `rng == "custom"`, reconstruct the exact recurrence from the decompiled code instead.
  - **FP**: keep the original's exact expressions and operand order (x87 `-mfpmath=387` is on; do not "simplify" arithmetic).
  - **Timing**: if NOTES says fixed-step, no clock reads in the update (the WM_TIMER handler IS the step). If dt-based: under `#if SKI_DETERMINISTIC` substitute the decompiled `GetTickCount` reads with a virtual clock `static unsigned int vtick_ms;` advanced by exactly `settimer_ms` per timer tick (advance in the WM_TIMER handler before the update).
  - Bitmap IDs, array bounds, magic constants: exactly as decompiled.
  - Route every key-state read through the wrapper `int ski_key_pressed(unsigned vk)` (declared in `ski_game.h`, defined in `ski_core.c` — Task 13 gives the body). Normally `return (GetKeyState(vk) & 0x8000) != 0;` — the harness swaps its behavior, so the reconstructed game code never calls `GetKeyState` directly.
- [ ] **Step 2: Headless smoke.** Rebuild with `-DSKI_HARNESS=ON`, run under Wine with no input for ~20s:
```bash
cmake --build build-native -j
WINEPREFIX=$HOME/.wine-ski xvfb-run -a -s "-screen 0 1024x768x24" wine build-native/ski.exe
```
Expected: no crash in the first 20s; a skier is descending (screenshot it: `evidence/m2-gamecore.png`).
- [ ] **Step 3: Commit:** `git add src/ evidence/m2-gamecore.png && git commit -m "M2: game core reconstructed (state/physics/modes/collisions/monster)"`.

### Task 12: Reconstruct render, status window, scores, INI, audio

**Files:** extend `src/ski_core.c` (render + `score_*` + `snd_*` functions).

**Source material:** the `draw_*`, `score_*`, `snd_*` functions from Task 5; the Task 6 sprite inventory; NOTES data model.

- [ ] **Step 1: Reconstruct** the `draw_*` functions (exact `BitBlt`/`PatBlt`/`TextOut`/`FillRect`/`FrameRect` call sequences, opcodes, rectangles, palette/`CreateBitmap` usage), the `SkiStatus` content (Time/Dist/Speed/Style via the string-table labels + format strings, `wsprintfA` formats exactly as decompiled), the high-score screen (`High Scores` string, the two markers, INI read/write via `entpack.ini` section/keys from NOTES), and the `snd_*` paths (`sndPlaySoundA` args/flags exactly; `#if SKI_DETERMINISTIC` does not affect sound).
  - The `button`-class windows: reconstruct whatever they do (Task 7 told you what they are) — creation, labels, click handling.
- [ ] **Step 2: Visual check against the original.** Run rebuild and original under Wine with the same seed-patched scenario (use `s02_start` manually: start a run, screenshot both at the same moment, `evidence/m2-render-rebuild.png` vs `evidence/m2-render-original.png`). They should already look nearly identical (diffs = remaining un-reconstructed behavior, which the Task 13 diff suite will quantify).
- [ ] **Step 3: Commit:** `git add src/ evidence/ && git commit -m "M2: render + status + scores + INI + audio reconstructed"`.

### Task 13: Rebuild-side determinism + capture harness

**Files:** modify `src/ski_core.c` (hooks), `src/ski_game.h`; create `harness/gen_input.py`, `harness/cap_x11.sh`, `harness/run_scenario.sh`, `harness/diff.py`.

- [ ] **Step 1: Add the harness hooks to `src/ski_core.c`.** (All under `#if SKI_HARNESS` unless noted; include `"ski_game.h"`.)
```c
/* ---- harness: deterministic input from ski_in.bin (one byte per tick) ----
 * bit layout (fixed): 0=left 1=right 2=crouch 3=mode1 4=mode2 5=mode3
 *                     6=pause  7=advance
 * VK mapping comes from seed.json keys and is generated into ski_keys.h
 * by harness/gen_input.py --emit-header (run before building the harness).
 */
#include "ski_keys.h"
static FILE *hin; static const unsigned char *hbuf; static int hlen;
static void h_input_init(void)
{
    hin = fopen("ski_in.bin", "rb");
    if (!hin) return;
    fseek(hin, 0, SEEK_END); hlen = (int)ftell(hin); fseek(hin, 0, SEEK_SET);
    hbuf = malloc(hlen);
    if (fread((void *)hbuf, 1, hlen, hin) != (size_t)hlen) hbuf = NULL;
}
int ski_key_pressed(unsigned vk)
{
    int bit = -1;
    if (vk == SKI_VK_LEFT)   bit = 0;
    if (vk == SKI_VK_RIGHT)  bit = 1;
    if (vk == SKI_VK_CROUCH) bit = 2;
    if (vk == SKI_VK_MODE1)  bit = 3;
    if (vk == SKI_VK_MODE2)  bit = 4;
    if (vk == SKI_VK_MODE3)  bit = 5;
    if (vk == SKI_VK_PAUSE)  bit = 6;
    if (vk == SKI_VK_ADV)    bit = 7;
#if SKI_HARNESS
    if (!hbuf) h_input_init();
    if (hbuf && g_ski_tick < hlen)
        return (hbuf[g_ski_tick] >> bit) & 1;
    return 0;
#else
    (void)vk; (void)bit;
    return (GetKeyState(vk) & 0x8000) != 0;
#endif
}
```
Note: in the real (non-HARNESS) build `ski_key_pressed(vk)` ignores the bit-table and calls GetKeyState directly (the `#if` above shows both; write it as shown — the `#else` branch uses `vk`). If Task 7 found `WM_KEYDOWN`-based input instead, `ski_key_pressed` becomes the message-derived state read — same signature, different body; record which in NOTES.
- [ ] **Step 2: Add the per-tick framebuffer dump.** In `ski_core.c`:
```c
#if SKI_HARNESS
/* Call after each window's EndPaint:  ski_harness_dump_dc(hwnd, "main"); */
void ski_harness_dump_dc(HWND hwnd, const char *tag)
{
    RECT rc; GetClientRect(hwnd, &rc);
    int w = rc.right, h = rc.bottom;
    BITMAPINFO bi; ZeroMemory(&bi, sizeof bi);
    bi.bmiHeader.biSize = 40;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;        /* top-down 24bpp */
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    unsigned char *px = malloc((size_t)w * h * 3);
    HDC dc = GetDC(hwnd);
    GetDIBits(dc, NULL, 0, (unsigned)h, px, &bi, DIB_RGB_COLORS);
    ReleaseDC(hwnd, dc);
    char name[80];
    snprintf(name, sizeof name, "frame_%06d_%s.ppm", g_ski_tick, tag);
    FILE *f = fopen(name, "wb");
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    fwrite(px, 1, (size_t)w * h * 3, f);
    fclose(f);
    free(px);
}
#endif
```
Add `#if SKI_HARNESS void ski_harness_dump_dc(HWND, const char *); #endif` to `ski_game.h`. At the end of each reconstructed `EndPaint` section in `src/ski_win.c` (the main and status WndProcs), call `ski_harness_dump_dc(hwnd, "main")` / `("status")` under the same guard. (Dumping from the real DC after EndPaint captures exactly what the window showed — independent of how the reconstruction did its blits.)
- [ ] **Step 3: Write `harness/gen_input.py`:**
```python
#!/usr/bin/env python3
"""Scenario JSON -> ski_in.bin (one byte per tick) + ski_keys.h.
Usage: gen_input.py SCENARIO_JSON OUT_BIN [--emit-header src/ski_keys.h]
Bit layout: 0=left 1=right 2=crouch 3=mode1 4=mode2 5=mode3 6=pause 7=advance
Event semantics: {"t":N,"key":"left","down":true} sets the bit from tick N on;
"down":false clears it. A press = down followed by up (two events in scenarios).
"""
import json, pathlib, sys
KEY_BITS = {"left": 0, "right": 1, "crouch": 2, "mode1": 3, "mode2": 4,
            "mode3": 5, "pause": 6, "advance": 7}
sc = json.loads(pathlib.Path(sys.argv[1]).read_text())
seed = json.loads(pathlib.Path("harness/seed.json").read_text())
# scenario key names -> seed.json key names (scenarios use semantic names)
renamed = seed["keys"].get("aliases", {})  # e.g. {"mode1": "VK_S"}
out = bytearray(sc["ticks"])
for ev in sc["events"]:
    k = KEY_BITS[ev["key"]]
    for t in range(ev["t"], sc["ticks"]):
        out[t] |= (1 << k) if ev["down"] else 0
        if not ev["down"]:
            out[t] &= ~(1 << k)
    # (up events clear from their tick; simple forward fill above is enough
    #  because scenarios only use down-then-up pairs with up.t > down.t)
pathlib.Path(sys.argv[2]).write_bytes(bytes(out))
print(f"wrote {sys.argv[2]}: {len(out)} ticks")
if "--emit-header" in sys.argv:
    hdr = pathlib.Path(sys.argv[sys.argv.index("--emit-header") + 1])
    kv = seed["keys"]
    hdr.write_text(
        "/* generated by harness/gen_input.py — do not edit */\n"
        f"#define SKI_VK_LEFT   {kv['left']}\n"
        f"#define SKI_VK_RIGHT  {kv['right']}\n"
        f"#define SKI_VK_CROUCH {kv['crouch']}\n"
        f"#define SKI_VK_MODE1  {kv['modes'][0]}\n"
        f"#define SKI_VK_MODE2  {kv['modes'][1] if len(kv['modes'])>1 else 0}\n"
        f"#define SKI_VK_MODE3  {kv['modes'][2] if len(kv['modes'])>2 else 0}\n"
        f"#define SKI_VK_PAUSE  {kv['pause']}\n"
        f"#define SKI_VK_ADV    {kv['advance']}\n")
    print(f"wrote {hdr}")
```
(Scenario `key` values are the semantic names `left/right/crouch/mode1..3/pause/advance`; `seed.json.keys` holds the real VK constants as C literals — e.g. `"left": "0x25"`. If NOTES' key map uses different semantics, adjust `KEY_BITS` + the scenarios together and record the mapping in NOTES.)
- [ ] **Step 4: Verify a harness run works end to end (single scenario, no original yet).**
```bash
harness/.venv/bin/python harness/gen_input.py harness/scenarios/s02_start.json \
    /tmp/ski_in.bin --emit-header src/ski_keys.h
cmake --build build-native -j   # rebuild with SKI_HARNESS=ON (reconfigure:
                                 # cmake -B build-native-h ... -DSKI_HARNESS=ON)
```
Reconfigure a harness build dir: `cmake -B build-native-h -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw-w64/i686/mingw-windows_i686.cmake -DSKI_DETERMINISTIC=ON -DSKI_HARNESS=ON`. Run: `mkdir -p harness/frames/rebuild_s02 && cd harness/frames/rebuild_s02 && cp /tmp/ski_in.bin . && WINEPREFIX=$HOME/.wine-ski xvfb-run -a -s "-screen 0 1024x768x24" wine <repo>/build-native-h/ski.exe` — wait ~30s, kill, then: `ls | head` shows `frame_000000_main.ppm`, `frame_000000_status.ppm`, …; `harness/.venv/bin/python -c "from PIL import Image; im=Image.open(sorted(__import__('glob').glob('frame_*_main.ppm'))[100]); im.save('/tmp/check.png'); print(im.size)"`. Expected: PPM frames exist, readable by Pillow, window-sized (matches Task 3 geometry). Commit: `git add src/ harness/gen_input.py && git commit -m "M2: rebuild-side deterministic input + per-tick frame dump"`.

### Task 14: Original-side injection (deterministic input for the untouched binary)

The original binary gets a minimal code injection: a `.stub` section containing a `stub_tick` function, hooked as the first instruction of the per-tick update (address from Task 7). Each tick it (once) reads `ski_in.bin` and writes the game's key-state global — exactly as if the WndProc had seen the key messages. The original's own logic, rendering, and CRT are otherwise byte-identical. (If Task 7 found no key-state global, use the fallback: xdotool key events with a `settimer_ms/3` early-margin plus the diff aligner's resync — record that this weakens tick-exactness for that scenario.)

**Files:** create `harness/stub/stubs.c`, `harness/inject_original.py`.

- [ ] **Step 1: Write `harness/stub/stubs.c`:**
```c
/* Injected into ORIGINAL ski32.exe by harness/inject_original.py.
 * Build: i686-w64-mingw32-gcc -m32 -nostdlib -O2 -c stubs.c -o stubs.o
 * All pointers/tables start zero; the patcher fills them from seed.json +
 * decompile/NOTES.md (CRT fopen/fread addrs, key-state global addr, mapping).
 */
typedef int (*FOPEN_T)(const char *, const char *);
typedef int (*FREAD_T)(void *, unsigned, unsigned, void *);
FOPEN_T fopen_p;
FREAD_T fread_p;
unsigned char in_buf[16384];
unsigned in_f, in_idx = 0xFFFFFFFFu, in_count;
const char *path_p, *mode_p;
unsigned *g_keys_p;          /* game's key-state global (NOTES: address + layout) */
unsigned map_table[256];     /* input byte -> g_keys word (patcher-generated) */

void stub_tick(void)
{
    if (in_idx == 0xFFFFFFFFu) {
        in_f = (unsigned)fopen_p(path_p, mode_p);
        in_count = (unsigned)fread_p(in_buf, 1, 16384, (void *)in_f);
        in_idx = 0;
    } else {
        in_idx++;
    }
    *g_keys_p = map_table[in_buf[in_idx]];
}
```
- [ ] **Step 2: Write `harness/inject_original.py`.** Full code:
```python
#!/usr/bin/env python3
"""Inject .stub section + tick hook + seed patch into a copy of ski32.exe.
Usage: inject_original.py OUT_PATH
Reads: original/ski32.exe, harness/seed.json, harness/stub/stubs.o,
       harness/inject.json  (addresses from Task 7 — see shape below)
inject.json shape:
{
  "tick_site":  "0x40XXXX",     # first instr of per-tick update (NOTES)
  "g_keys_addr":"0x40XXXX",     # key-state global (NOTES)
  "crt_fopen":  "0x40XXXX",     # CRT fopen (function-map.json)
  "crt_fread":  "0x40XXXX",     # CRT fread (function-map.json)
  "key_bits":   {"left": 0x..., "right": 0x..., "crouch": 0x...,
                 "mode1": 0x..., "mode2": 0x..., "mode3": 0x...,
                 "pause": 0x..., "advance": 0x...}   # bits in g_keys word
}
"""
import json, pathlib, struct, subprocess, sys

EXE = pathlib.Path("original/ski32.exe")
IMG, FILE_ALIGN, SEC_ALIGN = 0x400000, 0x200, 0x1000

def pe_fields(b):
    pe = struct.unpack_from("<I", b, 0x3c)[0]
    nsec, opt_size = struct.unpack_from("<HH", b, pe + 6)
    opt = pe + 24
    size_of_image = struct.unpack_from("<I", b, opt + 56)[0]
    return pe, nsec, opt_size, size_of_image

def elf_text_and_syms(o):
    e = o.read_bytes()
    shoff = struct.unpack_from("<I", e, 32)[0]
    shentsz, shnum, shstrndx = struct.unpack_from("<HHH", e, 46)
    def sh(i):
        off = shoff + i * shentsz
        name, typ, flags, addr, offset, size = struct.unpack_from("<IIIIII", e, off)
        shstr_off = struct.unpack_from("<I", e, shoff + shstrndx * shentsz + 28)[0]
        end = e.index(b"\0", shstr_off + name)
        return e[shstr_off + name:end].decode(), typ, offset, size
    syms = {}
    for i in range(shnum):
        name, typ, off, size = sh(i)
        if name == ".symtab" and typ == 2:
            stroff = struct.unpack_from("<I", e, off + 32)[0]
            nent, entsz = struct.unpack_from("<HH", e, off + 36)
            for k in range(nent):
                so = off + 40 + k * entsz
                shn, info, val, sz = struct.unpack_from("<BBII", e, so + 4)
                noff = struct.unpack_from("<I", e, so + 12)[0]
                if sz == 0: continue
                nm = e[stroff + noff:e.index(b"\0", stroff + noff)].decode()
                syms[nm] = val
    text = {"text": None, "data": None}
    for i in range(shnum):
        name, typ, off, size = sh(i)
        if name == ".text" and typ == 1: text["text"] = e[off:off+size]
        if name in (".data", ".bss") and typ in (1, 8):
            text["data"] = e[off:off+size] if typ == 1 else b"\0" * size
    return text["text"], text["data"] or b"", syms

def fixup_rel5(b, old_va, new_va):
    """Copy 5 bytes from old_va to new_va, adjusting relative branches/calls."""
    d = old_va - new_va
    out = bytearray(b)
    if b[0] in (0x74, 0x75, 0x76, 0x77, 0x72, 0x73, 0x7c, 0x7d):
        out[1] = (b[1] + d) & 0xFF
    elif b[0] == 0x0F and b[1] in range(0x80, 0x90):
        r = struct.unpack_from("<i", b, 2)[0]
        struct.pack_into("<i", out, 2, r + d)
    elif b[0] in (0xE8, 0xE9):
        r = struct.unpack_from("<i", b, 1)[0]
        struct.pack_into("<i", out, 1, r + d)
    elif b[0] in (0x6A, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x90, 0x31):
        pass  # absolute / register forms
    elif b[0] == 0x68 or b[0] in (0x89, 0x8B, 0x83, 0x81) and len(b) == 5:
        pass  # imm32 / reg forms (prologues)
    else:
        sys.exit(f"UNHANDLED first-5-bytes pattern {b.hex()} at tick site — "
                 f"inspect manually and extend fixup_rel5")
    return bytes(out)

def main():
    out_path = sys.argv[1]
    b = bytearray(EXE.read_bytes())
    seed = json.loads(pathlib.Path("harness/seed.json").read_text())
    inj = json.loads(pathlib.Path("harness/inject.json").read_text())
    pe, nsec, opt_size, soi = pe_fields(bytes(b))
    sec_tab = pe + 24 + opt_size

    # 1) seed patch (same check as seed_patch.py)
    o = seed["original_patch"]
    off = int(o["addr"], 16) - IMG
    before, after = (bytes.fromhex(x.replace(" ", "")) for x in
                     (o["bytes_before"], o["bytes_after"]))
    assert b[off:off+len(before)] == before, "seed bytes mismatch"
    b[off:off+len(before)] = after

    # 2) build stub section payload: [code][data(4-aligned)][strings][tramp]
    code, data, syms = elf_text_and_syms(pathlib.Path("harness/stub/stubs.o"))
    text_base = 0  # stubs.o is PIC-able plain; we place at chosen VA and
    # absolute references in the stub are to PE addresses, so the stub's own
    # data must sit at known VAs — layout:
    code_va  = soi
    data_va  = code_va + len(code) + ((4 - len(code) % 4) % 4)
    code_pad = (4 - len(code) % 4) % 4
    strings  = b"ski_in.bin\0rb\0"
    str_va   = data_va + len(data)
    tramp_va = str_va + len(strings)
    new_size = (tramp_va + 12 - IMG) + SEC_ALIGN
    new_size = (new_size + FILE_ALIGN - 1) & ~(FILE_ALIGN - 1)
    file_off = len(b) + ((FILE_ALIGN - len(b) % FILE_ALIGN) % FILE_ALIGN)

    tick_site = int(inj["tick_site"], 16)
    orig5 = bytes(b[tick_site - IMG: tick_site - IMG + 5])
    payload  = code + b"\0" * code_pad
    payload += data
    payload += strings
    payload += fixup_rel5(orig5, tick_site, tramp_va)
    payload += struct.pack("<i", (tick_site + 5) - (tramp_va + 5))  # E9 rel32
    payload += b"\x90" * (new_size - (tramp_va + 12 - IMG) - (len(payload) - (tramp_va + 12 - IMG)))
    # pad payload to section file size
    payload += b"\0" * (new_size - len(payload))

    # 3) fill stub data (offsets in `data` = symbol value - text_base)
    d = bytearray(data)
    def dat(sym): return syms[sym]
    def put32(sym, val):
        i = dat(sym)
        struct.pack_into("<I", d, i, val)
    put32("fopen_p", int(inj["crt_fopen"], 16))
    put32("fread_p", int(inj["crt_fread"], 16))
    put32("g_keys_p", int(inj["g_keys_addr"], 16))
    put32("path_p", str_va)
    put32("mode_p", str_va + 11)
    kb = inj["key_bits"]
    for byte in range(256):
        word = 0
        for name, bit in kb.items():
            if byte & (1 << {"left":0,"right":1,"crouch":2,"mode1":3,
                             "mode2":4,"mode3":5,"pause":6,"advance":7}[name]):
                word |= bit
        struct.pack_into("<I", d, dat("map_table") + byte * 4, word)
    payload = payload[: (data_va - code_va) ] + bytes(d) + payload[(data_va - code_va) + len(data):]

    # 4) append section header (slack check) + section bytes
    avail = (0x1000 - (sec_tab + (nsec + 1) * 40))
    assert avail >= 0, "no slack for another section header"
    struct.pack_into("<H", b, pe + 6, nsec + 1)
    e = sec_tab + nsec * 40
    b[e:e+8] = b".stub\0\0\0"
    struct.pack_into("<IIIIIIII", b, e + 8, new_size, code_va, new_size, file_off, 0, 0, 0, 0xC0000040)
    struct.pack_into("<I", b, pe + 24 + 56, new_size)  # SizeOfImage
    b += payload
    assert len(b) == file_off + new_size
    pathlib.Path(out_path).write_bytes(bytes(b))
    print(f"wrote {out_path}: .stub @ 0x{code_va:08x}, tick_site hooked, "
          f"g_keys=0x{int(inj['g_keys_addr'],16):08x}")

main()
```
Notes on the code: the `payload` slicing in step 3 splices the filled `data` back in (code/data/strings/tramp layout offsets are `code_va`/`data_va`/… — the splice uses `data_va - code_va` as the offset into the payload; keep these consistent when editing). If `stubs.o` emits a `.rodata` or `.bss` section beyond what `elf_text_and_syms` reads (fread with `-nostdlib -O2` may put `in_idx`'s initializer in `.data`), extend the reader to fold `.rodata` into `code` and zero-fill `.bss` — verify with `i686-w64-mingw32-objdump -h harness/stub/stubs.o` before first run and adjust the section list.
- [ ] **Step 3: Build the stub + write `harness/inject.json`.**
```bash
cd harness/stub && i686-w64-mingw32-gcc -m32 -nostdlib -O2 -c stubs.c -o stubs.o
i686-w64-mingw32-nm -n stubs.o
```
Write `harness/inject.json` from NOTES (tick site, g_keys address + bit layout) and `decompile/ghidra/function-map.json` (CRT `fopen`/`fread` — the CRT functions whose decompiled C does FILE* I/O with the `"rb"` mode string; confirm by disassembling the candidates: they should reference the IAT thunks for `_open`/`_read`).
- [ ] **Step 4: Inject + verify the hook fires.**
```bash
harness/.venv/bin/python harness/inject_original.py /tmp/ski32-inj.exe
mkdir -p /tmp/skiinj && cd /tmp/skiinj
harness/.venv/bin/python <repo>/harness/gen_input.py <repo>/harness/scenarios/s03_steering.json ski_in.bin
WINEPREFIX=$HOME/.wine-ski xvfb-run -a -s "-screen 0 1024x768x24" wine /tmp/ski32-inj.exe
```
Expected: the skier steers exactly per the script (alternating left/right every 100 ticks ≈ 3.3s) — visible in a screenshot (`import -window ... /tmp/skiinj-steer.png`) and, definitively, by running twice and comparing the first frames (identical = deterministic). If the skier doesn't move at all: the g_keys address/bit layout in `inject.json` is wrong (re-derive from NOTES); if it crashes: the trampoline pattern in `fixup_rel5` hit the unhandled case (the script says so) — inspect the tick site's first 5 bytes and extend the fixer.
- [ ] **Step 5: Commit:** `git add harness/stub/ harness/inject_original.py harness/inject.json && git commit -m "M2: original-side tick hook + input injection (verified deterministic)"`.

### Task 15: Frame capture, aligner, differ + the 8-scenario suite (M2 exit)

**Files:** create `harness/cap_x11.sh`, `harness/run_scenario.sh`, `harness/diff.py`.

- [ ] **Step 1: Write `harness/cap_x11.sh`:**
```bash
#!/usr/bin/env bash
# cap_x11.sh OUTDIR MAIN_WID STATUS_WID DURATION_S
# ~10 fps capture of both windows (import ~50-100ms/call).
set -e
mkdir -p "$1"
end=$(( $(date +%s%N) + $4 * 1000000000 ))
i=0
while [ "$(date +%s%N)" -lt "$end" ]; do
  import -window "$2" "$1/cap_$(printf %07d $i)_main.png" 2>/dev/null || true
  import -window "$3" "$1/cap_$(printf %07d $i)_status.png" 2>/dev/null || true
  i=$((i + 1))
  sleep 0.05
done
echo "captured $i frames"
```
`chmod +x harness/cap_x11.sh`
- [ ] **Step 2: Write `harness/run_scenario.sh`:**
```bash
#!/usr/bin/env bash
# run_scenario.sh SCENARIO_NAME
# Runs ORIGINAL (injected) and REBUILD (harness) for the scenario, captures
# frames from both, diffs. Exit 0 = zero-pixel-diff for every tick.
set -e
cd "$(dirname "$0")/.."
S=$1
TICKS=$(harness/.venv/bin/python -c "import json;print(json.load(open('harness/scenarios/$S.json'))['ticks'])")
harness/.venv/bin/python harness/gen_input.py "harness/scenarios/$S.json" /tmp/ski_in.bin
harness/.venv/bin/python harness/inject_original.py /tmp/ski32-inj.exe
DUR=$(( TICKS * 6 / 100 + 30 ))     # generous wall-clock: ticks*6ms (timer+slack) + 30s warmup

# --- original ---
rm -rf harness/frames/orig_$S; mkdir -p harness/frames/orig_$S
cp /tmp/ski_in.bin harness/frames/orig_$S/
( cd harness/frames/orig_$S && WINEPREFIX=$HOME/.wine-ski-noini \
  nohup xvfb-run -a -s "-screen 0 1024x768x24" wine /tmp/ski32-inj.exe > wine.log 2>&1 & echo $! > wine.pid )
MAIN=; for i in $(seq 1 40); do
  MAIN=$(xdotool search --name SkiFree 2>/dev/null | head -1 || true); [ -n "$MAIN" ] && break; sleep 2
done
STATUS=$(xdotool search --name SkiFree | tail -1)
harness/.venv/bin/python - "$MAIN" "$STATUS" "$DUR" <<'EOF' &
import subprocess, sys, time
main, status, dur = sys.argv[1], sys.argv[2], int(sys.argv[3])
end = time.time() + dur
i = 0
while time.time() < end:
    for tag, wid in (("main", main), ("status", status)):
        subprocess.run(["import", "-window", wid,
                        f"harness/frames/orig_{sc}/cap_{i:07d}_{tag}.png"],
                       capture_output=True)
    i += 1
    time.sleep(0.05)
EOF
CAPPID=$!
sleep $(( DUR - 5 ))   # let capture run while the game ticks
kill $(cat harness/frames/orig_$S/wine.pid) 2>/dev/null || true
pkill -f ski32-inj.exe 2>/dev/null || true
wait $CAPPID 2>/dev/null || true

# --- rebuild ---
rm -rf harness/frames/rebuild_$S; mkdir -p harness/frames/rebuild_$S
cp /tmp/ski_in.bin harness/frames/rebuild_$S/
( cd harness/frames/rebuild_$S && WINEPREFIX=$HOME/.wine-ski-noini \
  nohup xvfb-run -a -s "-screen 0 1024x768x24" wine build-native-h/ski.exe > wine.log 2>&1 & echo $! > wine.pid )
for i in $(seq 1 40); do
  [ -f "harness/frames/rebuild_$S/frame_$(printf %06d $TICKS)_main.ppm" ] && break
  sleep 2
done
pkill -f build-native-h/ski.exe 2>/dev/null || true

# --- diff ---
harness/.venv/bin/python harness/diff.py \
  --orig harness/frames/orig_$S --port harness/frames/rebuild_$S \
  --ticks $TICKS --prefix evidence/$S
echo "scenario $S done"
```
(Windows are found by the name `SkiFree` — both binaries use the original's class/title strings. The rebuild loop waits for the LAST expected tick's dump file. If the two `wine` instances fight over X display `:99`, the `xvfb-run -a` auto-increment handles it; `xdotool search` must target the right display — set `DISPLAY` from the captured xvfb log if windows aren't found, or run the two phases sequentially with separate `xvfb-run` invocations as written.)
- [ ] **Step 3: Write `harness/diff.py`:**
```python
#!/usr/bin/env python3
"""Align original (X11 captures) vs rebuild (per-tick dumps) by content hash,
pixel-diff every tick of both windows.
Usage: diff.py --orig DIR --port DIR --ticks N --prefix evidence/sNN
Exit 0 iff every tick matched with 0 differing pixels (both windows).
"""
import argparse, glob, hashlib, sys, pathlib
from PIL import Image

def load(p):
    return Image.open(p).convert("RGB")

def hsh(im):
    return hashlib.md5(im.tobytes()).hexdigest()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--orig"); ap.add_argument("--port")
    ap.add_argument("--ticks", type=int); ap.add_argument("--prefix")
    a = ap.parse_args()
    orig_main = sorted(glob.glob(f"{a.orig}/cap_*_main.png"))
    st_path = sorted(glob.glob(f"{a.orig}/cap_*_status.png"))
    port_main = sorted(glob.glob(f"{a.port}/frame_*_main.ppm")
                       + glob.glob(f"{a.port}/frame_*_main.png"))
    assert len(port_main) == a.ticks, f"expected {a.ticks} port frames, got {len(port_main)}"
    om = [load(p) for p in orig_main]
    oh = [hsh(im) for im in om]
    fails = []
    cursor = 0
    for t in range(a.ticks):
        pm = load(port_main[t])
        ph = hsh(pm)
        found = -1
        for j in range(cursor, len(oh)):
            if oh[j] == ph:
                found = j; break
        if found < 0:
            # try a full resync (input landed one tick off somewhere)
            for j in range(len(oh)):
                if oh[j] == ph:
                    found = j; break
        if found < 0:
            fails.append((t, "no matching original frame"))
            if len(fails) == 1:
                pm.save(f"{a.prefix}_diverge_main_{t:06d}.png")
            continue
        cursor = found
        ps = load(st_path[found])
        po_s = load(port_main[t].replace("_main.ppm", "_status.ppm").replace("_main.png", "_status.png"))
        for name, x, y in (("main", pm, None), ("status", po_s, None)):
            ref = om[found] if name == "main" else ps
            if ref.size != x.size:
                fails.append((t, f"{name} size {x.size} != {ref.size}")); break
            if x.tobytes() != ref.tobytes():
                import PIL.ImageChops as IC
                d = IC.difference(x, ref)
                n = sum(1 for px in d.getdata() if px != (0, 0, 0))
                fails.append((t, f"{name} {n}px differ"))
                if len([f for f in fails if f[0] == t]) == 1:
                    d.save(f"{a.prefix}_diverge_{name}_{t:06d}.png")
    if fails:
        print(f"FAIL {a.prefix}: {len(fails)} failing frames; first: {fails[:3]}")
        sys.exit(1)
    print(f"PASS {a.prefix}: {a.ticks} ticks x 2 windows, 0 differing pixels")

main()
```
(The `ps` computation above is clumsily written twice for clarity of intent — in the actual file, collapse it to: build `st_path = sorted(glob.glob(..._status.png))` once before the loop and use `ps = load(st_path[found])`.)
- [ ] **Step 4: Run the suite.** `chmod +x harness/run_scenario.sh`. Run all 8, in order (fix scenario JSONs as events mis-phase — data fixes, re-run that scenario only):
```bash
for s in s01_menu s02_start s03_steering s04_crouch s05_modes s06_longrun s07_monster s08_pause_scores; do
  harness/run_scenario.sh $s || break
done
```
Expected: `PASS <s>: N ticks x 2 windows, 0 differing pixels` for each. On FAIL: read the first failing frame + the saved divergence PNG; diagnose in this order — (1) capture pipeline (window sizes, X capture timing: re-run the scenario and check whether the SAME frame fails or a different one — non-reproducible = capture issue), (2) scenario event phasing (does the rebuild screenshot at that tick show the input took effect one tick late/early?), (3) reconstruction bug (the divergence is real — fix `src/` against `decompile/`, rebuild, re-run).
- [ ] **Step 5: Commit evidence:** `git add harness/cap_x11.sh harness/run_scenario.sh harness/diff.py evidence/ && git commit -m "M2: 8-scenario frame-diff suite GREEN — rebuild is pixel-identical to original"`.

**M2 exit criterion (project gate):** all 8 scenarios PASS at 0 differing pixels. Do not start M3 until this is green.

---

## M3 — WASM port (Tasks 16–21)

### Task 16: Authoritative shim API list (linker probe)

**Files:** create `shim/API.md`.

- [ ] **Step 1:** Build `src/` alone with emcc (no shim yet) and capture the undefined symbols:
```bash
source ~/.emsdk/emsdk_env.sh
mkdir -p /tmp/skiprobe && cd /tmp/skiprobe
emcc -O2 -c $(ls <repo>/src/*.c | sed "s|^<repo>/||") -o /dev/null 2>&1 || true
emcc -O2 <repo>/src/*.c -o ski.js 2>&1 | tee emcc-errors.txt
```
Expected: `wasm-ld: error: undefined symbol: ...` listing every Win32/GDI/WinMM/KERNEL32 symbol the game calls that emscripten doesn't provide.
- [ ] **Step 2:** Write `shim/API.md`: the exact symbol list from the probe, grouped by subsystem (user/gdi/winmm/kernel), each with a one-line "what it must do" note (from the import table + NOTES). This is the contract Tasks 17–20 implement — the shim implements exactly this list (plus `EMSCRIPTEN_KEEPALIVE` debug hooks from Task 18/21). If the probe shows a symbol you don't recognize, resolve it against `decompile/` before continuing.
- [ ] **Step 3: Commit:** `git add shim/API.md && git commit -m "M3: authoritative shim API list from linker probe"`.

### Task 17: Shim core — types, surfaces, blits (+ host unit tests)

**Files:** create `shim/types.h`, `shim/win32.h`, `shim/surface.h`, `shim/surface.c`, `test/test_surface.c`; modify `CMakeLists.txt` (test target).

- [ ] **Step 1: Write `shim/types.h`** (pure types; no Win32 headers; used by host tests too):
```c
#ifndef SHIM_TYPES_H
#define SHIM_TYPES_H
#include <stdint.h>
typedef void *HWND; typedef void *HDC; typedef void *HBITMAP;
typedef void *HFONT; typedef void *HBRUSH; typedef void *HICON;
typedef void *HCURSOR; typedef void *HINSTANCE; typedef void *HANDLE;
typedef void *HGDIOBJ; typedef uint32_t COLORREF;
typedef struct { int x, y, cx, cy; } POINT, SIZE;
typedef struct { int left, top, right, bottom; } RECT;
typedef struct { int w, h; uint32_t plane; int bpp; } BITMAP;
typedef struct { int cbSize; int w, h, plane, bpp; uint32_t comp; } BITMAPINFOHEADER;
typedef struct { BITMAPINFOHEADER hdr; uint32_t rgb[1]; } BITMAPINFO;
typedef struct { HDC dc; RECT rc; int erase; } PAINTSTRUCT;
typedef struct { const char *name; void (*proc)(HWND, unsigned, unsigned long, long);
                 HBRUSH bg; HCURSOR cur; HICON icon; HINSTANCE inst;
                 int cls_style; const char *title; } WNDCLASSEXA;
#define SRCCOPY   0x00CC0020
#define BLACKNESS 0x00000042
#define WHITENESS 0x000000FF
#define DIB_RGB_COLORS 0
#define WM_NULL 0
#define WM_QUIT 0x0012
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_TIMER 0x0113 WM_PAINT 0x000F
#define WM_CREATE 0x0001 WM_DESTROY 0x0002
#define WM_CLOSE 0x0010 WM_COMMAND 0x0111
#define WM_LBUTTONDOWN 0x0201 WM_LBUTTONUP 0x0202 WM_LBUTTONDBLCLK 0x0203
#define WM_MOUSEMOVE 0x0200
#define WS_VISIBLE 0x10000000
#define IDOK 1 IDCANCEL 2
#define MB_OK 0
#define DTM_NOWARN 1
#endif
```
(Fix the two `#define`s on one line above when transcribing — `WM_NULL`/`WM_QUIT` are separate defines. `WNDCLASSEXA` is the shim's internal class record, not the Windows one — the game's `RegisterClassA` is called with a Windows `WNDCLASSEX`-shaped struct per the decompiled call; `win32.h` declares `RegisterClassA(const WNDCLASSEXW32*)` where `WNDCLASSEXW32` is the real layout (cbSize, style, lpfnWndProc, clsAtom1, clsAtom2, hInstance, hIcon, hCursor, hbrBackground, lpszMenuName, lpszClassName) — add that struct to types.h too, sized to the decompiled call.)
- [ ] **Step 2: Write `shim/surface.h` + `shim/surface.c`** — the DC/bitmap model. Design: a `ShimDC` owns a 32-bit RGBA `uint8_t *px` + w/h; `HDC` is a `ShimDC*`; `HBITMAP` is a `ShimBmp*` (own px + w/h + palette for ≤8bpp); `SelectObject` swaps the DC's "current bitmap" (what BitBlt copies from when the source is a memory DC); `CreateCompatibleBitmap`/`CreateBitmap` (with DIB bits — the 1991 game loads sprites via `LoadBitmapA` resources: those become pre-decoded `ShimBmp`s from `web/assets/sprites` — see Task 20). Core ops:
```c
/* shim/surface.h */
#ifndef SHIM_SURFACE_H
#define SHIM_SURFACE_H
#include "types.h"
HDC  shim_dc_new(int w, int h);
void shim_dc_free(HDC dc);
void shim_dc_size(HDC dc, int *w, int *h);
void shim_dc_fill(HDC dc, int x, int y, int w, int h, COLORREF c);
void shim_dc_blt(HDC dst, int dx, int dy, int w, int h,
                 HDC src, int sx, int sy, uint32_t rop);
void shim_dc_get(const uint8_t *rgba_or_rgb_ptr /*unused*/);
#endif
```
`shim/surface.c` (full):
```c
#include "surface.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int w, h; uint8_t *px; int is_rgb; } ShimDC;   /* px: RGBA unless is_rgb */
typedef struct ShimBmp { int w, h; uint8_t *px; int bpp; uint8_t pal[768]; } ShimBmp;

HDC shim_dc_new(int w, int h)
{
    ShimDC *d = calloc(1, sizeof *d);
    d->w = w; d->h = h; d->px = calloc(1, (size_t)w * h * 4);
    return d;
}
void shim_dc_free(HDC h) { if (h) { free(((ShimDC *)h)->px); free(h); } }
void shim_dc_size(HDC h, int *w, int *hh) { *w = ((ShimDC *)h)->w; *hh = ((ShimDC *)h)->h; }

static uint32_t cget(const uint8_t *px, int x, int y, int w)
{
    const uint8_t *p = px + (size_t)(y * w + x) * 4;
    return (uint32_t)p[2] | ((uint32_t)p[1] << 8) | ((uint32_t)p[0] << 16) | (0xFFu << 24);
}
static void cput(uint8_t *px, int x, int y, int w, uint32_t c)
{
    uint8_t *p = px + (size_t)(y * w + x) * 4;
    p[0] = c & 0xFF; p[1] = (c >> 8) & 0xFF; p[2] = (c >> 16) & 0xFF; p[3] = 0xFF;
}

void shim_dc_fill(HDC h, int x, int y, int w, int hh, COLORREF c)
{
    ShimDC *d = h;
    for (int j = y; j < y + hh; j++)
        for (int i = x; i < x + w; i++)
            if (i >= 0 && j >= 0 && i < d->w && j < d->h)
                cput(d->px, i, j, d->w, c);
}

void shim_dc_blt(HDC dh, int dx, int dy, int w, int h, HDC sh, int sx, int sy, uint32_t rop)
{
    ShimDC *d = dh, *s = sh;
    if (rop == BLACKNESS) { shim_dc_fill(d, dx, dy, w, h, 0); return; }
    if (rop == WHITENESS) { shim_dc_fill(d, dx, dy, w, h, 0xFFFFFF); return; }
    /* SRCCOPY (and the game's other rops resolve to copies — verify the full
     * rop list against the decompiled BitBlt calls in Task 16/18 and extend): */
    for (int j = 0; j < h; j++) {
        int sj = sy + j, dj = dy + j;
        if (sj < 0 || sj >= s->h || dj < 0 || dj >= d->h) continue;
        for (int i = 0; i < w; i++) {
            int si = sx + i, di = dx + i;
            if (si < 0 || si >= s->w || di < 0 || di >= d->w) continue;
            cput(d->px, di, dj, d->w, cget(s->px, si, sj, s->w));
        }
    }
}
```
(Per-pixel C loops are correct-first; if the M3 diff is green but frame times are poor, optimize later — the game is small. 32bpp RGBA storage matches the native rebuild's GetDIBits 24bpp dumps after RGB conversion in `diff.py` — the X11 side and the native side are both 24bpp RGB; the WASM dump (Task 21) exports RGBA→RGB PNGs the same way.)
- [ ] **Step 3: Write `test/test_surface.c`** (host-side, native gcc — proves the core math without a browser):
```c
#include "shim/surface.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    HDC a = shim_dc_new(8, 4);
    HDC b = shim_dc_new(4, 2);
    shim_dc_fill(a, 0, 0, 8, 4, 0x00FFFFFF);          /* white */
    shim_dc_fill(b, 0, 0, 4, 2, 0x000000FF);          /* red   */
    shim_dc_blt(a, 1, 1, 2, 1, b, 0, 0, SRCCOPY);
    const uint8_t *px = ((struct { int w, h; uint8_t *px; int is_rgb; } *)a)->px;
    /* pixel (1,1) must now be red (0,0,255,255), (0,0) white, (3,3) white */
    assert(px[(1 * 8 + 1) * 4 + 2] == 0xFF && px[(1 * 8 + 1) * 4] == 0);
    assert(px[0] == 0xFF && px[1] == 0xFF && px[2] == 0xFF);
    assert(px[(3 * 8 + 3) * 4 + 2] == 0xFF);
    shim_dc_fill(a, 7, 3, 4, 4, 0x00000000);          /* clipped fill, no overflow */
    shim_dc_blt(a, 2, -2, 8, 8, b, -1, -1, SRCCOPY);  /* clipped blit, no overflow */
    shim_dc_free(a); shim_dc_free(b);
    printf("surface tests PASS\n");
    return 0;
}
```
(If the anonymous-struct cast is ugly, expose `void shim_dc_px(const HDC h, const uint8_t **px)` in surface.h and use it — cleaner; adjust the test accordingly.)
- [ ] **Step 4: Wire the test into CMake + run.** Append to `CMakeLists.txt`:
```cmake
if(NOT EMSCRIPTEN)
  enable_testing()
  add_executable(test_surface test/test_surface.c shim/surface.c)
  target_include_directories(test_surface PRIVATE shim)
  add_test(NAME surface COMMAND test_surface)
endif()
```
Run: `cmake -B build-test && cmake --build build-test -j && ctest --test-dir build-test --output-on-failure`. Expected: `surface tests PASS`, ctest 100% passed.
- [ ] **Step 5: Commit:** `git add shim/ test/ CMakeLists.txt && git commit -m "M3: shim core (DC/bitmap/blits) with host unit tests"`.

### Task 18: Shim windows + message pump + timers

**Files:** create `shim/win.c`, `shim/canvas.c`; extend `shim/win32.h` (full API declarations per `shim/API.md`).

- [ ] **Step 1: Write `shim/win32.h`** — declare every symbol from `shim/API.md` with the exact Windows signatures (the game's object files link against these prototypes; signatures must match the MSVC stdcall calling convention — under emscripten everything is cdecl, which is compatible for these ABIs because the callee cleans its own args in both; the risk is signature *shape* (arg order/count), which comes straight from the Windows SDK — copy the canonical signatures).
- [ ] **Step 2: Write `shim/win.c`** (full — the heart of the port):
```c
/* Win32 window/message/timer emulation on emscripten. */
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_js.h>
#include <stdlib.h>
#include <string.h>
#include "win32.h"
#include "surface.h"

#define MAXW 8
typedef struct { int used; const char *cls; const char *title;
                 int x, y, w, h; int visible;
                 void (*proc)(HWND, unsigned, unsigned long, long);
                 HDC dc; HBRUSH bg; } Win;
static Win wins[MAXW];
static HWND g_hmain;                      /* SkiMain */
static unsigned g_now_ms;                 /* virtual GetTickCount */
static int g_timer_period;                /* SetTimer period of the game timer */
static unsigned g_next_tick;              /* virtual-ms of next WM_TIMER */
static int g_tick_id;
static unsigned g_ski_tick;               /* exposed via debug hook */
static int g_quit;

static Win *mkwin(const char *cls, const char *title, int x, int y, int w, int h,
                  void (*proc)(HWND, unsigned, unsigned long, long))
{
    for (int i = 0; i < MAXW; i++) if (!wins[i].used) {
        Win *v = &wins[i];
        v->used = 1; v->cls = cls; v->title = title;
        v->x = x; v->y = y; v->w = w; v->h = h; v->visible = 1;
        v->proc = proc; v->dc = shim_dc_new(w, h);
        /* background: white (the game's class background brush — verify the
         * RegisterClassA hbrBackground value per window in the decompilation
         * and use it here); fill it once now: */
        shim_dc_fill(v->dc, 0, 0, w, h, 0x00FFFFFF);
        return v;
    }
    return NULL;
}

/* ---- message queue ---- */
#define MQCAP 64
typedef struct { HWND h; unsigned msg; unsigned long wp; long lp; } Msg;
static Msg mq[MQCAP]; static int mq_n, mq_r;
static void post(HWND h, unsigned msg, unsigned long wp, long lp)
{ if (mq_n < MQCAP) { mq[mq_n].h = h; mq[mq_n].msg = msg; mq[mq_n].wp = wp; mq[mq_n].lp = lp; mq_n++; } }

`RegisterClassA` stores the class in a table; `CreateWindowExA` looks the WndProc up by class name (the real Win32 does the same — the proc is never passed to CreateWindowEx). The rest of `shim/win.c` (write exactly):
```c
extern void ski_mainloop(void);   /* defined below in this file */
static int g_class_n;
typedef struct { const char *name;
                 void (*proc)(HWND, unsigned, unsigned long, long);
                 HBRUSH bg; } ClassRec;
static ClassRec g_classes[8];
int RegisterClassA(const WNDCLASSEXW32 *c)
{
    g_classes[g_class_n].name = c->lpszClassName;
    g_classes[g_class_n].proc = c->lpfnWndProc;
    g_classes[g_class_n].bg = c->hbrBackground;
    return ++g_class_n;
}
HWND CreateWindowExA(unsigned ex, const char *cls, const char *title,
                     unsigned style, int x, int y, int w, int h,
                     HWND parent, void *menu, HINSTANCE inst, void *param)
{
    (void)ex; (void)parent; (void)menu; (void)inst; (void)param;
    ClassRec *c = NULL;
    for (int i = 0; i < g_class_n; i++) if (!strcmp(g_classes[i].name, cls)) c = &g_classes[i];
    Win *v = mkwin(cls, title, x, y, w, h, c ? c->proc : NULL);
    if (cls && strstr(cls, "Main")) g_hmain = v;
    if (style & WS_VISIBLE) emscripten_set_main_loop(ski_mainloop, 0, 1);
    return v;
}
BOOL FindWindowA(const char *cls, const char *title)
{ (void)title;
  for (int i = 0; i < MAXW; i++)
      if (wins[i].used && (!cls || !strcmp(wins[i].cls, cls))) return &wins[i];
  return 0; }
void MoveWindow(HWND h, int x, int y, int w, int hh, int repaint)
{ Win *v = h; v->x = x; v->y = y; v->w = w; v->h = hh; (void)repaint; }
void SetWindowPos(HWND h, HWND after, int x, int y, int w, int hh, unsigned f)
{ (void)after;
  if (f & 0x0001) { } /* SWP_NOSIZE off: w/h given */
  if (!(f & 0x0002)) MoveWindow(h, x, y, w, hh, !(f & 0x0004));
}
void SetWindowTextA(HWND h, const char *s) { ((Win *)h)->title = s; }
void ShowWindow(HWND h, int how) { (void)h; (void)how; }
void UpdateWindow(HWND h) { (void)h; }
void SetFocus(HWND h) { (void)h; }
void InvalidateRect(HWND h, const RECT *r, int erase) { (void)h; (void)r; (void)erase; return; }
void KillTimer(HWND h, unsigned id) { (void)h; (void)id; }
BOOL SetTimer(HWND h, unsigned id, unsigned ms, void *fn)
{ (void)h; (void)fn; g_tick_id = id; g_timer_period = ms;
  g_next_tick = g_now_ms + ms; return 1; }
unsigned long GetTickCount(void) { return g_now_ms; }
PAINTSTRUCT g_ps;
BOOL BeginPaint(HWND h, PAINTSTRUCT *ps)
{ Win *v = h; ps->dc = v->dc; ps->rc.left = 0; ps->rc.top = 0;
  ps->rc.right = v->w; ps->rc.bottom = v->h; ps->erase = 1; return 1; }
void EndPaint(HWND h, const PAINTSTRUCT *ps) { (void)h; (void)ps; canvas_flush(); }
HDC GetDC(HWND h) { return ((Win *)h)->dc; }
int ReleaseDC(HWND h, HDC dc) { (void)h; (void)dc; return 1; }
BOOL GetClientRect(HWND h, RECT *r)
{ Win *v = h; r->left = 0; r->top = 0; r->right = v->w; r->bottom = v->h; return 1; }
void DefWindowProcA(HWND h, unsigned m, unsigned long wp, long lp)
{ (void)h; (void)m; (void)wp; (void)lp; }
void PostQuitMessage(int code) { (void)code; g_quit = 1; }
int GetMessageA(void *msg, HWND q, unsigned min, unsigned max)
{ (void)msg; (void)q; (void)min; (void)max;
  if (g_quit || mq_r >= mq_n) return 0;
  Msg m = mq[mq_r++];
  /* write into the MSG struct: hwnd(0) msg(4) wParam(8) lParam(12) */
  unsigned *o = msg;
  *o = (unsigned)m.h; o[1] = m.msg; o[2] = m.wp; o[3] = (unsigned)m.lp;
  return 1; }
void TranslateMessage(const void *msg) { (void)msg; }
long DispatchMessageA(const void *msg)
{ const unsigned *o = msg;
  Win *v = (Win *)o[0];
  if (v && v->proc) v->proc(v, o[1], o[2], (long)o[3]);
  return 0; }
void DestroyWindow(HWND h) { Win *v = h; if (v) v->visible = 0; }
int IsIconic(HWND h) { (void)h; return 0; }
HICON LoadIconA(HINSTANCE i, const char *n) { (void)i; (void)n; return (HICON)1; }
HCURSOR LoadCursorA(HINSTANCE i, const char *n) { (void)i; (void)n; return (HCURSOR)1; }
HICON OpenIcon(HICON i) { return i; }
char *GetModuleFileNameA(HINSTANCE i, char *buf, unsigned n)
{ (void)i; strcpy(buf, "ski32.exe"); return buf; }
unsigned GetVersion(void) { return 0x00050001; }  /* verify against Wine in Task 7 */
/* WM_COMMAND for the button windows: boot.js clicks arrive as ski_click(x,y) */
```
And the main loop + debug hooks (end of `shim/win.c`):
```c
/* canvas.c provides: canvas_flush() (draw all windows' framebuffers to the
 * JS canvas), ski_key_event(vk, down), ski_click(x, y), ski_set_input(bytes,
 * len) (WEB_DEBUG: per-tick input array), ski_tick_get(), ski_window_png(n). */
extern void canvas_flush(void);
extern void ski_key_event(unsigned vk, int down);
extern void ski_click(unsigned x, unsigned y);
extern void ski_set_input(const unsigned char *b, int n);
extern int ski_tick_get(void);
extern const char *ski_window_png(int n);

void ski_mainloop(void)
{
    unsigned long now = emscripten_get_now() / 1000.0;
    if (now > g_now_ms) g_now_ms = now;
    int acted = 0;
    while (g_now_ms >= g_next_tick) {
        if (g_ski_tick_dbg_input) { /* WEB_DEBUG per-tick input: set the game's
                                       key state here, mirroring Task 14's
                                       original-side stub (same byte layout) */
        }
        post(g_hmain, WM_TIMER, g_tick_id, 0);
        g_next_tick += g_timer_period;
        g_ski_tick++;
        acted = 1;
    }
    while (mq_r < mq_n) {
        /* pump all queued messages (timer, keys, clicks) */
        void *stack_msg; /* reuse GetMessageA via a local MSG on the stack */
        if (!GetMessageA(&stack_msg, 0, 0, 0)) break;
        TranslateMessage(&stack_msg);
        DispatchMessageA(&stack_msg);
        acted = 1;
    }
    if (acted || g_need_flush) { canvas_flush(); g_need_flush = 0; }
}
```
(Finalize the `MSG` struct usage with a proper `typedef struct { HWND h; unsigned msg; unsigned long wp; long lp; } MSG;` in types.h and pass a stack `MSG` to `GetMessageA` — the sketch's `stack_msg` pointer trick is wrong; write it cleanly with `MSG m; if (!GetMessageA(&m,0,0,0)) break; ...`. The `g_ski_tick_dbg_input` / `g_need_flush` globals live in `canvas.c`.)
- [ ] **Step 3: Write `shim/canvas.c`** — the JS bridge (full):
```c
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>
#include "win32.h"

int g_need_flush = 0;
/* window list for canvas: indices into the same table win.c owns — expose via
 * a small accessor: extern Win *shim_window(int n); extern int shim_window_count(void);
 * (add these two to win.c) */
extern Win *shim_window(int n);
extern int shim_window_count(void);

EMSCRIPTEN_KEEPALIVE void ski_key_event(unsigned vk, int down)
{
    HWND h = shim_window(0);              /* game window gets all keys */
    extern void shim_post(HWND, unsigned, unsigned long, long);
    shim_post(h, down ? WM_KEYDOWN : WM_KEYUP, vk, 0);
}
EMSCRIPTEN_KEEPALIVE void ski_click(unsigned x, unsigned y)
{
    extern void shim_post(HWND, unsigned, unsigned long, long);
    for (int i = 0; i < shim_window_count(); i++) {
        Win *w = shim_window(i);
        if (x >= w->x && x < w->x + w->w && y >= w->y && y < w->y + w->h) {
            shim_post(&w, WM_LBUTTONDOWN, 1, (x - w->x) | ((y - w->y) << 16));
            shim_post(&w, WM_LBUTTONUP, 1, (x - w->x) | ((y - w->y) << 16));
            return;
        }
    }
}
void canvas_flush(void)
{
    /* For every visible window: upload its framebuffer to the JS canvas via
     * emscripten_fill_rgba? Use canvas pixel manipulation:
     *   EM_JS(void, canvas_put, "(canvas, x, y, w, h, ptr)",
     *   """
     *   const gl = canvas.getContext('2d');
     *   const u8 = new Uint8Array(Module.HEAPU8.buffer, ptr, w*h*4);
     *   const img = new ImageData(new Uint8ClampedArray(u8.buffer, 0, w*h*4), w, h);
     *   gl.putImageData(img, x, y);
     *   """);
     * Convert RGBA->RGB(A) as needed; call per window at (w->x, w->y).
     */
}
EMSCRIPTEN_KEEPALIVE int ski_tick_get(void) { extern int g_ski_tick; return g_ski_tick; }
/* ski_window_png(n): render window n's framebuffer to a PNG dataURL via the
 * canvas (putImageData to an offscreen canvas, toDataURL) — WEB_DEBUG only. */
```
(Write the EM_JS bodies concretely per the comments; the `ImageData` + `putImageData` path is the one to use — 2D canvas, no WebGL.)
- [ ] **Step 4: Build check.** `emcmake cmake -B build-web -DSKI_DETERMINISTIC=ON -DSKI_HARNESS=OFF && cmake --build build-web -j`. Expected: links (remaining undefineds = Task 19/20's territory — GDI text/bitmap/INI/audio symbols; if the build still fails, the errors must be exactly the Task 19/20 API set, nothing else).
- [ ] **Step 5: Commit:** `git add shim/ && git commit -m "M3: shim windows + message pump + timers + canvas bridge"`.

### Task 19: Pixel-exact font (capture from the original's own rendering)

The only pixel-exactness threat is text: GDI's font under Wine vs any browser font. Solution: render every character the game can print, using the original's exact font path, under the same Wine, and bake the glyphs + advances into `web/assets/font.json`.

**Files:** create `harness/fontcap/fontcap.c`, `harness/fontcap_parse.py`; generated `web/assets/font.json`; create `shim/text.c`.

- [ ] **Step 1: Determine the font the game uses.** From the decompiled WndProcs/render code (NOTES): does anything call `SelectObject` with a font, `CreateFont` (NOT imported — so no), or use the class's `hFont`? Record the exact font object per window. The capture program must reproduce it (most likely: the class default — `hFont = NULL` → system default).
- [ ] **Step 2: Write `harness/fontcap/fontcap.c`** (mingw-w64, runs under Wine; renders a probe sheet):
```c
/* fontcap.c: render printable ASCII 32..126 in the game's font, plus
 * per-char extents, to fontcap.bmp + fontcap.txt (W x H, then per-char
 * "code w advance" lines). Compile: i686-w64-mingw32-gcc fontcap.c -o fontcap.exe -lgdi32 -luser32
 */
#include <windows.h>
#include <stdio.h>
int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s)
{
    (void)p; (void)c; (void)s;
    WNDCLASSEX wc; ZeroMemory(&wc, sizeof wc);
    wc.cbSize = sizeof wc;
    wc.lpfnWndProc = (WNDPROC)(void *)0; /* never used; we quit before pumping */
    wc.hInstance = i;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "fontcap";
    RegisterClassEx(&wc);
    int W = 95 * 16, H = 16 * 8 + 40;   /* 8 rows of 16px cells + metrics strip */
    HWND h = CreateWindowEx(0, "fontcap", "", WS_VISIBLE, 0, 0, W, H,
                            0, 0, i, 0);
    HDC dc = GetDC(h);
    /* THE GAME'S FONT: set exactly what Task 1 step 1 found, e.g.: */
    HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT); /* or NULL = class default */
    SelectObject(dc, f);
    TEXTMETRICS tm; GetTextMetrics(dc, &tm);
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 95; col++) {
            int code = 32 + row * 95 + col;
            if (code > 126) continue;
            RECT rc = { col * 16, row * 16, col * 16 + 16, row * 16 + 16 };
            FillRect(dc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
            char s2[2] = {(char)code, 0};
            TextOut(dc, col * 16, row * 16, s2, 1);
        }
    /* metrics strip: render "0123456789.:/m s" at (0, 128) and per-char extents */
    char out[512]; int n = 0;
    FILE *fo = fopen("fontcap.txt", "w");
    fprintf(fo, "%d %d %d %d %d\n", W, H, tm.tmHeight, tm.tmAveCharWidth, tm.tmMaxCharWidth);
    for (int code = 32; code < 127; code++) {
        char s2[2] = {(char)code, 0};
        SIZE sz; GetTextExtentPoint32(dc, s2, 1, &sz);
        n += fprintf(fo, "%d %d\n", code, sz.cx);
    }
    fclose(fo);
    /* dump client area to fontcap.bmp (24bpp BMP) */
    BITMAPINFO bi; ZeroMemory(&bi, sizeof bi);
    bi.bmiHeader.biSize = 40; bi.bmiHeader.biWidth = W; bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 24;
    unsigned char *px = malloc((size_t)W * H * 3);
    GetDIBits(dc, 0, 0, (unsigned)H, px, &bi, DIB_RGB_COLORS);
    bi.bmiHeader.biHeight = H;
    FILE *fb = fopen("fontcap.bmp", "wb");
    /* minimal BMP: file header + INFOHEADER + top-down rows (stride padded to 4) */
    int stride = (W * 3 + 3) & ~3;
    unsigned char fh[14] = { 'B', 'M' };
    int dataSz = stride * H, total = 14 + 40 + dataSz;
    *(int *)(fh + 2) = total; *(int *)(fh + 10) = 14 + 40;
    fwrite(fh, 1, 14, fb); fwrite(&bi, 1, 40, fb);
    for (int y = 0; y < H; y++)
        fwrite(px + (size_t)y * W * 3, 1, stride, fb);   /* rows already 4-aligned if W*3 is; pad if not */
    fclose(fb);
    ReleaseDC(h, dc);
    PostQuitMessage(0);
    return 0;
}
```
(If `W*3 % 4 != 0`, pad each row with zeros up to `stride` — `W=1520` → 4560 % 4 == 0, fine as written.)
- [ ] **Step 3: Run + parse.**
```bash
cd harness/fontcap && i686-w64-mingw32-gcc fontcap.c -o fontcap.exe -lgdi32 -luser32
WINEPREFIX=$HOME/.wine-ski xvfb-run -a wine ./fontcap.exe
harness/.venv/bin/python harness/fontcap_parse.py   # -> web/assets/font.json
```
`harness/fontcap_parse.py` (write + run; ~60 lines): read `fontcap.txt` (W, H, tmHeight, tmAve, tmMax, per-char `code advance`), read `fontcap.bmp`, crop each 16×16 cell (rows 0–7), trim nothing (keep full cells — the game's text baseline/box matters for pixel match), store per char: `{"adv": <advance>, "px": base64(rgb rows)}`; also store `{"tmHeight":…, "tmAveCharWidth":…, "tmMaxCharWidth":…}`. Verify a few glyphs visually (read the JSON → save char 'S' as PNG).
- [ ] **Step 4: Write `shim/text.c`** — `TextOutA`, `GetTextExtentPoint32A`, `GetTextMetricsA` from `font.json` (embed via a generated header `shim/font.inc` — add a small step to `fontcap_parse.py`: also emit `shim/font.inc` with the pixel arrays as C `static const`).
```c
/* shim/text.c */
#include <string.h>
#include "win32.h"
#include "surface.h"
extern const ShimFont g_font;   /* from shim/font.inc: cells + advances + metrics */
void TextOutA(HDC h, int x, int y, const char *s, int n)
{
    /* draw char-by-char with the captured glyph cells; text color: the game
     * never calls SetTextColor (not imported) → constant per NOTES (black).
     * Advance x by the captured per-char advance (NOT tmAveCharWidth). */
}
BOOL GetTextExtentPoint32A(HDC h, const char *s, int n, SIZE *sz)
{ (void)h; sz->cx = 0; for (int i = 0; i < n; i++) sz->cx += g_font.adv[(unsigned char)s[i]]; sz->cy = g_font.tmHeight; return 1; }
BOOL GetTextMetricsA(HDC h, TEXTMETRICS *tm)
{ (void)h; /* fill from g_font metrics; zero the rest */ return 1; }
```
(Fill the bodies exactly as commented; add `TEXTMETRICS` + `ShimFont` to types.h/font.inc.)
- [ ] **Step 5: Commit:** `git add harness/fontcap/ harness/fontcap_parse.py web/assets/font.json shim/text.c shim/font.inc && git commit -m "M3: pixel-exact captured font + shim text path"`.

### Task 20: Shim misc — sprites, INI, audio, strings, remaining GDI

**Files:** create `shim/misc.c`; extend `shim/canvas.c` if needed.

- [ ] **Step 1: `LoadBitmapA`/`LoadIconA` resources.** The game loads the 89 bitmaps via `LoadBitmapA(hInst, MAKEINTRESOURCE(id))`. Shim: pre-decode `web/assets/sprites/bmp_NNN.png` at startup into `ShimBmp`s (PNG decode: embed the 89 PNGs as base64 in a generated header `shim/sprites.inc` via a one-off script step in this task — write `harness/embed_sprites.py`: reads `web/assets/resources.json`, base64-encodes each PNG, emits `shim/sprites.inc` with a table `[{id, b64, len}]` + dimensions/bpp; `misc.c` decodes on first use using emscripten's `libpng`? NO extra deps — instead embed the DECODED RGB pixels directly: `harness/embed_sprites.py` decodes with Pillow and emits `static const uint8_t bmp_<id>[] = {...}` RGB rows + `{w,h}`. `LoadBitmapA` returns a `ShimBmp*` wrapping the table entry; `GetObjectA` on it fills the `BITMAP` struct (w/h/bpp); `DeleteObject` is a no-op for table-backed bitmaps (never free const data).
- [ ] **Step 2: INI (entpack.ini).** `GetPrivateProfileStringA`/`WritePrivateProfileStringA` over a JS-persisted string:
```c
/* INI text lives in JS (localStorage "entpack.ini"); C side holds a cached copy.
 * Implement classic INI semantics: [section] lines, key=value, case-insensitive
 * keys/sections, default returned when missing. JS bridge:
 *   EM_JS(const char*, ini_load, "()", "return Module._ini_text || ''")
 *   EM_JS(void, ini_save, "(text)", "Module._ini_text = text; localStorage.setItem('entpack.ini', text)")
 * Load once at startup (call from WinMain wrapper — the shim provides its own
 * _main that calls ini_load() then WinMain); save on every Write*.
 */
```
Write the full parse/serialize (≈80 lines) in `misc.c`.
- [ ] **Step 3: Audio per the M1 answer.** If silent-by-default (expected): `sndPlaySoundA` = no-op that logs to console. If it plays named system sounds: extract the exact WAVs from the Wine prefix used for verification (`find $WINEPREFIX -name '*.wav' | grep -i <name>`), place them in `web/assets/audio/`, and implement `sndPlaySoundA` as WebAudio playback via EM_JS (decode base64-embedded WAV with `AudioContext.decodeAudioData`; `SND_ASYNC` = fire-and-forget; `nosound` condition from NOTES gates playback).
- [ ] **Step 4: Remaining symbols from `shim/API.md`.** Write in `misc.c`: `wsprintfA` (thin `vsnprintf` wrapper — all formats used are snprintf-compatible; handle `%2u`/`%2.2u`/`%5.2d`/`%7ld` natively), `LoadStringA` (embed the exact string-table contents — group 1: `SkiFree`, `Ski Paused ... Press F3 to continue`, `Time:`, `Dist:`, `Speed:`, `Style:`, `00:00:00.00`, ` 0000m`, ` 0000m/s`, `0000000`, `%2u:%2.2u:%2.2u.%2.2u`, `%5.2dm`, `%5.2dm/s`, `%7ld`, `High Scores` (+ the two group-2 markers) — index = resource ID − 1), `MessageBoxA` (console log + `EM_JS` alert), `FrameRect` (3D raised/etched edge per the `HBRUSH` stock value — implement the standard 2-line highlight/shadow), `FillRect` (`shim_dc_fill` with the brush color: WHITE/BLACK/NULL_BRUSH per `GetStockObject` mapping), `GetStockObject` (map: WHITE_BRUSH→white fill, BLACK_BRUSH→black, NULL_BRUSH→transparent, DEFAULT_GUI_FONT→font handle, DC_PEN→dummy), `CreateBitmap`/`CreateCompatibleBitmap` (ShimBmp alloc), `CreateCompatibleDC` (new ShimDC), `DeleteDC`/`DeleteObject`/`SelectObject` (SelectObject: bitmap→set DC current source bitmap; brush/font→track for TextOut color/brush context), `PatBlt` (fill with the DC's pattern brush = solid per GetStockObject), `GetDeviceCaps` (return the values Wine/GDI would: LOGPIXELSX/Y=96, RASTERCAPS bits — only if the decompiled code actually queries them; check NOTES), `GetTextExtentPoint32A`/`GetTextMetricsA`/`TextOutA` (from Task 19).
- [ ] **Step 5: Build + boot smoke.** `cmake --build build-web -j`; serve `web/` (a minimal `web/index.html` placeholder from Task 22 or a bare `<canvas>` + `boot.js` stub that calls `createSki()`); open in headless Chrome: `--headless=new --no-sandbox --use-gl=swiftshader <url>` with console capture. Expected: module loads, main loop runs, `ski_tick_get()` advances (CDP `Runtime.evaluate`), canvas shows the game's title screen (screenshot via `--screenshot`).
- [ ] **Step 6: Commit:** `git add shim/ harness/embed_sprites.py web/assets/ && git commit -m "M3: shim complete — sprites/INI/audio/strings/GDI; web build boots in headless Chrome"`.

### Task 21: WASM-vs-rebuild frame diff (M3 exit)

**Files:** create `harness/wasm_capture.mjs`, `harness/run_scenario_wasm.sh`; modify `harness/diff.py` (port glob accepts PNG).

- [ ] **Step 1: One-line edit to `harness/diff.py`:** change the port glob from `frame_*_main.ppm` to accept both: `port_main = sorted(glob.glob(f"{a.port}/frame_*_main.ppm") + glob.glob(f"{a.port}/frame_*_main.png"))`.
- [ ] **Step 2: Add the WEB_DEBUG input hook (mirrors Task 14).** In `shim/canvas.c`:
```c
static const unsigned char *g_in; static int g_in_n;
EMSCRIPTEN_KEEPALIVE void ski_set_input(const unsigned char *b, int n) { g_in = b; g_in_n = n; }
/* win.c tick loop, before posting WM_TIMER (WEB_DEBUG): write the per-tick
 * input byte into the game's key state — SAME byte layout as Task 13/14:
 * bits 0..7 = left/right/crouch/mode1/mode2/mode3/pause/advance. The game's
 * reconstructed key reads go through ski_key_pressed (Task 11); under
 * SKI_HARNESS that already reads the byte — so the WASM diff build is
 * configured with SKI_HARNESS=ON and ski_set_input feeds the same array the
 * rebuild reads from ski_in.bin. No duplicate mechanism: win.c just ensures
 * g_ski_tick (the shared counter) is the index. */
```
(Concretely: the WASM diff build uses `-DSKI_HARNESS=ON -DSKI_DETERMINISTIC=ON`; `ski_key_pressed` (Task 13) reads `g_in[g_ski_tick]` instead of `fopen` when `g_in` is set — add that one `if (g_in && g_ski_tick < g_in_n) return (g_in[g_ski_tick] >> bit) & 1;` branch to Task 13's `ski_key_pressed`; commit it here.)
- [ ] **Step 3: Write `harness/wasm_capture.mjs`** (Node ≥22, built-in WebSocket; zero deps):
```js
// Usage: node harness/wasm_capture.mjs SCENARIO OUTDIR
// Serves web/ on :8123, drives the game, dumps per-tick window PNGs.
import { spawn } from "node:child_process";
import { writeFileSync, mkdirSync } from "node:fs";
import { readFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const [sc, outdir] = process.argv.slice(2);
mkdirSync(outdir, { recursive: true });
const spec = JSON.parse(readFileSync(path.join(root, "harness/scenarios", sc + ".json"), "utf8"));
const ticks = spec.ticks;
const input = readFileSync(path.join(outdir, "ski_in.bin"));

const http = spawn("python3", ["-m", "http.server", "8123"], { cwd: path.join(root, "web") });
await new Promise(r => setTimeout(r, 800));
const chrome = spawn("chromium", [
  "--headless=new", "--no-sandbox", "--disable-gpu",
  "--disable-background-timer-throttling", "--disable-renderer-backgrounding",
  "--disable-backgrounding-occluded-windows", "--window-size=1024,768",
  "--remote-debugging-port=9333", "about:blank",
], { stdio: "ignore" });
await new Promise(r => setTimeout(r, 1500));
const targets = await (await fetch("http://127.0.0.1:9333/json")).json();
const ws = new WebSocket(targets[0].webSocketDebuggerUrl);
await new Promise(r => (ws.onopen = r));
let id = 0; const pending = new Map();
const send = (method, params) => new Promise(r => {
  const i = ++id; pending.set(i, r);
  ws.send(JSON.stringify({ id: i, method, params }));
});
ws.onmessage = (m) => { const d = JSON.parse(m.data); if (d.id && pending.has(d.id)) { pending.get(d.id)(d); pending.delete(d.id); } };
await send("Page.enable");
await send("Runtime.enable");
await send("Page.navigate", { url: "http://127.0.0.1:8123/index.html" });
await new Promise(r => setTimeout(r, 2500));
const evalJs = async (expr) => (await send("Runtime.evaluate", { expression: expr, awaitPromise: true })).result.result.value;

// feed input array + wait for ready
const b64 = input.toString("base64");
await evalJs(`(async () => {
  if (!window.__ski) return "not-ready";
  const bin = atob("${b64}");
  const u8 = new Uint8Array(bin.length);
  for (let i = 0; i < bin.length; i++) u8[i] = bin.charCodeAt(i);
  window.__ski.ski_set_input(u8);
  return "ok"; })()`);
// pump: for each tick, wait for tick counter then dump both windows
for (let t = 0; t < ticks; t++) {
  const ok = await evalJs(`new Promise(res => {
    const iv = setInterval(() => { if (window.__ski && window.__ski.ski_tick_get() > ${t}) { clearInterval(iv); res(true); } }, 5);
    setTimeout(() => { clearInterval(iv); res(false); }, 5000); })`);
  if (!ok) { console.error(`timeout at tick ${t}`); process.exit(2); }
  for (const w of ["main", "status"]) {
    const n = w === "main" ? 0 : 1;
    const url = await evalJs(`window.__ski.ski_window_png(${n})`);
    if (url && url.startsWith("data:image/png;base64,"))
      writeFileSync(path.join(outdir, `frame_${String(t).padStart(6, "0")}_${w}.png`),
                   Buffer.from(url.split(",")[1], "base64"));
  }
}
console.log(`captured ${ticks} ticks`);
chrome.kill(); http.kill();
process.exit(0);
```
(If the system browser is `google-chrome`/`chromium-browser` instead of `chromium`, adjust the binary name — check `which chromium google-chrome` first. `window.__ski` must be the module object: with `MODULARIZE`+`EXPORT_NAME=createSki`, `boot.js` (Task 22) sets `window.__ski = await createSki()`.)
- [ ] **Step 4: Write `harness/run_scenario_wasm.sh`:**
```bash
#!/usr/bin/env bash
# run_scenario_wasm.sh SCENARIO_NAME  (rebuild frames must already exist from
# harness/run_scenario.sh; this produces the WASM side + the diff)
set -e
cd "$(dirname "$0")/.."
S=$1
cmake --build build-web -j
TICKS=$(harness/.venv/bin/python -c "import json;print(json.load(open('harness/scenarios/$S.json'))['ticks'])")
rm -rf harness/frames/wasm_$S; mkdir -p harness/frames/wasm_$S
harness/.venv/bin/python harness/gen_input.py "harness/scenarios/$S.json" harness/frames/wasm_$S/ski_in.bin
node harness/wasm_capture.mjs $S harness/frames/wasm_$S
harness/.venv/bin/python harness/diff.py \
  --orig harness/frames/rebuild_$S --port harness/frames/wasm_$S \
  --ticks $TICKS --prefix evidence/$S-wasm
echo "wasm scenario $S done"
```
Note the diff direction here: the REBUILD (per-tick PPMs, exactly one per tick) is the stable reference, the WASM captures are aligned to it. (Rebuild frames are tick-exact by construction, so alignment is trivial — the hash aligner still applies as a safety net.)
- [ ] **Step 5: Run all 8.** `for s in s01_menu s02_start s03_steering s04_crouch s05_modes s06_longrun s07_monster s08_pause_scores; do harness/run_scenario_wasm.sh $s || break; done`. Expected: `PASS ... 0 differing pixels` × 8. On FAIL: divergence PNG first; suspect order — (1) canvas upload (window position/size in `canvas_flush` vs the rebuild's window geometry), (2) font glyphs (a specific character diverging = font.json issue), (3) blit/rop difference, (4) timing (WASM tick rate vs Wine — if frames are systematically shifted, the rAF scheduler in `win.c` is drifting: pin ticks to a fixed real-time cadence as designed and re-check).
- [ ] **Step 6: Commit:** `git add harness/wasm_capture.mjs harness/run_scenario_wasm.sh harness/diff.py shim/ && git commit -m "M3: WASM-vs-rebuild frame diff suite GREEN"`.

**M3 exit criterion:** all 8 scenarios PASS (WASM vs rebuild). By transitivity (M2 already proved rebuild == original), the WASM port is pixel-identical to the original.

---

## M4 — Ship (Tasks 22–24)

### Task 22: Browser app (index.html + boot.js)

**Files:** create `web/index.html`, `web/boot.js`.

- [ ] **Step 1: Write `web/index.html`** (full):
```html
<!doctype html>
<html><head><meta charset="utf-8"><title>SkiFree</title>
<style>
  html,body{margin:0;height:100%;background:#111;display:flex;flex-direction:column;
            align-items:center;justify-content:center;font-family:monospace}
  #wrap{position:relative;line-height:0}
  canvas{image-rendering:pixelated;background:#fff}
  #bar{color:#ccc;margin:8px;font-size:13px}
  select{margin:0 6px}
</style></head><body>
<div id="bar">SkiFree 1.04 — decompiled WASM port
  <select id="scale">
    <option value="1">1×</option><option value="2" selected>2×</option><option value="3">3×</option>
  </select></div>
<div id="wrap"><canvas id="c"></canvas></div>
<script src="assets/ski.js"></script>
<script src="boot.js"></script>
</body></html>
```
- [ ] **Step 2: Write `web/boot.js`** (full):
```js
// Boot the module, size the canvas to the game's two windows (canvas =
// the union bounding box of SkiMain + SkiStatus at their original offsets —
// values from evidence/m0-geometry.txt; hardcode as CANVAS_W/H below),
// map keyboard, unlock audio, persist INI via localStorage (shim does it).
(async () => {
  const CANVAS_W = <main_x+main_w+slack>, CANVAS_H = <union height>;  // from Task 3 geometry
  const c = document.getElementById("c");
  const scale = () => +document.getElementById("scale").value;
  const fit = () => { c.width = CANVAS_W * scale(); c.height = CANVAS_H * scale(); };
  fit(); document.getElementById("scale").onchange = fit;

  window.__ski = await createSki({ canvas: c });
  // the shim draws windows at their native (x,y); the JS canvas is scaled via
  // CSS-free direct pixel write, so canvas_flush must scale — implement the
  // scale factor inside the EM_JS putImageData path (ImageSmoothing off).

  const VK = { ArrowLeft: 0x25, ArrowRight: 0x27, ArrowUp: 0x26, ArrowDown: 0x28,
               Space: 0x20, KeyA: 0x41, KeyD: 0x44, KeyS: 0x53, KeyF: 0x46,
               Enter: 0x0D, Escape: 0x1B, F3: 0x73, KeyM: 0x4D };
  window.addEventListener("keydown", e => {
    const vk = VK[e.code];
    if (vk !== undefined) { e.preventDefault(); window.__ski.ski_key_event(vk, 1); }
  });
  window.addEventListener("keyup", e => {
    const vk = VK[e.code];
    if (vk !== undefined) { e.preventDefault(); window.__ski.ski_key_event(vk, 0); }
  });
  c.addEventListener("mousedown", e => {
    const r = c.getBoundingClientRect();
    window.__ski.ski_click(Math.floor((e.clientX - r.left) / scale()),
                           Math.floor((e.clientY - r.top) / scale()));
  });
  window.addEventListener("keydown", () => {
    if (window.__ski.ski_audio_unlock) window.__ski.ski_audio_unlock();
  }, { once: true });
  document.body.addEventListener("click", () => {
    if (window.__ski.ski_audio_unlock) window.__ski.ski_audio_unlock();
  }, { once: true });
})();
```
(The `VK` map: keep 1:1 with the keys the game actually uses per NOTES — remove unused entries, add any NOTES lists (e.g. letter-steering variants). The two-argument `createSki({canvas})` must be matched in `shim/win.c`'s `emscripten_set_main_loop`/canvas init — the EM_JS `canvas` reference resolves to this element; wire it in Task 20's `canvas_flush` if not already.)
- [ ] **Step 3: Manual play check** (real browser, not headless): serve `web/` (`python3 -m http.server 8030` from the repo, open `/web/`), play through: title → start → steer/crouch → pause (F3) → resume → die → high-score entry. Expected: everything behaves like the original (you've seen the original in Task 3; it should feel identical). Screenshot the running game → `evidence/m4-play.png`.
- [ ] **Step 4: Commit:** `git add web/ evidence/m4-play.png && git commit -m "M4: browser app — play SkiFree in the browser"`.

### Task 23: Full verification matrix + README

**Files:** create `README.md`; evidence updates.

- [ ] **Step 1: Re-run everything clean.**
```bash
# native rebuild vs original (M2 gate)
for s in s01_menu s02_start s03_steering s04_crouch s05_modes s06_longrun s07_monster s08_pause_scores; do harness/run_scenario.sh $s; done
# wasm vs rebuild (M3 gate)
for s in s01_menu s02_start s03_steering s04_crouch s05_modes s06_longrun s07_monster s08_pause_scores; do harness/run_scenario_wasm.sh $s; done
```
Expected: 16 PASS lines. Record the output verbatim in `evidence/final-verification.txt`.
- [ ] **Step 2: Write `README.md`** with exactly these sections (fill from reality, no placeholders):
  - What this is (decompilation + pixel-perfect WASM port of SkiFree 1.04 32-bit; private project — rights note: original by Chris Pirih, Microsoft Entertainment Pack 3 1991; this repo decompiles his 2005 32-bit build; not for redistribution).
  - Layout (the tree from the spec).
  - Build (native: the cmake commands; web: emcmake commands; prerequisites from `harness/TOOLCHAIN.md`).
  - Run (`python3 -m http.server` + URL; scale selector; key list from NOTES).
  - Decompilation (Ghidra version, function counts, where the annotated C lives, the three M1 answers with one-line summaries).
  - Verification (the staged-diff design in three sentences; the 16-scenario result table; how to re-run the suite).
  - Known deltas (anything recorded in `evidence/` during the diff debugging; the text-font provenance: captured from Wine-rendered glyphs; GetVersion value; anything M1 forced).
- [ ] **Step 3: Commit:** `git add README.md evidence/ && git commit -m "M4: full verification matrix green + README"`.

### Task 24: Final cleanup + closeout

- [ ] **Step 1:** Remove scratch: `/tmp/ski*` files (outside repo — just kill any lingering wine/xvfb: `pkill -f ski32 || true; pkill -f ski.exe || true`), confirm `git status` is clean, confirm `harness/frames/` is not tracked (`git ls-files harness/ | grep frames` → empty).
- [ ] **Step 2:** Final diff of the three deliverables against the spec's acceptance criteria (1: `decompile/` complete + three answers; 2: M2 green; 3: M3 green; 4: playable with persisting high scores). Note any deviation in the commit message.
- [ ] **Step 3:** `git add -A && git commit -m "SkiFree decompilation + pixel-perfect WASM port complete"` — done.

---

## Plan self-review (run by plan author)

1. **Spec coverage:** decompilation deliverable → Tasks 4–7 ✓; native rebuild + proof → 9–15 ✓; shim + port → 16–21 ✓; browser app → 22 ✓; verification matrix + docs → 23–24 ✓; max-fidelity/0-pixel bar → ground rules + Task 15/21 exits ✓; private/no-contact decision → README rights note (Task 23) ✓; out-of-scope honored (no CI/hosting/mobile tasks) ✓.
2. **Placeholder scan:** the two places the plan references *runtime-discovered* values (Ghidra addresses, VK codes, window sizes) are always produced by an earlier task into a named file (`decompile/NOTES.md`, `harness/seed.json`, `harness/inject.json`, `evidence/m0-geometry.txt`) before the task that consumes them — data dependencies, not gaps.
3. **Type/name consistency:** `ski_key_pressed` (T11/T13), `ski_harness_dump_dc` (T13), `stub_tick` (T14), `ski_window_png`/`ski_tick_get`/`ski_set_input`/`ski_key_event`/`ski_click` (T18/T21/T22), `g_ski_tick` (T9–T21), frame naming `frame_%06d_{main,status}.{ppm,png}` (T13/T15/T21) — used identically across tasks.
