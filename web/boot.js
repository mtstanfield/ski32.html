/* SkiFree web boot (Task 20 smoke; T22 polishes).
 *
 * Loads the emscripten module (build output: build-web/ski.js, copied to
 * web/ for the smoke — see .gitignore) and parks it on window.__ski,
 * the surface the harness drives (API.md "Debug hooks" — exports carry
 * the emscripten `_` prefix):
 *   __ski._ski_tick_get()            ticks completed
 *   __ski._ski_key_event(vk, down)   interactive keys
 *   __ski._ski_click(x, y)           canvas clicks
 *   __ski._ski_window_png(n, ptr, cap)  PNG dataURL of window n's client,
 *                                       written UTF-8 into buf ptr (cap
 *                                       bytes); returns length (0 = error).
 *                                       buf = __ski._malloc(cap); read it
 *                                       back with __ski.UTF8ToString(ptr)
 *   __ski._ski_messagebox_get()      1 while a MessageBoxA box is up
 *   __ski._ski_messagebox_type()     the box's UINT type
 *   __ski._ski_messagebox_text()     box text
 *   __ski._ski_messagebox_caption()  box caption
 *   __ski._ski_messagebox_answer(r)  answer the box (IDOK 1 / IDCANCEL 2)
 */
"use strict";

/* ?nopump=1 (T21 harness): do NOT start the pump here — the harness
 * mjs installs the per-tick input (_ski_set_input) BEFORE the first
 * tick, then starts the pump itself (_ski_start_pump). Without the
 * flag the boot path is unchanged (T20: auto-pump after createSki
 * resolves). The flag matters because the SKI_HARNESS input reader
 * caches its ready-state on the first tick (src/ski_core.c
 * ski_harness_ready): a tick that ran before ski_set_input would
 * leave the harness inert for the whole run. */
const nopump = new URLSearchParams(location.search).has("nopump");

window.__ski_ready = createSki().then((mod) => {
  window.__ski = mod;
  /* The shim's rAF pump starts here, after main() (WinMain) returned:
     emscripten_set_main_loop inside main() unwinds the C stack
     (simulate_infinite_loop handoff) and the rest of boot never runs
     (shim/win.c notes). By now boot is complete: windows created,
     40 ms game timer armed, main window dirty. */
  if (!nopump)
    mod._ski_start_pump();
  return mod;
});

/* Browser input (minimal wiring — T22 polishes the page).
 *
 * Keyboard: the game has NO WM_KEYUP path — each WM_KEYDOWN is one
 * discrete state transition (ski_win.c:296 switch), and Windows
 * auto-repeat of a held key arrives as repeated WM_KEYDOWNs, so the
 * browser keydown repeats are passed straight through: hold an arrow
 * key to keep steering, exactly as under Windows.
 *
 * Escape is deliberately NOT mapped: in the original it only
 * minimizes the window (ShowWindow(6) -> shim win.c SW_MINIMIZE:
 * visible=0, canvas drops the window, c770 auto-pause) — under a
 * browser there is no Alt-Tab to restore it, so Esc would vanish the
 * game. Pause is F3, which the game fully supports.
 *
 * VK map (ski_win.c:251-340): the game accepts arrows OR the original
 * numpad layout (Numpad0 crouch, Numpad1/3/7/9 facing, Numpad4/6
 * steer, Numpad8/2 up/down), Home/End/PageUp/PageDown, '-' or Insert
 * (both 0x2d) for crouch, F2 restart, F3 pause, Enter advance.
 *
 * Letters A-Z map to their VKs (uppercase ASCII — 'KeyF' -> 0x46):
 * ski_key_event posts WM_KEYDOWN + WM_CHAR, and the char handler
 * (ski_char_key, ski_win.c:399-420) is where letters act — 'f' toggles
 * turbo (2x movement deltas, ski_core.c:828 — the original's "F key
 * speeds things up"), plus the debug set x/X/y/y teleport, r render,
 * t manual tick; all other letters are no-ops, as in the original. */
const SKI_KEY_MAP = {
  "ArrowLeft": 0x25, "ArrowUp": 0x26, "ArrowRight": 0x27, "ArrowDown": 0x28,
  "Numpad0": 0x60, "Numpad1": 0x61, "Numpad2": 0x62, "Numpad3": 0x63,
  "Numpad4": 0x64, "Numpad5": 0x65, "Numpad6": 0x66, "Numpad7": 0x67,
  "Numpad8": 0x68, "Numpad9": 0x69,
  "Home": 0x24, "End": 0x23, "PageUp": 0x21, "PageDown": 0x22,
  "Minus": 0x2d, "Insert": 0x2d,
  "F2": 0x71, "F3": 0x72, "Enter": 0x0d,
};
window.addEventListener("keydown", (e) => {
  const mod = window.__ski;
  let vk = SKI_KEY_MAP[e.code];
  if (vk === undefined && e.code.length === 4 &&
      e.code.charCodeAt(0) === 75 && e.code.charCodeAt(1) === 101 &&
      e.code.charCodeAt(2) === 121 && /* "KeyA".."KeyZ" */
      e.code.charCodeAt(3) >= 65 && e.code.charCodeAt(3) <= 90)
    vk = e.code.charCodeAt(3);
  if (!mod || vk === undefined)
    return;
  e.preventDefault(); /* arrows/page keys would scroll the page */
  mod._ski_key_event(vk, 1);
});

/* Mouse: canvas coords are reference-screen space (1024x768); the
 * rect scale keeps it correct if the page ever CSS-scales the canvas.
 * ski_click (shim/canvas.c) posts WM_LBUTTONDOWN/UP to the topmost
 * window whose client contains the point (a click with no player
 * restarts; with a player it jumps — ski_win.c:456-460). ski_mouse_move
 * posts WM_MOUSEMOVE — the original's mouse steering: the skier faces
 * toward the cursor relative to the client center (ski_mouse_aim,
 * ski_win.c:431-449). */
const canvas = document.getElementById("canvas");
const canvasPt = (e) => {
  const r = canvas.getBoundingClientRect();
  return [
    Math.round((e.clientX - r.left) * (canvas.width / r.width)),
    Math.round((e.clientY - r.top) * (canvas.height / r.height)),
  ];
};
canvas.addEventListener("mousedown", (e) => {
  const mod = window.__ski;
  if (!mod)
    return;
  const [x, y] = canvasPt(e);
  mod._ski_click(x, y);
});
canvas.addEventListener("mousemove", (e) => {
  const mod = window.__ski;
  if (!mod)
    return;
  const [x, y] = canvasPt(e);
  mod._ski_mouse_move(x, y);
});

/* Modal boxes: the shim's MessageBoxA RECORDS the box and returns
 * IDOK immediately — the pump never stops (wine 9.0 keeps the main
 * window ticking AND painting under a modal; shim/win.c modal note).
 * To keep that box visible to the player, poll the pending state and
 * render a DOM overlay; the buttons replay the answer via
 * _ski_messagebox_answer (1=IDOK / 2=IDCANCEL — only the assert site
 * acts on IDCANCEL: DestroyWindow, ski_core.c:142). */
let boxOpen = false;
setInterval(() => {
  const mod = window.__ski;
  if (!mod)
    return;
  const pending = mod._ski_messagebox_get() === 1;
  if (pending === boxOpen)
    return;
  boxOpen = pending;
  let ov = document.getElementById("ski-box");
  if (pending) {
    if (!ov) {
      ov = document.createElement("div");
      ov.id = "ski-box";
      ov.style.cssText = "position:fixed;inset:0;display:flex;" +
        "align-items:center;justify-content:center;" +
        "background:rgba(0,0,0,0.4);z-index:10";
      const dlg = document.createElement("div");
      dlg.style.cssText = "background:#c0c0c0;border:2px solid;border-color:" +
        "#fff #000 #000 #fff;padding:0;min-width:320px;font:12px 'MS Sans Serif',sans-serif";
      const cap = document.createElement("div");
      cap.style.cssText = "background:#000080;color:#fff;padding:3px 6px";
      cap.textContent = mod._ski_messagebox_caption() || "";
      const txt = document.createElement("div");
      txt.style.cssText = "padding:16px 12px;white-space:pre-wrap";
      txt.textContent = mod._ski_messagebox_text() || "";
      const row = document.createElement("div");
      row.style.cssText = "display:flex;justify-content:center;gap:8px;" +
        "padding:0 0 12px";
      const mk = (label, r) => {
        const b = document.createElement("button");
        b.textContent = label;
        b.style.cssText = "min-width:80px;padding:3px 8px;font:12px 'MS Sans Serif',sans-serif";
        b.onclick = () => {
          mod._ski_messagebox_answer(r);
        };
        return b;
      };
      row.appendChild(mk("OK", 1));
      row.appendChild(mk("Cancel", 2));
      ov.appendChild(dlg);
      dlg.appendChild(cap);
      dlg.appendChild(txt);
      dlg.appendChild(row);
      document.body.appendChild(ov);
    }
  } else if (ov) {
    ov.remove();
  }
}, 50);
