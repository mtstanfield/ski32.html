// harness/wasm_capture.mjs — WASM side of the T21 frame diff (M3 exit).
//
// Usage: node harness/wasm_capture.mjs SCENARIO OUTDIR
//   OUTDIR/ski_in.bin must already exist (harness/gen_input.py output).
//
// Serves web/ on :8123, drives the HARNESS=ON WASM build in headless
// Chrome over CDP (Node >= 22, zero deps — the T20 probe20.mjs pattern):
//
//   1. navigate index.html?nopump=1 — boot.js's pump opt-out, so the
//      input is installed BEFORE the first tick (the SKI_HARNESS input
//      reader caches its ready-state at tick 1, src/ski_core.c
//      ski_harness_ready; a pre-input tick would leave the harness
//      inert for the whole run);
//   2. _ski_set_input: the u16-LE word stream is _malloc'd + copied via
//      HEAPU8 (NOT passed as a TypedArray — emscripten frees the
//      glue's temp copy after the call, which would let a later
//      _malloc clobber g_ski_in mid-run) and _ski_start_pump;
//   3. per poll (~12 ms): one CDP eval reads [_ski_tick_get,
//      _ski_frame_get, _ski_messagebox_get] and pulls every newly
//      sealed frame as a PNG dataURL (C-buffer pattern, _malloc/
//      UTF8ToString/_free); writes frame_%06d_main.png;
//   4. stop at tick T+1 (a T-word scenario runs T+1 ticks; frame N is
//      sealed at tick N+1 start — the rebuild's frame_N capture point,
//      so shift 0), or on a 3 s tick stall (s08: the F3 pause kills
//      the timer at tick 301 and both sides stop at 301 frames), or
//      on the total wall-clock budget.
//
// Modal safety: no scenario is expected to raise a MessageBoxA
// (s08 was confirmed modal-free — it is an F3-pause stall). Any box
// means a divergence from that assumption: fail loudly (exit 3) with
// the box type/caption/text.
//
// Determinism: the shim's virtual clock advances exactly one period
// per timer dispatch (shim/win.c), so the sealed frames are a pure
// function of the tick history — independent of the CDP pull timing.
// A fresh --user-data-dir per run keeps localStorage (the INI) clean.
import { spawn } from "node:child_process";
import { writeFileSync, mkdirSync, readFileSync, rmSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const [sc, outdir] = process.argv.slice(2);
if (!sc || !outdir) {
  console.error("usage: node harness/wasm_capture.mjs SCENARIO OUTDIR");
  process.exit(2);
}
const OUT = path.isAbsolute(outdir) ? outdir : path.join(process.cwd(), outdir);
mkdirSync(OUT, { recursive: true });
const spec = JSON.parse(readFileSync(path.join(root, "harness/scenarios", sc + ".json"), "utf8"));
const T = spec.ticks;
const input = readFileSync(path.join(OUT, "ski_in.bin"));
if (input.length !== T * 2) {
  console.error(`bad ski_in.bin: ${input.length} bytes != ${T} ticks x 2`);
  process.exit(2);
}
const TARGET = T + 1; // T+1 ticks: frame N is sealed during tick N+1

const HTTP_PORT = 8123;
const DBG_PORT = 9333;
const UDD = `/tmp/ski-wasm-${sc}-${process.pid}`;
rmSync(UDD, { recursive: true, force: true });

// A stale headless Chrome on the debug port would silently poison this run
// (CDP attaches to the wrong browser — seen once: a previous run's tree
// survived chrome.kill). Fail loudly rather than pkill by guesswork.
try {
  await (await fetch(`http://127.0.0.1:${DBG_PORT}/json`)).json();
  console.error(`debug port ${DBG_PORT} is already in use — kill the stale headless Chrome (pkill -f remote-debugging-port=${DBG_PORT}) and re-run`);
  process.exit(2);
} catch {}
// Same for the file-server port: an occupant serving a FOREIGN web/ would
// otherwise slow-fail the 30 s ready check. (python http.server self-exits
// when its own spawn hits a taken port, so this only catches foreign or
// same-dir orphans — the latter is harmless, this is just instant.)
try {
  await fetch(`http://127.0.0.1:${HTTP_PORT}/`);
  console.error(`file-server port ${HTTP_PORT} is already in use — kill the stale server (pkill -f "http.server ${HTTP_PORT}") and re-run`);
  process.exit(2);
} catch {}

const http = spawn("python3", ["-m", "http.server", HTTP_PORT], {
  cwd: path.join(root, "web"), stdio: "ignore",
});
const chrome = spawn("/usr/bin/google-chrome", [
  "--headless=new", "--no-sandbox", "--use-gl=swiftshader",
  "--disable-background-timer-throttling", "--disable-renderer-backgrounding",
  "--disable-backgrounding-occluded-windows",
  "--window-size=1024,768", `--remote-debugging-port=${DBG_PORT}`,
  `--user-data-dir=${UDD}`, "about:blank",
], { stdio: "ignore", detached: true });
const cleanup = (code) => {
  // detached: true => chrome leads its own process group; killing -pid
  // reaps the headless tree (zygote/GPU/renderer) that chrome.kill()
  // alone would leave alive on the debug port.
  try { process.kill(-chrome.pid, "SIGKILL"); }
  catch { try { chrome.kill("SIGKILL"); } catch {} }
  try { http.kill("SIGKILL"); } catch {}
  process.exit(code);
};
const sleep = (ms) => new Promise(r => setTimeout(r, ms));
const deadline = Date.now() + (T * 40 * 3) / 2 + 180_000; // 1.5x cadence + 180 s

// ---- CDP (built-in WebSocket; the probe20.mjs pattern) -------------------
let dbgWs = null;
for (let i = 0; i < 50; i++) {
  await sleep(200);
  try {
    const targets = await (await fetch(`http://127.0.0.1:${DBG_PORT}/json`)).json();
    const page = targets.find(t => t.type === "page");
    if (page) { dbgWs = new WebSocket(page.webSocketDebuggerUrl); await new Promise(r => (dbgWs.onopen = r)); break; }
  } catch {}
}
if (!dbgWs) { console.error("chrome did not open a debug target"); cleanup(2); }

let id = 0; const pending = new Map();
const consoleMsgs = []; const exceptions = [];
dbgWs.onmessage = (m) => {
  const d = JSON.parse(m.data);
  if (d.id && pending.has(d.id)) { pending.get(d.id)(d); pending.delete(d.id); return; }
  if (d.method === "Runtime.consoleAPICalled")
    consoleMsgs.push(d.params.args.map(a => a.value ?? a.description ?? "").join(" "));
  if (d.method === "Runtime.exceptionThrown")
    exceptions.push(JSON.stringify(d.params.exceptionDetails.exception?.description ?? d.params.exceptionDetails.text));
};
dbgWs.onerror = (e) => { console.error("cdp socket error", e?.message ?? ""); cleanup(2); };
const send = (method, params) => new Promise((r, j) => {
  const i = ++id; pending.set(i, (d) => d.error ? j(new Error(d.error.message)) : r(d));
  dbgWs.send(JSON.stringify({ id: i, method, params }));
});
const evalJs = async (expr) => {
  const r = await send("Runtime.evaluate",
    { expression: expr, awaitPromise: true, returnByValue: true, timeout: 30000 });
  return r.result?.result?.value;
};
await send("Page.enable");
await send("Runtime.enable");

// ---- boot with nopump, install input, start the pump ----------------------
await send("Page.navigate", { url: `http://127.0.0.1:${HTTP_PORT}/index.html?nopump=1` });
let ready = false;
for (let i = 0; i < 150 && !ready; i++) {
  await sleep(200);
  ready = await evalJs("!!(window.__ski && window.__ski._ski_frame_get)");
}
if (!ready) {
  console.error("module not ready after 30 s", exceptions.slice(0, 3));
  console.error(consoleMsgs.slice(-10).join("\n"));
  cleanup(2);
}
if (exceptions.length) {
  console.error("page exceptions at boot:", exceptions.slice(0, 3));
  cleanup(2);
}

// C-buffer input: _malloc + HEAPU8.set (the TypedArray glue copy is
// freed after the call — see the header).
const b64 = input.toString("base64");
const nset = await evalJs(`(() => {
  const m = window.__ski;
  const b64 = "${b64}";
  const bin = atob(b64);
  const u8 = new Uint8Array(bin.length);
  for (let i = 0; i < bin.length; i++) u8[i] = bin.charCodeAt(i);
  const p = m._malloc(u8.length);
  m.HEAPU8.set(u8, p);
  m._ski_set_input(p, u8.length);
  return u8.length;
})()`);
if (nset !== T * 2) { console.error(`ski_set_input returned ${nset}, want ${T * 2}`); cleanup(2); }

// page-side pull state: frames already handed to Node
await evalJs("window.__t21_pulled = 0; true");
await evalJs("window.__ski._ski_start_pump(); true");

// ---- capture loop ---------------------------------------------------------
let pulled = 0, lastTick = -1, stallAt = 0;
const CAP = 200000; // dataURL of the 760x734 client ~35 KB (T20 probe)
while (true) {
  if (Date.now() > deadline) {
    console.error(`TIMEOUT: tick ${lastTick}/${TARGET}, ${pulled} frames`);
    cleanup(2);
  }
  let r;
  try {
    r = await evalJs(`(() => {
      const m = window.__ski;
      const s = m._ski_frame_get();
      const out = [];
      if (s > window.__t21_pulled) {
        const p = m._malloc(${CAP});
        for (let f = window.__t21_pulled; f < s; f++) {
          const len = m._ski_frame_pull(f, p, ${CAP});
          if (!len) { m._free(p); return JSON.stringify({ pull_fail: f, s }); }
          out.push(m.UTF8ToString(p));
        }
        m._free(p);
        window.__t21_pulled = s;
      }
      return JSON.stringify({ t: m._ski_tick_get(), s, b: m._ski_messagebox_get(), f: out });
    })()`);
  } catch (e) {
    console.error("eval failed:", e.message, exceptions.slice(0, 2));
    cleanup(2);
  }
  const st = JSON.parse(r);
  if (st.pull_fail !== undefined) {
    console.error(`frame ${st.pull_fail} pull failed (seal n=${st.s})`);
    cleanup(2);
  }
  for (const url of st.f) {
    const i = pulled++;
    if (!url.startsWith("data:image/png;base64,")) {
      console.error(`frame ${i}: bad dataURL ${String(url).slice(0, 40)}`);
      cleanup(2);
    }
    writeFileSync(path.join(OUT, `frame_${String(i).padStart(6, "0")}_main.png`),
                  Buffer.from(url.split(",")[1], "base64"));
  }
  if (st.b) {
    const box = await evalJs(`(() => {
      const m = window.__ski;
      return JSON.stringify({
        type: m._ski_messagebox_type(),
        caption: m.UTF8ToString(m._ski_messagebox_caption()),
        text: m.UTF8ToString(m._ski_messagebox_text())
      });
    })()`);
    console.error(`MODAL SAFETY: a MessageBoxA box is up (no scenario may raise one): ${box}`);
    cleanup(3);
  }
  if (st.t >= TARGET) {
    lastTick = st.t;
    break;
  }
  if (st.t === lastTick) {
    if (!stallAt) stallAt = Date.now();
    // 3 s without a tick: the game paused (timer disarmed) — the run
    // ends where the rebuild's stall detector ends it (s08: tick 301).
    if (Date.now() - stallAt > 3000)
      break;
  } else {
    lastTick = st.t;
    stallAt = 0;
  }
  await sleep(12);
}

// ---- verify + report ------------------------------------------------------
// Invariant: exactly one seal per completed tick (frame N seals at the top
// of the (N+1)th ski_tick call, src/ski_core.c ski_harness_step), so at
// break time pulled === lastTick. (The earlier Math.min(pulled,
// lastTick+1) comparison was a tautology — pulled <= lastTick+1 always.)
if (pulled !== lastTick) {
  console.error(`frame count ${pulled} != tick count ${lastTick}`);
  cleanup(2);
}
console.log(JSON.stringify({
  scenario: sc, ticks: lastTick, target: TARGET,
  frames: pulled,
  note: lastTick < TARGET ? "stalled (pause) — expected for s08" : null,
  exceptions,
}));
cleanup(0);
