/* SkiFree web boot (Task 20 smoke; T22 polishes).
 *
 * Loads the emscripten module (build output: build-web/ski.js, copied to
 * web/ for the smoke — see .gitignore) and parks it on window.__ski,
 * the surface the harness drives (API.md "Debug hooks"):
 *   __ski.ski_tick_get()            ticks completed
 *   __ski.ski_key_event(vk, down)   interactive keys
 *   __ski.ski_click(x, y)           canvas clicks
 *   __ski.ski_window_png(n, ptr, cap)  PNG dataURL of window n's client,
 *                                      written UTF-8 into buf ptr (cap
 *                                      bytes); returns length (0 = error).
 *                                      buf = __ski._malloc(cap); read it
 *                                      back with __ski.UTF8ToString(ptr)
 *   __ski.ski_messagebox_get()      1 while a MessageBoxA box is up
 *   __ski.ski_messagebox_type()     the box's UINT type
 *   __ski.ski_messagebox_text()     box text
 *   __ski.ski_messagebox_caption()  box caption
 *   __ski.ski_messagebox_answer(r)  answer the box (IDOK 1 / IDCANCEL 2)
 */
"use strict";

window.__ski_ready = createSki().then((mod) => {
  window.__ski = mod;
  /* The shim's rAF pump starts here, after main() (WinMain) returned:
     emscripten_set_main_loop inside main() unwinds the C stack
     (simulate_infinite_loop handoff) and the rest of boot never runs
     (shim/win.c notes). By now boot is complete: windows created,
     40 ms game timer armed, main window dirty. */
  mod._ski_start_pump();
  return mod;
});
