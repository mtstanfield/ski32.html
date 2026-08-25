# NOTES — SkiFree (ski32.exe) decompilation

Source: `original/ski32.exe` (i386, PE32, MSVC6-era, single source file `V:\hack\ski32\ski2.c`),
decompiled with Ghidra 12.1.3 (project `ski32`, program `ski32.exe`) into `decompile/ghidra/*.c` —
**168 functions** (`FUN_XXXXXXXX.c`; 163 from the initial decompile + 5 added by the
function-pointer audit, see *Function-pointer audit* below), cross-referenced against the binary.

## Classification summary

- **106 GAME** (ski2.c logic — all renamed below with `ghidra_name` preserved)
- **62 CRT** (MSVC6 C runtime + PE entry/exit/SEH — left unrenamed)

Method: `harness/triage.py` heuristically classifies by scanning each `.c` for `DAT_0040XXXX`
references into verified game-owned data ranges, then every classification was hand-verified by
reading the decompiled C of all 163 functions (see *Triage adjustment* below); the 5 audit-added
functions were classified from their decompiled C (2 GAME, 3 CRT).

## Naming convention

| prefix | meaning |
|---|---|
| `main_*` | process entry (WinMain) |
| `wproc_main_*` | SkiMain window handlers; `wproc_main` (0x405800) is the WndProc dispatcher itself |
| `wproc_status_*` | SkiStatus window handlers (0x4068d0 is the real WndProc) |
| `game_*` | game logic (entities, gates, spawning, style, pause, level, windows, sprites) |
| `draw_*` | rendering (entities, scene, status values, text, offscreen) |
| `score_*` | high-score dialog + style score |
| `snd_*` | WAVE resource loading/playing (winmm `sndPlaySoundA`) |
| `util_*` | helpers (rects, lerp, time fmt, string cache, bitmap load, asserts, facing math) |

## Full function map (168)

Role text is grounded in the decompiled C (`decompile/ghidra/FUN_*.c`).

| addr | name | ghidra_name | size | class | role |
|---|---|---|---|---|---|
| 0x401000 | game_tick | FUN_00401000 | 96 | GAME | timer tick: c5f4=now-c698, c708=prev, c698=now; physics (game_physics), render pass (game_render); redraw status if >327ms since c5dc; c610=1 |
| 0x401060 | game_render | FUN_00401060 | 476 | GAME | render+collide pass over active entity list: merge groups, collide, draw |
| 0x401240 | util_assert | FUN_00401240 | 47 | GAME | assert fail: wsprintf("%s line %u", file, line) -> util_assert_box, then game_pause_toggle (game pauses after an assert) |
| 0x401270 | util_assert_box | FUN_00401270 | 30 | GAME | assert MessageBox (MB_ABORTRETRYIGNORE=0x31); Abort -> DestroyWindow(main_hwnd) |
| 0x401290 | util_rect_overlap | FUN_00401290 | 95 | GAME | AABB overlap test (NULL-asserts) |
| 0x4012f0 | util_rect_equal | FUN_004012f0 | 95 | GAME | AABB equality test (NULL-asserts) |
| 0x401350 | game_entity_die | FUN_00401350 | 52 | GAME | entity die: unlink group partner (+0x04), set flag 8 (dead) |
| 0x401390 | game_entity_reap | FUN_00401390 | 117 | GAME | reap dead entities: unlink from list/group, push onto freelist (c744) |
| 0x401410 | game_entity_rect | FUN_00401410 | 146 | GAME | compute entity world rect into +0x20 from sprite col table (c5f8); asserts rect-cache flag 4 and col-table consistency |
| 0x4014b0 | game_entity_rect_calc | FUN_004014b0 | 143 | GAME | sprite geometry -> world rect (player x c5fc, center c704, offset c640) |
| 0x401540 | draw_entity | FUN_00401540 | 1058 | GAME | draw entity group: BitBlt sprite columns with mask (asserts rect/size invariants) |
| 0x401970 | draw_offscreen_resize | FUN_00401970 | 240 | GAME | resize offscreen bitmap (c614) to 0x40-aligned size; c690/c6e8 = new w/h |
| 0x401a60 | game_group_merge | FUN_00401a60 | 180 | GAME | merge two entities into a group (link via +0x04, flags 0x10, bbox) |
| 0x401b20 | game_bbox_expand | FUN_00401b20 | 96 | GAME | expand group bounding box (min/max of corners) |
| 0x401b80 | draw_status_values | FUN_00401b80 | 354 | GAME | draw 4 status values (time via util_fmt_time, dist "%5.2dm", speed "%5.2dm/s", style "%7ld"); c5dc=now |
| 0x401cf0 | util_str_cache | FUN_00401cf0 | 123 | GAME | string cache: LoadStringA(id) -> LocalAlloc (c674); returns "[out o' memory]" on failure |
| 0x401d70 | util_fmt_time | FUN_00401d70 | 174 | GAME | format ms -> H:MM:SS.TT via RT_STRING 11 ("%2u:%2.2u:%2.2u.%2.2u") |
| 0x401e20 | draw_text_line | FUN_00401e20 | 42 | GAME | draw text line (TextOutA with width/color params) |
| 0x401e50 | game_physics | FUN_00401e50 | 607 | GAME | main physics step: scroll world (c5d8/c714 cursors, c640), step entities, spawn in 4 bands (game_spawn), rand(0x29a) gate |
| 0x4020b0 | util_rand_range | FUN_004020b0 | 23 | GAME | rand() % n |
| 0x4020d0 | game_entity_new | FUN_004020d0 | 78 | GAME | new entity from template: type (assert 0..0x11) at +0x18, set frame |
| 0x402120 | game_entity_set_frame | FUN_00402120 | 96 | GAME | set entity frame (assert <= 0x3f); column from frame_col_table (a1ac) |
| 0x402180 | game_entity_set_col | FUN_00402180 | 152 | GAME | set sprite column (+0x10) and adjust on-screen area budget (c6fc) |
| 0x402220 | game_group_split | FUN_00402220 | 86 | GAME | split/clone group: alloc copy, link as partner (+0x04) |
| 0x402280 | game_entity_alloc | FUN_00402280 | 130 | GAME | entity alloc from freelist (c744) in pool (c648) |
| 0x402310 | util_frame_special | FUN_00402310 | 21 | GAME | frame-is-special test (frames with alt column mapping in a1ac) |
| 0x402330 | game_entity_from_template | FUN_00402330 | 22 | GAME | new entity: copy 80B zero-init template (c030) into a fresh pool slot |
| 0x402350 | game_spawn_dir | FUN_00402350 | 58 | GAME | spawn entity in a direction (relative to player) |
| 0x402390 | game_entity_set_pos | FUN_00402390 | 216 | GAME | set entity position (+0x42); world-shifts when the player moves (game_world_shift) |
| 0x402470 | game_world_shift | FUN_00402470 | 126 | GAME | world shift: move all entities opposite to player scroll |
| 0x4024f0 | game_spawn_pos | FUN_004024f0 | 192 | GAME | compute spawn position per direction (rand over client w/h: c5f0/c6d8) |
| 0x4025c0 | game_spawn | FUN_004025c0 | 213 | GAME | spawn dispatcher: picks a type via one of 4 pickers by position/speed band; type<0xb via game_entity_new+spawn_frame_table (a22c), type>=0xb via game_entity_new_col+game_sprite_frame; 0x12 = no spawn |
| 0x4026a0 | game_entity_new_col | FUN_004026a0 | 78 | GAME | new entity + sprite column (type 0..0x11, col from game_sprite_frame) |
| 0x4026f0 | game_spawn_pick_wide | FUN_004026f0 | 122 | GAME | type pick (wide zone): r=rand(1000): <50→10, <500→0xd, <700→0xf, <750→0xb, <950→0xe, <970→0x10, <990→0x12 (none), ≥990→2 (dog); 0x12 when view full (c748/32 ≤ c6fc); no type-1 outcome |
| 0x402770 | game_spawn_pick_speed | FUN_00402770 | 37 | GAME | type pick (speed zone): 11 or 0x12 from speed threshold (c748/64 vs c6fc) |
| 0x4027a0 | game_spawn_pick_narrow | FUN_004027a0 | 52 | GAME | type pick (GS lane): r = rand(64): r == 0 → 2 (dog, 1/64), else 0xd (13, 63/64); 0x12 when view full (disasm 0x4027be-0x4027d3: `neg %ax; sbb %eax,%eax; and $0xb,%eax; add $0x2,%eax`) |
| 0x4027e0 | game_spawn_pick_mid | FUN_004027e0 | 100 | GAME | type pick (FS lane): r = rand(100): <2→10, <20→0xd, <50→0xf, <60→0xb, <80→0x10 (16, banner), else 0xe (14, rock); 0x12 when view full (disasm 0x402838-0x402840: `cmp $0x50; sbb; and $0xfe,%al; add $0x10`) |
| 0x402850 | game_sprite_frame | FUN_00402850 | 118 | GAME | sprite column from entity type (type switch) |
| 0x4028e0 | game_entity_activate | FUN_004028e0 | 584 | GAME | entity activate: per-type dispatch (activate_sound_table a308, activate_anim_table a434), sound via c6c0/c718 |
| 0x402ba0 | snd_play | FUN_00402ba0 | 58 | GAME | play WAVE: LockResource + sndPlaySoundA (c790), gated by sound_disabled (c794) |
| 0x402be0 | game_entity_step | FUN_00402be0 | 125 | GAME | entity step: per-type movement/velocity substep |
| 0x402c60 | game_style_ss | FUN_00402c60 | 452 | GAME | SS style check: INI "Ski"/"SS" (c0d8); style tick c944/c948/c94c/c95c/c964 |
| 0x402e30 | util_lerp | FUN_00402e30 | 66 | GAME | lerp: (p1 - (p1-p2)*(p3-p5))/(p3-p4); asserts p3 != p4 |
| 0x402e80 | game_player_face | FUN_00402e80 | 56 | GAME | player facing-frame update (c72c type, +0x11 sign -> frame 3-13 via game_entity_set_frame) + status redraw |
| 0x402ec0 | score_show | FUN_00402ec0 | 619 | GAME | high-score dialog: GetPrivateProfileStringA("Ski",key,"",buf,256,"entpack.ini"), parse <=10 whitespace-separated longs (_atoi), stored = (is_time) ? -value : value, insert at first index where stored[i] > new (list ascending in stored = descending time; max 10, last dropped), write back one line of "%ld " (c0ec) items via WritePrivateProfileStringA, MessageBoxA(c6c8, body, "High Scores" RT15, 0): lines "\n"-joined, times via util_fmt_time(-stored) / FS scores "%9ld" (c0e4), inserted line += RT16 " <-- that's you!"; insert index 10 = did not qualify → append "\n\n" (c0dc) + new value + RT17 " <-- try again!" |
| 0x403130 | game_gate_set_col | FUN_00403130 | 66 | GAME | gate column set (from sprite table c5f8) |
| 0x403180 | game_style_fs | FUN_00403180 | 207 | GAME | FS style check: INI "Ski"/"FS" (c0f4); c954/c968 flags + style score |
| 0x403250 | game_style_gs | FUN_00403250 | 452 | GAME | GS style check: INI "Ski"/"GS" (c0f8); c944/c948/c950/c958/c960 flags |
| 0x403420 | score_add | FUN_00403420 | 16 | GAME | add to style score (c6a8) |
| 0x403430 | game_anim_update | FUN_00403430 | 259 | GAME | anim update from table (advance frame/anim state) |
| 0x403540 | game_anim_type1 | FUN_00403540 | 201 | GAME | type-1 animation (table a490) |
| 0x403610 | game_anim_type2 | FUN_00403610 | 289 | GAME | type-2 animation (sound c608) |
| 0x403750 | game_anim_type9 | FUN_00403750 | 92 | GAME | type-9 animation |
| 0x4037b0 | game_anim_type10 | FUN_004037b0 | 331 | GAME | type-10 animation |
| 0x403910 | game_anim_type3 | FUN_00403910 | 231 | GAME | type-3 animation (table a4e0) |
| 0x403a00 | game_collide | FUN_00403a00 | 1492 | GAME | collision handler: big type switch; player crash types 5-8; kill/respawn, sounds |
| 0x404070 | game_group_head | FUN_00404070 | 33 | GAME | group head: walk group links to first member |
| 0x4040a0 | game_gate_update | FUN_004040a0 | 129 | GAME | gate array update/spawn loop (c720 list; view band c5fc/c684/c68c) |
| 0x404130 | game_gate_spawn | FUN_00404130 | 138 | GAME | spawn entity from gate descriptor (36B records at c758) |
| 0x4041c0 | game_gate_step | FUN_004041c0 | 194 | GAME | gate state step (per descriptor, disasm 0x4041d7-0x40427a): x+=vx, y+=vy, z+=fdelta; type 4 → game_gate_type4, 5-8 → game_gate_cruise, else assert 0xaf9; if spawned: assert entity+0x0c==desc (0xb00), set_pos(entity,x,y,z) + set_frame(entity, desc+0x10) every tick |
| 0x404290 | game_gate_type4 | FUN_00404290 | 177 | GAME | bench update (disasm 0x4042bb-0x40433f): y ≤ -0x400 → frame 41, vy=+2, x=-144; y ≥ 0x5c00 → frame 39, vy=-2, x=-112; else if spawned && frame == 39 && rand(1000)==0 → spawn snowboarder (type 3, frame 0x21) at bench x/y, bench → frame 0x28 |
| 0x404350 | game_gate_cruise | FUN_00404350 | 883 | GAME | yeti update (types 5-8): airborne z/fdelta handling (0x404378); wake-anim states 50-55 vs c698-desc+0x20 (jump table 0x4046c4); self-gate → per-type sentinel velocity, else player-threshold trigger → teleport beyond view edge + clamp velocity (±16, ±26) + snd_play(c6f0); (0,0) idle → rand(10) frame 42/43 |
| 0x4046e0 | game_gate_scan | FUN_004046e0 | 222 | GAME | gate array scan: spawn within view band (c5fc +/- 0x3c, c684/c68c) |
| 0x4047c0 | game_tick_cb | FUN_004047c0 | 22 | GAME | SetTimer callback (c940): if game active (c67c) -> game_tick; returns 1 |
| 0x4047e0 | main_winmain | FUN_004047e0 | 213 | GAME | WinMain: lstrcmpiA "nosound" -> c794; game_init_mem, game_reset, game_create_windows, game_start; GetMessage/TranslateMessage/DispatchMessage loop; snd_shutdown; returns msg.wParam |
| 0x4048c0 | game_init_mem | FUN_004048c0 | 138 | GAME | LocalAlloc pools: c674 str cache (0x50), c5f8 sprite cols (0x5a0), c648 entity pool (8000=100x80B), c758 gate descs (0x2400=256x36B); fatal "Insufficient local memory." on failure |
| 0x404950 | util_fatal_msg | FUN_00404950 | 27 | GAME | fatal message: MessageBoxA(NULL, msg, "SkiFree" (RT 1), MB_ICONERROR=0x30) |
| 0x404970 | game_reset | FUN_00404970 | 143 | GAME | reset all game state: srand(GetTickCount), freelist, zero ~15 counters/flags, c678=0x28 (40ms timer), c610=1 |
| 0x404a00 | game_freelist_init | FUN_00404a00 | 100 | GAME | init 100-entity freelist (c648 pool, 80B slots) -> c744 |
| 0x404a70 | game_gate_idx_reset | FUN_00404a70 | 10 | GAME | clear gate descriptor index (c702 = 0) |
| 0x404a80 | game_start | FUN_00404a80 | 71 | GAME | game_start: create player (type 0, frame 3) at origin -> c64c/c72c; spawn start poles; level init; clear game_over; game_resume |
| 0x404ad0 | game_resume | FUN_00404ad0 | 128 | GAME | game_resume: guards c6c8/c6d0/c650; c6d0=1, c698=now; style-tick adjust (c948 += now - c600) if c95c||c958; SetTimer(c6c8, 0x29a, c678, c940) |
| 0x404b50 | game_level_init | FUN_00404b50 | 1448 | GAME | level init: build 5 gate lists (SS c630 0x404b72-0x404cd7, GS c5e0 0x404ce1-0x404e79, FS c658 0x404e83-0x404f48, pines c738 0x404f4d-0x404fb6, benches+monsters c720 0x404fb8-0x4050eb) via game_gate_list_add, style reset |
| 0x405100 | game_gate_list_clear | FUN_00405100 | 30 | GAME | gate list clear (free descriptors, c720=NULL, c702=0) |
| 0x405120 | game_gate_list_add | FUN_00405120 | 68 | GAME | gate list add (disasm 0x405120-0x405163): copy 36B stack descriptor into the next c758 slot (assert c702 ≤ 0x100), [slot+0] = NULL (backfilled by game_gate_spawn), [slot+4] = c5f8 + low16(desc+0x08)*0x10 (column entry ptr), [slot+0x0c] = type<<16, append slot ptr to the caller's list triple |
| 0x4051e0 | game_startpoles_spawn | FUN_004051e0 | 226 | GAME | spawn start flagpoles (type 0x11, frames 0x35-0x38) |
| 0x4052d0 | game_create_windows | FUN_004052d0 | 842 | GAME | create_windows: GetDeviceCaps HORZRES/VERTRES (c6a0/c74c), c61c=hInstance, white brush c69c, FindWindowA("SkiMain") single-instance check, load 9 WAVE resources (gated by !c794), RegisterClassA SkiMain (lpfnWndProc = wproc_main, 0x405800) + SkiStatus (wproc_status; the "button" string a1a4 is NOT registered — dead .rdata), CreateWindowExA both, icon (c120); c940 = &game_tick_cb; flags init c6d0=0, c770=1, c694=0, c67c=0 (starts paused) |
| 0x405620 | snd_init | FUN_00405620 | 20 | GAME | snd_init: c790 = sndPlaySoundA import |
| 0x405640 | snd_load_wave | FUN_00405640 | 83 | GAME | snd_load_wave: FindResourceA(hInst, id, "WAVE") -> pair {HGLOBAL, LockResource(pData)} |
| 0x4056a0 | snd_shutdown | FUN_004056a0 | 131 | GAME | snd_shutdown: stop sound, FreeLibrary(c78c), free all 9 WAVE pairs |
| 0x405730 | snd_free | FUN_00405730 | 38 | GAME | snd_free: FreeResource(pair[0]) |
| 0x405760 | game_pause_toggle | FUN_00405760 | 89 | GAME | pause/resume toggle (F2 key; also the post-assert path): running -> game_pause + title "Ski Paused ... Press F3 to continue" (RT 2); else title "SkiFree" (RT 1) + game_resume |
| 0x4057c0 | game_pause | FUN_004057c0 | 52 | GAME | game_pause: KillTimer(0x29a), c600 = now, c6d0 = 0 |
| 0x405800 | wproc_main | FUN_00405800 | 513 | GAME | SkiMain WndProc dispatcher (computed jump table 0x4059c4/0x4059e0; code proper 451B, body includes the table): WM_CREATE -> wproc_main_create (return -1 on fail) + wproc_main_size; WM_DESTROY -> wproc_main_destroy + PostQuitMessage(0); WM_SIZE -> wproc_main_size + status reposition + c770=(wParam==SIZE_MINIMIZED) + game_pause_auto + UpdateWindow(main) if c67c; WM_ACTIVATE -> c694=wParam, SetFocus(main) if active, game_pause_auto; WM_PAINT -> wproc_main_paint; WM_GETMINMAXINFO -> min size 320x300 (fields at lParam+0x18/+0x1C); WM_ERASEBKGND -> unhandled (default class-brush erase); WM_MOUSEMOVE -> wproc_main_aim (if c67c); WM_KEYDOWN -> wproc_main_input (if c67c); WM_CHAR -> wproc_main_key (if c67c); WM_LBUTTONDOWN/LBUTTONDBLCLK -> wproc_main_turn (if c67c); WM_MOUSEACTIVATE -> MA_NOACTIVATE if screen-x==1; default -> DefWindowProcA |
| 0x405a10 | game_pause_auto | FUN_00405a10 | 48 | GAME | auto pause/resume on activate/minimize: if window active (c694) && !minimized (c770) -> c67c=1 + game_resume; else c67c=0 + game_pause |
| 0x405a40 | wproc_main_create | FUN_00405a40 | 111 | GAME | wproc_main WM_CREATE: GetDC (c63c), reset GDI objects, game_sprites_load; returns 0 on failure (WndProc then returns -1); fatal "Whoa, like, can't load bitmaps!" on failure; WndProc also calls wproc_main_size on success |
| 0x405ab0 | game_sprites_load | FUN_00405ab0 | 996 | GAME | sprites_load: LoadBitmapA ids 1-0x59 (util_load_bitmap), build c5f8 column table, mask DCs (c710/c6a4/c730/c6ec) + offscreen bitmap |
| 0x405ea0 | util_load_bitmap | FUN_00405ea0 | 20 | GAME | LoadBitmapA wrapper |
| 0x405ec0 | wproc_main_destroy | FUN_00405ec0 | 214 | GAME | wproc_main WM_DESTROY: restore+delete GDI objects (DCs, bitmaps) |
| 0x405fa0 | wproc_main_size | FUN_00405fa0 | 179 | GAME | wproc_main WM_SIZE: client rect (c6b0/c6b4/c6b8/c6bc), view bounds (c680-c68c), view width/area (c5f0/c748), client height (c6d8), center via game_set_center; also invoked from the WM_CREATE success path |
| 0x406060 | game_set_center | FUN_00406060 | 73 | GAME | reset entity rect-cache flags (list walk), set view center c5fc/c704 |
| 0x4060b0 | wproc_main_paint | FUN_004060b0 | 71 | GAME | paint pass: BeginPaint, FillRect (white brush c69c), draw_scene, EndPaint — called from the WM_PAINT (0x0F) case; WM_ERASEBKGND (0x14) is unhandled and goes to DefWindowProcA (default class-brush erase) |
| 0x406100 | draw_scene | FUN_00406100 | 112 | GAME | draw_scene: render pass (c618 list, offscreen c614) |
| 0x406170 | wproc_main_input | FUN_00406170 | 565 | GAME | wproc_main keyboard handler (WM_KEYDOWN, gated by c67c in wproc_main): Enter=restart (only when no player), F1=restart, Esc=ShowWindow minimize, F2=game_pause_toggle; arrows/i/c/a/g/d/h/f/b/space/` keys -> facing frames (mode 0), steer +0x46 by -8/+8 (clamp 8) via tables a258/a25c, slide/jump frame-state transitions (frames 3/7/0xc/6/8/0xd/0xe/0xf/0x12/0x13/0x14/0x15); sets frame via game_entity_set_frame + renders if c610 |
| 0x406500 | game_restart | FUN_00406500 | 76 | GAME | game_restart (Enter/F1 via wproc_main_input): if game_over -> game_reset + game_start + InvalidateRect/UpdateWindow, else DestroyWindow(main) |
| 0x406550 | wproc_main_aim | FUN_00406550 | 136 | GAME | wproc_main mouse handler (WM_MOUSEMOVE, gated by c67c): mouse pos -> facing via util_facing_delta/crouch -> set frame; store c700/c70c, c760=1 |
| 0x4065e0 | util_facing_delta | FUN_004065e0 | 132 | GAME | facing delta: upright orientation from mouse delta |
| 0x406670 | util_facing_crouch | FUN_00406670 | 85 | GAME | facing crouch: crouched orientation from mouse delta |
| 0x4066d0 | wproc_main_turn | FUN_004066d0 | 148 | GAME | wproc_main mouse button handler (WM_LBUTTONDOWN/LBUTTONDBLCLK, gated by c67c): key-repeat facing change; slide/jump states |
| 0x406780 | wproc_main_key | FUN_00406780 | 203 | GAME | wproc_main char handler (WM_CHAR, gated by c67c): X/Y/Z + numpad 1/2/3 = move +/-2, F = speed toggle (c670), R = redraw, T = manual tick (game_tick) |
| 0x406890 | wproc_status_reposition | FUN_00406890 | 58 | GAME | wproc_status reposition: place status window vs client rect (c66a/c66c vs c6b4/c6b8) |
| 0x4068d0 | wproc_status | FUN_004068d0 | 119 | GAME | wproc_status: SkiStatus WndProc (WM_PAINT -> wproc_status_paint, WM_DESTROY -> wproc_status_destroy, reposition on move) |
| 0x406970 | wproc_status_paint | FUN_00406970 | 253 | GAME | wproc_status_paint: FrameRect (c778), 4 labels (RT 3-6) + values (draw_status_values) |
| 0x406a70 | wproc_status_create | FUN_00406a70 | 480 | GAME | wproc_status_create: GetDC (c6cc), GetTextMetricsA (c668), measure RT strings 3-10 -> panel size (c66c/c66a), select DEFAULT_GUI_FONT (c664) |
| 0x406c50 | util_text_width | FUN_00406c50 | 47 | GAME | max text width: GetTextExtentPoint32A helper |
| 0x406c80 | wproc_status_destroy | FUN_00406c80 | 65 | GAME | wproc_status_destroy: restore stock object, ReleaseDC, assert if hwnd != c624 |
| 0x406cd0 | FUN_00406cd0 | — | 10 | CRT | CRT srand (seed at c16c) |
| 0x406cda | FUN_00406cda | — | 30 | CRT | CRT rand |
| 0x406cf8 | FUN_00406cf8 | — | 139 | CRT | CRT _atoi |
| 0x406d83 | entry | — | 235 | CRT | CRT PE entry (_tmainCRTStartup): env/heap/stdio/locale init, calls WinMain |
| 0x406e6e | FUN_00406e6e | — | 11 | CRT | CRT abort-continuation stub (SEH table slot a568): rewrite SEH record handler -> FUN_00406e79, then __exit(code); no ret |
| 0x406e79 | FUN_00406e79 | — | 34 | CRT | CRT abort path |
| 0x406e9e | FUN_00406e9e | — | 35 | CRT | CRT abort path 2 |
| 0x406ec2 | FUN_00406ec2 | — | 117 | CRT | CRT _getbyteclass |
| 0x406f74 | FUN_00406f74 | — | 45 | CRT | CRT _initterm |
| 0x406fa1 | FUN_00406fa1 | — | 17 | CRT | CRT atexit chain |
| 0x406fb2 | __exit | — | 17 | CRT | CRT exit state reset |
| 0x406fc3 | FUN_00406fc3 | — | 153 | CRT | CRT common exit |
| 0x40705c | FUN_0040705c | — | 26 | CRT | CRT SEH helper |
| 0x407076 | FUN_00407076 | — | 321 | CRT | CRT SEH filter |
| 0x4071b7 | FUN_004071b7 | — | 67 | CRT | CRT exception handler lookup |
| 0x4071fa | FUN_004071fa | — | 88 | CRT | CRT cmdline arg parse |
| 0x407252 | FUN_00407252 | — | 185 | CRT | CRT env init |
| 0x40730b | FUN_0040730b | — | 153 | CRT | CRT prog-path init |
| 0x4073a4 | FUN_004073a4 | — | 436 | CRT | CRT cmdline split (ctype table caa0) |
| 0x407558 | FUN_00407558 | — | 306 | CRT | CRT env string alloc |
| 0x40768a | FUN_0040768a | — | 427 | CRT | CRT ctype/locale table init (cbc0-ccc0) |
| 0x407835 | FUN_00407835 | — | 60 | CRT | CRT InitCommonControls import-table init (a860) |
| 0x407874 | __global_unwind2 | — | 32 | CRT | CRT __global_unwind2 |
| 0x407894 | FUN_00407894 | — | 34 | CRT | CRT SEH unwind-target helper (the address pushed as unwind target by __local_unwind2): if erec->flags & (EH_NESTED_CALL|EH_EXIT_UNWIND) -> *arg4 = arg3, return 3; else return 1 |
| 0x4078b6 | __local_unwind2 | — | 104 | CRT | CRT __local_unwind2 |
| 0x40794a | FUN_0040794a | — | 24 | CRT | CRT exception table lookup |
| 0x40796c | FUN_0040796c | — | 189 | CRT | CRT SEH unwind/exit dispatcher (shared SEH-record target of entry/0x407bd0/0x4090ff): walks the handler table, invokes handlers, dispatches __global_unwind2/__local_unwind2/FUN_0040794a |
| 0x407a29 | FUN_00407a29 | — | 27 | CRT | CRT __C_specific_handler |
| 0x407a44 | FUN_00407a44 | — | 57 | CRT | CRT reported-error setup |
| 0x407a7d | FUN_00407a7d | — | 339 | CRT | CRT runtime error report (shared c0dc string) |
| 0x407bd0 | FUN_00407bd0 | — | 318 | CRT | CRT locale CP init (c900-c93c) |
| 0x407d19 | FUN_00407d19 | — | 17 | CRT | CRT char-class lookup |
| 0x407d2a | FUN_00407d2a | — | 49 | CRT | CRT char-class lookup 2 |
| 0x407d5b | FUN_00407d5b | — | 409 | CRT | CRT codepage char lookup (CodePage c984, tables c4d0/caa0) |
| 0x407ef4 | FUN_00407ef4 | — | 74 | CRT | CRT locale/cpinfo |
| 0x407f3e | FUN_00407f3e | — | 51 | CRT | CRT DBCS codepage |
| 0x407f71 | FUN_00407f71 | — | 41 | CRT | CRT codepage reset |
| 0x407f9a | FUN_00407f9a | — | 389 | CRT | CRT tolower table init |
| 0x40811f | FUN_0040811f | — | 28 | CRT | CRT CP init state |
| 0x40813b | FUN_0040813b | — | 47 | CRT | CRT _free (HeapFree wrapper) |
| 0x408170 | FUN_00408170 | — | 7 | CRT | CRT _strcpy |
| 0x408180 | FUN_00408180 | — | 224 | CRT | CRT _strcat |
| 0x408260 | _malloc | — | 18 | CRT | CRT _malloc |
| 0x408272 | __nh_malloc | — | 44 | CRT | CRT __nh_malloc |
| 0x40829e | FUN_0040829e | — | 54 | CRT | CRT heap malloc |
| 0x4082e0 | _strlen | — | 123 | CRT | CRT _strlen |
| 0x408360 | FUN_00408360 | — | 664 | CRT | CRT malloc free-list scan |
| 0x408695 | FUN_00408695 | — | 62 | CRT | CRT heap block init |
| 0x4086d3 | FUN_004086d3 | — | 43 | CRT | CRT heap block find |
| 0x4086fe | FUN_004086fe | — | 811 | CRT | CRT heap free (c96c-c980) |
| 0x408a29 | FUN_00408a29 | — | 777 | CRT | CRT heap alloc/coalesce (c974-c980) |
| 0x408d32 | FUN_00408d32 | — | 177 | CRT | CRT heap new block |
| 0x408de3 | FUN_00408de3 | — | 251 | CRT | CRT heap block init 2 |
| 0x408ede | FUN_00408ede | — | 137 | CRT | CRT MessageBoxA indirect (lazy import) |
| 0x408f70 | _strncpy | — | 254 | CRT | CRT _strncpy |
| 0x409070 | _memset | — | 88 | CRT | CRT _memset |
| 0x4090d0 | FUN_004090d0 | — | 47 | CRT | CRT stack align (_chkstk) |
| 0x4090ff | FUN_004090ff | — | 511 | CRT | CRT LCMapStringA/W wrapper (state c934) |
| 0x409323 | FUN_00409323 | — | 43 | CRT | CRT strnlen |
| 0x40934e | FUN_0040934e | — | 27 | CRT | CRT OOM handler |
| 0x409370 | FUN_00409370 | — | 664 | CRT | CRT _memcpy |
| 0x4096a6 | RtlUnwind | — | 6 | CRT | CRT RtlUnwind |
### GAME functions (106) — renamed

See the table above for the per-function role. Key subsystems:

- **Timing/pause**: `game_tick` (0x401000) is driven by the 40ms `SetTimer` callback
  `game_tick_cb` (0x4047c0, c940), which calls `game_tick` only while the game is active (c67c);
  `game_tick` -> `game_physics` (0x401e50) + `game_render` (0x401060). Pause: `game_pause`
  (0x4057c0, `KillTimer`), resume: `game_resume` (0x404ad0). `game_pause_toggle` (0x405760) is
  the F2 key and the post-assert path; `game_pause_auto` (0x405a10) pauses when the window is
  deactivated or minimized (`c694`/`c770`) and resumes on activate+restore — see *WndProc
  routing / auto-pause semantics* below.
- **Entities**: 80B structs from a 100-slot pool (`c648`), freelist `c744`; types 0-17
  (0 = player, 1-10 spawnable obstacles/gates, 11-17 gate variants; 0x12 = "no spawn").
- **Gates**: five lists (`c630` SS, `c5e0` GS, `c658` FS, `c738` tall poles, `c720`
  moving: benches + yetis), each a `{first, end, scan-cursor}` pointer triple into the
  contiguous 36B descriptor pool `c758` (256 slots, max 256); the four static lists are
  spawned by `game_gate_scan` within the view band (client rect extended by 0x78px);
  `c720` is additionally stepped every tick by `game_gate_update`.
- **Style score** (FS/SS/GS bonuses, INI-gated via keys at 0x40c0d8/c0f4/c0f8): accumulators at
  `c944-c968`, displayed as `Style:` on the status panel.
- **High scores**: `score_show` (0x402ec0) reads/writes section `Ski` of `entpack.ini`
  (0x40c084), keeps up to 10 scores, index 0 = best (times stored negated → list
  **ascending** in stored value = **descending** time), one space-separated `"%ld "` line;
  box caption `High Scores` (RT_STRING 15), suffixes `that's you!`/`try again!` (16/17).
- **Sound**: 9 `snd_load_wave` calls at startup (ids 1-9) into 8-byte `{HGLOBAL,
  pData}` pairs; played via `sndPlaySoundA` (`c790`); disabled by the `nosound`
  cmdline flag (`c794`); pair map: id1→c6c0, id2→c768, id3→c5d0, id4→c718, id5→c750,
  id6→c628, id9→c6f0, id7→c6e0, id8→c608. **No WAVE resources exist in the PE** —
  every load returns NULL and all playback is silent (M1#3).
- **Windows**: `main_winmain` -> `game_create_windows` (0x4052d0) registers classes `SkiMain`
  (WndProc = `wproc_main`, 0x405800) and `SkiStatus` (WndProc = `wproc_status`) and creates both
  windows (status window initial text = `c788`, an empty 64B buffer). The main window title is
  RT_STRING 1 (`SkiFree`); the paused title is RT_STRING 2. The game starts paused (c770=1,
  c694=0, c67c=0) and only runs once the window is both activated and sized.

### CRT functions (62) — left unrenamed

All MSVC6 CRT: PE entry `0x406d83`, heap (`0x40813b-0x408de3`, heap state `c96c-c980`, threshold
`c5c0`), string/locale/ctype (`0x40768a`, `0x407bd0`, `0x407d19-0x407f9a`, `0x408ede`, `0x4090ff`,
`0x409370`, `0x408f70`, `0x409070`, `0x4082e0`, `0x408260`), exit/SEH (`0x406e6e`,
`0x406e79-0x407a7d`, `0x407874`, `0x407894`, `0x4078b6`, `0x40796c`, `0x4096a6`), and
`srand`/`rand`/`_atoi` (`0x406cd0-0x406cf8`).
Every CRT function with size > 400 was opened and checked: `0x4073a4` (cmdline split over ctype
table `caa0`), `0x40768a` (ctype table init `cbc0-ccc0`), `0x407d5b` (codepage char lookup via
`CodePage`/`c984`), `0x408360` (malloc free-list scan), `0x4086fe` (heap free, `c96c-c980`),
`0x408a29` (heap alloc/coalesce, `c974-c980`), `0x4090ff` (LCMapStringA/W probe, state `c934`),
`0x409370` (`_memcpy`) — all confirmed CRT.

## Triage adjustment (why the pattern changed)

Original pattern `DAT_0040([a-fc-d])([0-9a-f]{2})` matched all of 0x40a000-0x40ffff and produced a
**degenerate 116 GAME / 47 CRT (71%)** split: it counted CRT-owned data as game signal — the CRT
heap (`c5c0`, `c96c-c9a0`), ctype table (`c178-c182`), locale state (`c900-c93c`, `c384+`), env
strings (`c798-c7dc`), SEH unwind tables (`a560`, `a860`, `a8a8`) and CRT init arrays (`c000-c020`).

Adjusted pattern (in `harness/triage.py`) matches only ranges verified game-owned by reading the C
and cross-referencing the Ghidra data-reference dump (196 unique DAT addresses):

| range | content |
|---|---|
| `0x40a190-0x40a4ff` | .rdata: class names + game const tables (a560/a8xx SEH excluded) |
| `0x40d0xx` | .rsrc (no code references it directly; pattern kept) |
| `0x40c030-0x40c13f` | .data game strings (specific subranges; **excludes shared `c0dc`** and CRT `c16c+`) |
| `0x40c5d0-0x40c6ff` | game state block part 1 (excludes CRT `c5c0`) |
| `0x40c700-0x40c794` | game state block part 2 (excludes CRT env `c798+`) |
| `0x40c940-0x40c968` | timer proc ptr + style accumulators (excludes CRT heap `c96c+`) |

Result: **76 GAME / 87 CRT (46.6%)** heuristic — no false positives (all 76 are truly game code).
The 28 game functions with zero `DAT_` refs (pure computation or named-symbol string refs only)
were reclassified to GAME by hand review — for the original 163 functions the truth was
**104 GAME / 59 CRT**. The follow-up function-pointer audit then added 5 functions (2 GAME:
`game_tick_cb`, `wproc_main`; 3 CRT: SEH helpers) — final: **106 GAME / 62 CRT** (168 total).
Missed-by-heuristic list: 1240, 1290, 12f0, 1350, 1540, 1a60, 1b20, 1d70, 20b0, 20d0, 2220, 2310,
2350, 26a0, 2850, 2e30, 3430, 3750, 37b0, 4070, 41c0, 4290, 4950, 5100, 5730, 65e0, 6670, 6c50
(all addresses 0x40-prefixed).

## Function-pointer audit (follow-up, all targets resolved)

The initial triage flagged two `.text` targets referenced by decompiled code but absent from the
163-function set (0x4047c0, 0x405800). A full audit was then performed; **every `.text` pointer
target now has a decompiled function**.

1. **`&LAB_` targets in the decompiled C** — grep of all 163 `.c` files for `LAB_004xxxx`:

   | target | site | status |
   |---|---|---|
   | 0x4047c0 | `DAT_0040c940 = &LAB_004047c0` in `game_create_windows` (SetTimer callback) | was missing -> now `game_tick_cb` |
   | 0x405800 | `local_28.lpfnWndProc = &LAB_00405800` in `game_create_windows` (SkiMain class) | was missing -> now `wproc_main` |
   | 0x40796c | SEH-record unwind target (in `entry`, 0x407bd0, 0x4090ff) | was missing -> now FUN_0040796c |
   | 0x407894 | unwind address pushed by `__local_unwind2` (and tested by the SEH frame-check at 0x40791e) | was missing -> now FUN_00407894 |

   (Every other `LAB_` hit is an intra-function goto label, not a pointer.)

2. **Static `.rdata`/`.data` pointer values** — every 4-byte value in `.rdata`
   (0x40a000-0x40b0f0) and `.data` (0x40c000-0x40ccca) that falls in `.text`
   (0x401000-0x4096ac): exactly 10 values, all covered after the audit:

   | data addr | value | function |
   |---|---|---|
   | 0x40a564 | 0x406e5a | `entry` (SEH unwind target inside entry) |
   | 0x40a568 | 0x406e6e | was missing -> now FUN_00406e6e (CRT abort-continuation stub) |
   | 0x40a864 / 0x40a868 | 0x407cc9 / 0x407ccd | FUN_00407bd0 (SEH unwind targets) |
   | 0x40a8ac / 0x40a8b0 / 0x40a8b8 / 0x40a8bc | 0x40920f / 0x409213 / 0x4092c3 / 0x4092c7 | FUN_004090ff (SEH unwind targets) |
   | 0x40c00c | 0x40811f | FUN_0040811f |
   | 0x40c170 | 0x406fb2 | `__exit` |

   (c940 -> 0x4047c0 is zero-initialized in the image and set at runtime, so it does not appear
   in a static file scan — covered by item 1. The a560 slot 0x40a560 = 0xFFFFFFFF is a
   sentinel, and the bytes after 0x40a56c are CRT strings, not pointers.)

3. **RegisterClassA WndProc fields**: SkiMain -> 0x405800 (now `wproc_main`); SkiStatus ->
   0x4068d0 (`wproc_status`, already in the set); no third class is registered — the `button`
   string at a1a4 has **no reference anywhere in `.text`** (dead `.rdata`).

**Functions added by the audit** (created + decompiled by `harness/ghidra/AddFunctions.java`,
invoked with `ADD_FN_ADDRS="0x4047c0 0x405800 0x406e6e 0x407894 0x40796c"`; script is
idempotent — existing functions are skipped):

| addr | name | size | class | role |
|---|---|---|---|---|
| 0x4047c0 | game_tick_cb | 22 | GAME | `if (c67c) game_tick(); return 1;` |
| 0x405800 | wproc_main | 513 (code proper 451B + jump table) | GAME | SkiMain WndProc dispatcher |
| 0x406e6e | FUN_00406e6e | 11 | CRT | abort-continuation stub: SEH record handler -> 0x406e79, `__exit(code)`, no ret |
| 0x407894 | FUN_00407894 | 34 | CRT | SEH unwind-target helper |
| 0x40796c | FUN_0040796c | 189 | CRT | SEH unwind/exit dispatcher |

Residual un-decompiled bytes (not pointer targets, left as-is): 0x40791e-0x407940 (35B SEH
frame-check predicate `fs:[0]+4 == 0x407894` — referenced nowhere in `.text`), 0x407941-0x407949
(10B tail-jump thunk into FUN_0040794a — unreferenced), 0x407964-0x40796b (8B before the
FUN_0040796c prologue — no references).

**Class-name resolution** (previous concern, now precise): the `.rdata` copies `SkiMain`@a190
and `SkiStatus`@a198 ARE referenced — by `game_create_windows` (0x4052d0) itself: a190 at
0x405335 (FindWindowA class arg, single-instance check), 0x4054d5 (WNDCLASSEX.lpClassName) and
0x40557d (CreateWindowExA class arg); a198 at 0x40550a (WNDCLASSEX.lpClassName) and 0x4055ac
(CreateWindowExA class arg). Ghidra's decompiler renders these references as inline string
literals, which is why the data-reference dump appeared empty. `button`@a1a4: zero references
anywhere — dead string.

**WndProc routing / auto-pause semantics** (from `wproc_main` + disassembly): the game runs
only while `c67c` (game active) is set. `game_pause_auto` sets `c67c = c694 && !c770`, where
`c694` = window active (WM_ACTIVATE: c694 = wParam) and `c770` = window minimized
(WM_SIZE: c770 = (wParam == SIZE_MINIMIZED)). `game_create_windows` initializes c694=0, c770=1,
c67c=0 — the game starts paused and resumes once the window is activated (WM_ACTIVATE ->
SetFocus + game_pause_auto) and sized (WM_SIZE -> game_pause_auto + UpdateWindow if active).
`c67c` gates the 40ms timer tick (`game_tick_cb`) and every input handler. Original quirks
confirmed in the dispatcher: the scene is drawn from the WM_PAINT case (WM_ERASEBKGND is
unhandled — the default class-brush erase runs); the WM_CREATE case also invokes the WM_SIZE handler on
success; WM_GETMINMAXINFO fixes the minimum window size at 320x300; WM_MOUSEACTIVATE returns
MA_NOACTIVATE when the mouse screen-x == 1; the computed jump table (7 target dwords at
0x4059c4, 33 index bytes at 0x4059e0) is resolved by Ghidra — all message cases decompiled.

## Globals

Full map in `decompile/ghidra/globals.json` (132 entries: every game data symbol with `addr`,
`type`, `refs`). Highlights:

- `.rdata`: class names `a190/a198/a1a4`; `frame_col_table` (a1ac, 64×u16 frame→column);
  `spawn_frame_table` (a22c, 11 frames for types 0-10); key action maps (a258/a25c); activation
  sound/anim tables (a308/a434); per-type anim tables (a490/a4e0).
- `.data` strings: `Ski` (c080), `entpack.ini` (c084), assert file/caption/fmt (c090/c0a8/c0bc),
  `[out o' memory]` (c0c8), INI keys `SS`/`FS`/`GS` (c0d8/c0f4/c0f8), `nosound` (c0fc),
  `Insufficient local memory.` (c104), `iconSki` (c120), `WAVE` (c128),
  `Whoa, like, can't load bitmaps! ...` (c130).
- Game state block `c5d0-c968`: HWNDs (`c6c8` main, `c624` status), DCs (`c63c` screen, `c6cc`
  status, offscreen/mask DCs `c730/c5ec/c710/c6a4/c6ec`), offscreen bitmap `c614`
  (`c690/c6e8` size), entity pool `c648` (100×80B) + freelist `c744`, active list `c618`,
  player `c64c`/`c72c`, sprite column table `c5f8`, gate lists `c630/c5e0/c658/c738/c720` +
  descriptors `c758` (256×36B) + index `c702` + count `c6fc`, view rect
  `c6b0-c6bc` (client) / `c680-c68c` (extended by 0x78) + area `c748`, spawn cursors
  `c5d8/c714` (step 0x3c), active/minimize flags (`c694` window active, `c770` window
  minimized, `c67c` game active = active && !minimized), ticks (`c698` current, `c5f4` delta,
  `c708` resume, `c5dc` last status, `c600` pause, `c6f8` style), timer (`c678` interval 40ms,
  `c6d0` active, `c940` proc ptr -> `game_tick_cb`), sound (`c790` fn ptr, `c794` disabled, 9
  WAVE pairs, `c78c` HMODULE), style accumulators `c944-c968`, score `c6a8`, string cache
  `c674`, screen `c6a0/c74c` (HORZ/VERTRES).
- Resources: RT_STRING STRINGTABLE at VA 0x41c718 (17 length-prefixed UTF-16 entries, see below);
  89 RT_BITMAP sprites + 6 RT_ICON + 2 RT_GROUP_ICON in `.rsrc` — **no RT_WAVE node**
  (the 9 `snd_load_wave` FindResourceA calls all return NULL at runtime; see M1#3);
  icon `ICONSKI`/`ICONSKI2`.

## RT_STRING table (17 strings, VA 0x41c718)

| id | string | used by |
|---|---|---|
| 1 | `SkiFree` | window title (`game_create_windows`), pause toggle title (`game_pause_toggle`), fatal-box caption (`util_fatal_msg`) |
| 2 | `Ski Paused ... Press F3 to continue` | paused title (`game_pause_toggle`) |
| 3 | `Time:` | status labels (`wproc_status_paint`/`wproc_status_create`) |
| 4 | `Dist:` | same |
| 5 | `Speed:` | same |
| 6 | `Style:` | same |
| 7 | `00:00:00.00` | panel sizing (`wproc_status_create`) |
| 8 | ` 0000m` | panel sizing |
| 9 | ` 0000m/s` | panel sizing |
| 10 | `0000000` | panel sizing |
| 11 | `%2u:%2.2u:%2.2u.%2.2u` | time format (`util_fmt_time`) |
| 12 | `%5.2dm` | dist format (`draw_status_values`) |
| 13 | `%5.2dm/s` | speed format |
| 14 | `%7ld` | style format |
| 15 | `High Scores` | score box caption (`score_show`) |
| 16 | ` <-- that's you!` | score suffix (placed) |
| 17 | ` <-- try again!` | score suffix (not placed) |

## Entity struct (80B, 20 dwords) — inferred from code

| off | field |
|---|---|
| +0x00 | next (active list) |
| +0x04 | group partner / next-in-group |
| +0x08 | group prev (walked by `game_group_head`) |
| +0x0c | prev (active list) |
| +0x10 | sprite column (short) |
| +0x11 | mode (short; 0x0b upright, 0x11 crouched) |
| +0x14 | sprite column-array pointer (into `c5f8` table) |
| +0x18 | type (0-17) |
| +0x1c | frame (0-63; template default 0x40) |
| +0x20..0x2b | world rect {x1, y1, x2, y2} (rect-cache flag 4 at +0x4c) |
| +0x2c.. | group bounding box |
| +0x40..0x44 | sprite geometry shorts (width/height/offset for `game_entity_rect_calc`) |
| +0x42 | position short (player: y; distance = y/16 meters, per `%5.2dm` format) |
| +0x46 | anim state |
| +0x48 | distance/anim |
| +0x4c | flags byte: 1=in-list, 2=group, 4=rect-cached, 8=dead, 0x10=in-group, 0x20=col-changed |

Template (zero-init, +0x1c=0x40) at `c030`; pool 100×80B at `c648`.

## Extracted resources (M1 — 89 sprites + string tables → `web/assets/`)

`harness/extract_resources.py` pulls all 89 RT_BITMAP sprites and both RT_STRING groups out of
`original/ski32.exe` and writes `web/assets/sprites/bmp_NNN.png` (ids 1-89) +
`web/assets/resources.json` (per-bitmap `{w, h, bpp, file}` + both string groups). Verified
89/89, exit 0.

### DIB format (as stored in `.rsrc`)

Every leaf is a 4-bit DIB, but **not** stock BMP:
- 40-byte `BITMAPINFOHEADER` (`biSize=40`, `biCompression=0`, `biPlanes=1`, `biBitCount=4`,
  `biSizeImage=0`, `biClrUsed` = 16 on 68 entries / 0 on 21).
- Palette: **16 × 4-byte RGBQUAD** (64 bytes; the standard VGA 16-color palette, reserved byte 0) —
  not 3-byte RGBTRIPLE.
- Pixel rows 4-byte aligned, stored **bottom-up** (all heights positive); rows MSB-first.
- Payload = `40 + 64 + rowsz·h`, plus a trailing 0/8-byte pad (8 bytes on exactly 3 entries).
- Leaf data entries in this PE store the **file offset** (not an RVA) — the extractor validates
  each of three candidate offsets (file-off, `+base`, RVA-mapped) against the header + size.

### RT_STRING groups

Format: length-prefixed UTF-16LE entries `[u8 len][u8 pad][2·len chars]` — **not** a stock
STRINGTABLE (no leading count). Group 1 terminates after 16 entries; group 2 after 2 (then zero pad).

- **Group 1** (16 entries, first empty): `''`, `SkiFree`, `Ski Paused ... Press F3 to continue`,
  `Time:`, `Dist:`, `Speed:`, `Style:`, `00:00:00.00`, ` 0000m`, ` 0000m/s`, `0000000`,
  `%2u:%2.2u:%2.2u.%2.2u`, `%5.2dm`, `%5.2dm/s`, `%7ld`, `High Scores`
- **Group 2** (2 entries): ` <-- that's you!`, ` <-- try again!`

(17 distinct named strings total; the score suffixes live in group 2, the UI/format strings in
group 1. This matches the 17-string `.rdata` STRINGTABLE documented above, which is the same set.)

### Sprite inventory (89)

Frame-set membership is from the runtime `frame_col_table` (`a1ac`, 64×u16 frame→column) cross-
checked against `spawn_frame_table` (`a22c`, type 0-10 → initial frame), `game_sprite_frame`
(type 11-16 → frame), and `game_startpoles_spawn` (type 0x11 → **direct** columns 53-56 via
`game_entity_new_col`). Entity types 0-17; type 0 = player, 0x12 = no-spawn. Identity is
visual (see `web/assets/sprites/`). "col" = bitmap id = `LoadBitmapA` id.

| id | w×h | bpp | identity | notes (frame-set membership) |
|---|---|---|---|---|
| 1 | 16×32 | 4 | player, rear view standing | **player set = cols 1-22 (frames 0-21)**; back/start view, T-arms, dashed box |
| 2 | 16×32 | 4 | player, side right, skis up | turning/airborne (skis tilted) |
| 3 | 24×28 | 4 | player, side right, crouch | turning (skis tilted) |
| 4 | 24×28 | 4 | player, side right, glide | skis flat |
| 5 | 16×32 | 4 | player, side left, skis up | turning/airborne |
| 6 | 24×28 | 4 | player, side left, crouch | turning (skis tilted) |
| 7 | 24×28 | 4 | player, side left, glide | skis flat |
| 8 | 24×28 | 4 | player, side left, glide | skis flat |
| 9 | 24×28 | 4 | player, side right, glide | skis flat |
| 10 | 24×28 | 4 | player, side right, glide | skis flat |
| 11 | 24×28 | 4 | player, side right, deep crouch | skis flat |
| 12 | 32×32 | 4 | "OUCH!" crash burst | hat+skis+poles flying, snow spray (frame 11) |
| 13 | 32×24 | 4 | player crashed (fallen) | on back, skis crossed |
| 14 | 32×32 | 4 | player arms-up (victory) | legs apart, skis crossed |
| 15 | 28×31 | 4 | player arms-up (victory) | skis flat |
| 16 | 28×31 | 4 | player arms-up (victory) | mirrored of 15 |
| 17 | 28×34 | 4 | player rear view arms-up | skis vertical, snow spray |
| 18 | 32×26 | 4 | player tumbling | mid-flip crash |
| 19 | 32×32 | 4 | player stuck in gate | caught between 2 poles, up/down arrows |
| 20 | 31×24 | 4 | player somersault | upside-down crash |
| 21 | 25×31 | 4 | player crouch turning, right | arm raised |
| 22 | 25×31 | 4 | player crouch turning, left | arms raised |
| 23 | 12×24 | 4 | arrow signpost, left | blue sign, white left arrow, on pole; static |
| 24 | 12×24 | 4 | arrow signpost, right | red sign, white right arrow, on pole; static |
| 25 | 12×24 | 4 | smiley signpost (good) | green smiley on pole; static |
| 26 | 12×24 | 4 | frown signpost (bad) | blue angry face on pole; static |
| 27 | 64×32 | 4 | snowdrift band | faint gray humps; background tile; static |
| 28 | 24×30 | 4 | AI skier crouch, right | **AI-skier set = cols 28-32 (frames 22-26)**, type 1; magenta suit, yellow face |
| 29 | 21×29 | 4 | AI skier crouch, right | |
| 30 | 21×29 | 4 | AI skier crouch, right | |
| 31 | 24×24 | 4 | AI skier crashed | fallen, hat off |
| 32 | 24×24 | 4 | AI skier tumbling | mid-flip |
| 33 | 21×15 | 4 | dog walking, right | **dog set = cols 33-36 (frames 27-30)**, type 2; gray |
| 34 | 21×15 | 4 | dog walking, right | |
| 35 | 19×19 | 4 | dog "WOOF!" | barking, text |
| 36 | 19×19 | 4 | dog "WOOF!" | |
| 37 | 26×30 | 4 | snowboarder riding | **snowboarder set = cols 37-44 (frames 31-38)**, type 3; green shirt, red board |
| 38 | 20×30 | 4 | snowboarder crouch | |
| 39 | 25×31 | 4 | snowboarder turning | |
| 40 | 30×29 | 4 | snowboarder flipping | |
| 41 | 32×32 | 4 | snowboarder crash | board angled |
| 42 | 32×32 | 4 | snowboarder flip | board up |
| 43 | 25×29 | 4 | snowboarder flipping | |
| 44 | 29×25 | 4 | snowboarder horizontal | |
| 45 | 23×11 | 4 | rock (boulder) | gray speckled; static |
| 46 | 16×11 | 4 | rock (mossy) | gray w/ cyan-green moss; static |
| 47 | 16×4 | 4 | snowdrift (small) | faint gray outline; background |
| 48 | 24×8 | 4 | snowdrift (large) | faint gray outline; background |
| 49 | 28×32 | 4 | green pine (berries) | **green-pine set = cols {49,87,88,89} (frames 60-63)**, type 10 |
| 50 | 32×24 | 4 | bare fir (winter) | dark blue leafless; not in frame table |
| 51 | 32×64 | 4 | large green pine | not in frame table |
| 52 | 32×8 | 4 | rainbow banner | horizontal stripes (blue/green/red/magenta); static |
| 53 | 93×57 | 4 | "Ski Free" logo | Copyright 1991 by Chris Pirih; **start banner** (type 0x11, col 53) |
| 54 | 52×10 | 4 | "Version 1.04" | start banner (type 0x11, col 54) |
| 55 | 92×30 | 4 | "Use NumPad [0-9]…" | start banner (type 0x11, col 55) |
| 56 | 63×32 | 4 | "F2 = Restart, F3 = Pause" | start banner (type 0x11, col 56) |
| 57 | 42×27 | 4 | "Start" sign, right | green, right arrow; static |
| 58 | 42×27 | 4 | "Start" sign, left | green, left arrow; static |
| 59 | 50×29 | 4 | "Finish" sign, right | blue/purple checkered, right arrow; static |
| 60 | 50×29 | 4 | "Finish" sign, left | blue/purple checkered, left arrow; static |
| 61 | 40×36 | 4 | "Slalom" sign | yellow-green, diagonal arrow; static |
| 62 | 44×36 | 4 | "Tree Slalom" sign | cyan, diagonal arrow; static |
| 63 | 40×35 | 4 | "Free-style" sign | green, down arrow; static |
| 64 | 24×64 | 4 | tall gate pole | black T-pole, blue emblem, cyan base; static |
| 65 | 26×32 | 4 | bench: AI skier + snowboarder | **bench/gate set = cols 65-67 (frames 39-41)**, type 4; 3-frame exit (65→66→67) |
| 66 | 26×32 | 4 | bench: AI skier | |
| 67 | 26×32 | 4 | bench: empty | |
| 68 | 32×48 | 4 | yeti arms-up | **yeti set = cols 68-81 (frames 42-55)**; types 5-8 (+13, 16) share it; front view |
| 69 | 32×48 | 4 | yeti arms-up (shouting) | |
| 70 | 32×48 | 4 | yeti walking, front | |
| 71 | 32×48 | 4 | yeti walking, front | |
| 72 | 32×48 | 4 | yeti walking, front | |
| 73 | 32×48 | 4 | yeti walking, front | |
| 74 | 32×48 | 4 | yeti walking, back | |
| 75 | 32×48 | 4 | yeti walking, back | |
| 76 | 32×48 | 4 | yeti grappling player | red-suited skier (type 13 → cols 75-77) |
| 77 | 32×48 | 4 | yeti grappling player | |
| 78 | 32×48 | 4 | yeti grappling player | (type 16 → col 78) |
| 79 | 32×48 | 4 | yeti, pole overhead | crossbar + green (startpole frame 53) |
| 80 | 32×48 | 4 | yeti, chest mark | (startpole frame 54) |
| 81 | 32×48 | 4 | yeti, chest mark | (startpole frame 55) |
| 82 | 16×8 | 4 | snow patch | small cyan blob; static |
| 83 | 22×27 | 4 | frosted fir (blue) | **frosted-fir set = cols 83-85 (frames 56-59→83,84,85,84)**, type 9; 3-sprite sway |
| 84 | 22×27 | 4 | frosted fir (blue) | |
| 85 | 22×27 | 4 | frosted fir (blue) | |
| 86 | 8×11 | 4 | snowflake/plant | small blue bloom on stem; static |
| 87 | 28×32 | 4 | green pine | green-pine set (frames 61-63) |
| 88 | 28×32 | 4 | green pine | |
| 89 | 28×32 | 4 | green pine | (type 9 start frame 56 → col 83; type 10 start frame 60 → col 49) |

**Set summary** — player 1-22 (22), arrow-signs 23-24, smiley-signs 25-26, snowdrift-band 27,
AI-skier 28-32 (5), dog 33-36 (4), snowboarder 37-44 (8), rocks 45-46, snowdrifts 47-48,
trees {49,50,51} + frosted-fir 83-85 + green-pine 87-89, banner 52, start-banners 53-56,
course-signs 57-63, tall-pole 64, bench 65-67 (3), yeti 68-81 (14), snow-patch 82, snowflake 86.

**Extraction stats:** 89/89 bitmaps, all 4-bpp (1- and 8-bpp present in the format contract but
zero entries use them). 44 unique w×h sizes; PNG output 102-1178 bytes each, 31,288 bytes total.
`git ls-files web/assets` = 90 tracked files (89 PNGs + `resources.json`).


## Data model

Authoritative, disassembly-verified data model (objdump -d on `original/ski32.exe`).
Where the Ghidra C and the disassembly disagree, the disassembly wins — see
*Ghidra offset mis-render* below. Positions are in **1/16-m** units (1 m = 16 units);
`Dist:` on the status panel is `y/16` meters.

### Ghidra offset mis-render (read this first)

Ghidra's decompiler shows two overlapping byte-views of the entity struct. In the
"byte view" the player position fields are rendered as `+0x10/+0x11/+0x12`, but the
disassembly proves the real offsets are `+0x40/+0x44/+0x48`:

- `game_entity_step` 0x402be0: `mov bx,[esi+0x40]; mov bp,[esi+0x42]; mov di,[esi+0x44];
  add bx,[esi+0x46]; add bp,[esi+0x48]; add di,[esi+0x4a]`
- `game_entity_set_pos` 0x402390 tail: `mov [esi+0x42],cx (y); mov [esi+0x40],bp (x);
  mov [esi+0x44],dx (mode)`
- `game_entity_set_col` 0x402180: `cmp di,[esi+0x10] ... mov [esi+0x10],di; mov [esi+0x14],c5f8+col*0x10`
- `game_entity_rect` 0x401410: `cmp WORD [esi+0x10],0; ax=[esi+0x10]; ax<<4; +c5f8 == [esi+0x14]`
- WM_CHAR handler 0x406780: `mov ax,[ecx+0x44]; mov dx,[ecx+0x42]; mov dx,[ecx+0x40]`

So wherever decompiled C says `*(short*)(x + 0x10/+0x11/+0x12)` on a **player** position
access, the true field is `+0x40 (x) / +0x44 (mode) / +0x48 (speed)`. The genuine `+0x10`
field (sprite column, u16) is separate and real (proven by the 0x402180/0x401410
disassembly above). The older "inferred" entity table above this section is superseded.

### Entity struct (80B = 20 dwords)

Pool: `c648` = LocalAlloc(8000) (zero-filled), 100 slots × 80B; freelist `c744` (100
nodes chained through `+0x00`, built by `game_freelist_init` 0x404a00); active list head
`c618`. Allocation 0x402280: `rep movsd` of 16 dwords from the zero template `c030`,
clears `+0x0c`, head-insert (param 0) or splice (param 1).

| off | field | evidence |
|---|---|---|
| +0x00 | next (active list) | reaper 0x401390 `*puVar2 = *puVar1`; allocator |
| +0x04 | group partner (bidirectional dword; 0 = alone) | `game_group_split` 0x402220; die 0x401350 clears partner's back-link |
| +0x08 | group chain next | draw 0x401540 walks to group head |
| +0x0c | gate descriptor ptr (36B rec; 0 = not gate-spawned) | spawn 0x404130 sets `entity[3]=desc`; reaper: `if (e[3]) e[3]->entity = 0` |
| +0x10 | sprite column (u16, 1..0x59, must be ≠ 0) | set_col 0x4021c6 `mov [esi+0x10],di`; rect 0x40143c asserts nonzero |
| +0x12 | (unused padding — never written; decompiled `+0x12` on player = real `+0x48`) | see mis-render |
| +0x14 | sprite column table entry ptr (`c5f8 + col*0x10`) | set_col 0x4021da `mov [esi+0x14],eax`; rect 0x401466 asserts consistency |
| +0x16 | (unused padding) | — |
| +0x18 | type (u16: 0..0x11; 0x12 = no-spawn sentinel, not a type) | alloc: `puVar1[6] = type`; activate 0x4028e0 `switch(param_1[6])`; 0x402912 `mov eax,[esi+0x18]` |
| +0x1c | frame (u16, 0..0x3f asserted by set_frame 0x402120) | anim_update 0x403430 asserts `*(int*)(+0x1c) == row[6]` |
| +0x20..+0x27 | world/camera-space rect {x1,y1,x2,y2} (4×s16) | rect_calc 0x4014b0 writes 4 shorts; world_shift 0x402470: `e[8]/e[9]/e[10]/e[0xb] -= dx/dy` = +0x20/+0x24/+0x28/+0x2c |
| +0x28..+0x3f | (rect part 2 / screen-space; copied by template, used by draw 0x401540) | — |
| +0x40 | x (world, 1/16 m) | step disasm; set_pos |
| +0x42 | y (world, 1/16 m; distance) | step disasm; `Dist:` = y/16 |
| +0x44 | mode (u16: 0 = upright, >0 = crouched depth) | set_pos writes it; step `di = +0x44 + +0x4a` |
| +0x46 | steer velocity (s16, clamped ∓8) | step `add bx,[esi+0x46]`; input writes ±8 |
| +0x48 | speed (s16, y-velocity per tick; 0..24) | step `add bp,[esi+0x48]`; anim_update 0x403430 computes it; `Speed:` = `+0x48*1000/(c5f4<<4)` |
| +0x4a | transition counter (s16; ramps mode up/down) | step: `if (di>0) { +0x4a--; set_pos(mode=di) } else { +0x4a=0; set_pos(mode=0) }` |
| +0x4c | flags (u32) — see below | — |

### Entity flags (+0x4c, u32)

| bit | meaning | evidence |
|---|---|---|
| 0 (0x1) | group-duplicate mark (draw-order dedup) | draw 0x401540 sets/tests; set_pos clears (0x40244e `and ecx,0xfffffffb`); split: orig `&=~1`, copy `\|= 2` |
| 1 (0x2) | split copy / duplicate to be culled after a member draws | split 0x402220 sets on clone; render pass 4 (0x401060:44,109) `if (flags&2) game_entity_die` |
| 2 (0x4) | world rect cached (valid) | rect_calc asserts clear before (0x401427 `test [esi+0x4c],4`), sets after (0x401498 `or al,4`); 0x406060/0x402470 clear on camera change |
| 3 (0x8) | dead | die 0x401350 `flags \|= 8`; reaper 0x401390 reaps `flags&8`, clears `c72c`/`c64c` if it was the player |
| 4 (0x10) | in group | merge 0x401a60 |
| 5 (0x20) | pos/col changed this tick (rect pair needs refresh) | set_pos always sets (0x402449 `or al,8; shl 2` → 0x20); set_col 0x4021f7 `or edx,0x20`; physics pass 1 clears at tick start (0x401e50 `&= 0xffffffdf`); collision pair-test only when either has 0x20 |
| 6 (0x40) | special column (col ∈ {0x1b=27 snowdrift band, 0x52=82 snow patch}) | set_col 0x4021f7: `bit6 = util_frame_special(col)` (0x402310: `v==0x1b \|\| v==0x52`) |

### Sprite column table (`c5f8`, 90 × 16B = 0x5a0 bytes)

Built by `game_sprites_load` 0x405ab0 from the 89 RT_BITMAPs (ids 1..0x59; index 0
unused). Entry layout (disasm-verified):

| +0x00 | +0x04 | +0x08 | +0x0a | +0x0c | +0x0e |
|---|---|---|---|---|---|
| image DC | mask DC | yOff (u16) | width (u16) | height (u16) | area = w·h (u16) |

`game_entity_rect_calc` 0x4014b0 reads `entry+0x0a` (w) and `entry+0x0c` (h);
`game_entity_set_col` reads `entry+0x0e` (area) for the on-screen area budget `c6fc`.

### Gate descriptor (36B = 9 dwords)

Pool `c758` = LocalAlloc(0x2400) (256 slots); count `c702` (max 0x100 asserted by
`game_gate_list_add` 0x405120). The five lists (`c630` SS, `c5e0` GS, `c658` FS,
`c738` tall poles, `c720` **moving**: benches + yetis) are each a
`{first, end, scan-cursor}` pointer triple into the contiguous `c758` slot array;
list_add takes slot `c758 + c702*0x24`, asserts `[list+4]` equals it (0xa20) and
then advances `[list+4]` by 0x24 — the single global counter `c702` is shared by
all five lists. Only `c720` is stepped every
tick by `game_gate_update` 0x4040a0; the other four are touched only by
`game_gate_scan` 0x4046e0 (spawn) — so gate_step's per-tick `set_frame(entity,
desc+0x10)` applies **only** to benches/yetis.

| off | field | evidence |
|---|---|---|
| +0x00 | entity ptr (NULL until spawned) | gate_step 0x40421f `edi=[esi]`; list_add `*desc = 0` |
| +0x04 | col entry ptr = `c5f8 + low16(+0x08)*0x10` | list_add 0x405120: `desc[1] = (ushort)desc[2]*0x10 + c5f8` |
| +0x08 | col (u16) | list_add 0x4051bd `mov 0x8(%ebx),%ax` feeds the col-entry ptr; level_init writes it **directly** (signposts 23/24, banners 57-63, pines 64); 0 for benches/monsters → frame path (gate_spawn 0x404187) |
| +0x0c | type (u16: 0xc = SS/GS signpost, 0xd = pine/pole, 0x11 = start/finish banner, 4 = bench, 5-8 = yeti) | gate_step 0x4041e7/0x4041f2: `eax=[esi+0xc]; cmp 4 / cmp 5..8` |
| +0x10 | frame (u16) | used only when col == 0 (gate_spawn 0x404187 `alloc_by_type_frame(type, desc+0x10)`); gate_step `set_frame(entity, desc+0x10)` every tick for c720 gates; level_init **never writes it** (stack garbage for col ≠ 0 gates — harmless, those lists are never stepped) |
| +0x14 | x | gate_step 0x4041d7-0x4041df: `[esi+0x14] += [esi+0x1a]` |
| +0x16 | y | `[esi+0x16] += [esi+0x1c]` |
| +0x18 | z (mode passed to set_pos; benches carry 32) | `[esi+0x18] += [esi+0x1e]` |
| +0x1a | vx (steer) | 0x4041d7 |
| +0x1c | vy | 0x4041db/0x4041ea |
| +0x1e | fdelta (z velocity; yeti chase decay) | 0x4041e3; cruise: `dec [esi+0x1e]` while chasing |
| +0x20 | timestamp (u32; wake-anim start, set to `c698` on yeti-kill) | cruise `edi=[esi+0x20]; eax = c698 - edi` |

`game_gate_set_col` 0x403130: `desc->col = col; desc->+0x04 = c5f8+col*0x10; if
spawned: game_entity_set_col(entity, col)`. Style gate cursors: `c94c` = first SS
signpost descriptor, `c950` = first GS signpost descriptor (set in level_init).

### Level layout (`game_level_init` 0x404b50; all y in 1/16 m)

Static arrays (spawned by `game_gate_scan` when they enter the extended view):

(Values in 1/16-m position units; ÷16 = meters. Disasm-verified, 0x404b50-0x405100.)

- **SS course (c630):** banner type 0x11 **col 61** "Slalom" at
  (x = max(c6b0−c704+c640+0x3c, −320 (0xFEC0)); y = c if c ≤ 0x280 else 0x208,
  c = c6bc−c5fc+c5f2−0x3c); "Start" signs type 0x11 **cols 57/58** at (−576 (0xFDC0),
  0x280) / (−320 (0xFEC0), 0x280) [40 m]; **signpost gates type 0xc**, alternating
  x = −496 (0xFE10, col 23) / x = −400 (0xFE70, col 24), y = 0x3c0 .. step
  0x140 while < 0x21c0 (**24 gates, 60..520 m, step 20 m**; disasm 0x404c40-0x404c72:
  `neg %eax; sbb %eax,%eax; and $0xa0,%al; add $0xfffffe70`, esi toggles 1/0 each
  iteration — esi = 1 → 0xFFA0+0xFE70 = 0xFE10 = −496 with col 23, esi = 0 →
  0xFE70 = −400 with col 24); "Finish" signs type 0x11
  **cols 59/60** at (−576 (0xFDC0), 0x21c0) / (−320 (0xFEC0), 0x21c0) [540 m].
  Gate cursor `c94c` = first signpost.
- **GS course (c5e0):** banner type 0x11 **col 62** "Tree Slalom" at
  (x = min(c6b8−c704+c640−0x3c, 320 (0x140)); y = c if c ≤ 0x280 else 0x208,
  c = c6bc−c5fc+c5f2−0x3c); "Start" signs type 0x11 **cols 57/58** at
  (0x140=320, 0x280) / (0x200=512, 0x280) [40 m]; **signpost gates type 0xc**,
  alternating x = −80 (0xFFB0, col 23) / x = 432 (0x1B0, col 24), y = 0x410 ..
  step 0x190 while < 0x4100 (**39 gates, 65..1015 m, step 25 m**; disasm
  0x404da5-0x404de6: `neg %edx; sbb %edx,%edx; and $0xffffffe0,%edx; add $0x1b0`
  — index 1 → 0xFFFFFE00+0x1B0 = 0xFFB0 = −80 with col 23, index 0 → 0x1B0 = 432
  with col 24). Gate cursor `c950` = first signpost. **No pine gates exist in the
  GS course** — the loop computes a type 0xd pine (col = `game_sprite_frame(0xd)`
  = rand(8) → 49/50/51, x = 0x190 + rand(0x20)) into the stack template but never
  calls list_add for it, and the following `rand(400)` result is discarded: 3
  rand() calls per gate × 39 gates = **117 RNG consumption per game_start** (must
  be reproduced for exact seed-freeze; see M1 RNG). "Finish" signs type 0x11
  **cols 59/60** at (320, 0x4100) / (512, 0x4100) [1040 m].
- **FS course (c658):** banner type 0x11 **col 63** "Free-style" at (x = 0;
  y = c if c ≤ 0x280 else 0x208, c = c6bc−c5fc+c5f2−0x3c); "Start" signs type 0x11
  **cols 57/58** at (-160 (0xFF60), 0x280) / (160 (0xA0), 0x280) [40 m];
  "Finish" signs type 0x11 **cols 59/60** at (-160, 0x4100) / (160, 0x4100) [1040 m].
- **Tall poles (c738):** type 0xd **col 64 (direct)** at x = -128 (0xFF80),
  y = -0x400 .. 0x5c00 step 0x800 (**13 poles, -64..1472 m, step 128 m**; loop
  `cmp $0x5c00; jle` includes the 23552 endpoint).

Moving list (c720, `game_gate_update` 0x4040a0 — moves every tick):

- **Benches type 4**, z = 32 (24 total): loop `si = -0x400; si ≤ 0x5c00; si += 0x800`
  (disasm 0x404fc2-0x40504f). At y = -0x400 (-64 m) one: frame 41 (x = -144 (0xFF70),
  vy = +2); at every y = 0x400 .. 0x5400 step 0x800 (64..1344 m, 11 pairs): frame 39
  (x = -112 (0xFF90), vy = -2) + frame 41 (x = -144, vy = +2); at y = 0x5c00 (1472 m)
  one: frame 39 (x = -112, vy = -2). Benches drift vertically (gate_step); 0x404290
  re-wraps them at y ≤ -0x400 (→ frame 41, x = -144, vy = +2) / y ≥ 0x5c00 (→ frame
  39, x = -112, vy = -2), and a spawned frame-39 bench spawns a snowboarder (type 3,
  frame 0x21, at the bench's x/y) when rand(1000)==0 (0x404305), then switches to
  frame 0x28.
- **Yetis (types 5-8), frame 42 (sleep):** added in order 7, 8, 5, 6 (disasm
  0x405051-0x4050eb). Type 7 at (49476 (0xC144) ≈ 3092 m, 0); type 8 at
  (16060 (0x3EBC) ≈ 1004 m, 0); type 5 at (0, **-2060 (0xF7F4) = -128.75 m**)
  (0x4050c7: `movw $0xf7f4,0x26(%esp)`); type 6 at (0, 32060 (0x7D3C) ≈ 2004 m).
  Triggers: see *Monster*.

### Windows and status panel (child-window conclusion)

**The status panel is a real child window, not a direct draw.**

- `game_create_windows` 0x4052d0:
  - main: `CreateWindowExA(0, "SkiMain", str(1)="SkiFree", 0x2cf0000
    (WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN), X=(HORZRES-min)/2, 0, min(HORZRES,VERTRES),
    VERTRES, NULL)` → `c6c8`.
  - status: `CreateWindowExA(0, "SkiStatus", &c788 (empty 64B buffer), 0x40000000
    (WS_CHILD), 0,0,0,0, parent=c6c8)` → `c624`; `ShowWindow(c624, SW_SHOW);
    UpdateWindow(c624)`.
- Classes (both registered in 0x4052d0, both `hbrBackground = c69c =
  GetStockObject(0) = WHITE_BRUSH`): `SkiMain` style 0x2023
  (CS_VREDRAW|CS_HREDRAW|CS_OWNDC|CS_BYTEALIGNCLIENT), WndProc `wproc_main`
  0x405800, icon "iconSki"; `SkiStatus` WndProc `wproc_status` 0x4068d0. The
  `button` string (a1a4) is dead .rdata (never registered).
- `wproc_status` 0x4068d0: WM_CREATE → 0x406a70 (`c6cc = GetDC(hwnd)`,
  `c664 = SelectObject(c6cc, GetStockObject(10) = OEM_FIXED_FONT)` — disasm
  0x406a95 `push 0xa`; `c664` then holds the previously-selected HGDIOBJ;
  GetTextMetricsA → `c668` tmHeight, measures RT strings 3-10 → panel widths
  `c66c` (left+right) / `c66a`); WM_SIZE → `GetClientRect → c778`;
  WM_PAINT → 0x406970 (`BeginPaint`; `FrameRect(hdc, &c778, GetStockObject(4)
  = BLACK_BRUSH)`; TextOut 4 labels RT 3-6 at x=2 advancing y by `c668` each via
  0x401e20; then `draw_status_values(paint hdc)`; EndPaint).
- Reposition: `wproc_status_reposition` 0x406890 — `nWidth = (short)c66c + 4;
  MoveWindow(c624, c6b8 - nWidth, c6b4, nWidth, (short)c66a + 4, 1)` — called from
  `wproc_main`'s WM_SIZE (0x405800:34) when `c624 != 0`. Child coords: right edge at
  client width, top at client height (panel sits at the bottom-right, height
  tmHeight+4).
- `wproc_main_destroy` 0x405ec0: ReleaseDC(c63c), game_pause, delete GDI objects
  (c620/c644/c6d4/c75c/c614 selected on DCs c710/c730/c6a4/c6ec/c5ec), DeleteDC ×5.
- NCCALCSIZE quirk (wproc_main): `rgrc[0].right = 300; rgrc[0].bottom = 320` (absolute
  screen coords, not deltas) → client size = 300×320 only when the window's top-left
  is at (0,0); otherwise smaller (M0 observed 124×309 at window pos (176,11)).

### Status panel value sources (`draw_status_values` 0x401b80)

Called from `game_tick` when `c698 - c5dc > 0x147` (327 ms; `c5dc` = last update,
stamped at the end of 0x401b80) and directly from `game_player_face` 0x402e80 at style
run end. Values (right column, x = `c66c._2_2_ + 2`, y advances tmHeight per line):

| line | source | formula |
|---|---|---|
| Time: | `util_fmt_time(c944)` (RT 11 `%2u:%2.2u:%2.2u.%2.2u`) | `c944` = **style-run elapsed ms**, written only by `game_style_ss` 0x402c60 / `game_style_gs` 0x403250 while `c95c`/`c958` is active (`c944 = c698 - c948`); zeroed by `game_reset` 0x404970 |
| Dist: | `wsprintf(RT12 "%5.2dm", sVar1/16)` | `sVar1` = player `+0x42`; if SS run active (c95c) → `0x21c0 - y`; if GS (c958) or FS (c954) run active → `0x4100 - y`; else `y` (total distance); signed round-div by 16 |
| Speed: | `wsprintf(RT13 "%5.2dm/s", v)` | `v = (+0x48 * 1000) / (c5f4 << 4)`; 0 if `c5f4 == 0` |
| Style: | `wsprintf(RT14 "%7ld", c6a8)` | style score accumulator |

**Why `Time:` stays `0:00:00.00` (under Wine and on Windows):** `Time:` is **not**
wall-clock time — it is the SS/GS style-run timer (`c944`). It only starts when the
player crosses y = 0x280 (640 = 40 m) moving +y with the interpolated crossing-point
x (`util_lerp` 0x402e30) inside the SS lane x ∈ (-577, -319) or the GS lane
x ∈ (319, 513) (`game_style_ss` 0x402c60 / `game_style_gs` 0x403250). Outside a style
lane `c944` keeps its `game_reset` zero, so the panel shows the default
`00:00:00.00` (RT string 7). The M0 Wine run simply never entered a style lane.

### Player mode model (`+0x44` / `+0x4a` / frames)

- `mode` (0x44): 0 = upright; >0 = crouched depth. `transition` (0x4a): ramp counter.
  `game_entity_step` 0x402be0 per tick: `mode_new = +0x44 + +0x4a`; if `mode_new > 0`:
  `+0x4a--; set_pos(mode = mode_new)` else `+0x4a = 0; set_pos(mode = 0)`. So crouch
  (Alt: `+0x4a = 2`, frame 0xd) ramps mode 2→1→0 over 3 ticks; mouse-button crouch sets
  `+0x4a = 4`.
- Facing frames (a308 table @ 0x40a308, 22 rows × 8×u16:
  `{accel-flag, max-speed, steer-decay, steer-window, dir-sign (0xFFFF=left,
  1=right, 0=auto), 0, frame-idx, 0}`): 0 = downhill (max 16, no window), 1-3 = left
  turn (max 12/6/0, window 1/4/8, left sign), 4-6 = right turn (same, right sign),
  7/8 = left/right lean (max 0, window 8), 9/10 = jump (max 0), 11/12 = **crash
  (accel flag 0, max 0 — frozen, terminal)**, 13-21 = crouch (max
  24/22/22/20/24/20/20/22/22).
- `game_anim_update` 0x403430 (per tick, player only via activate): speed `+0x48`
  accelerates +1/tick (only when accel flag = 1) toward the row max, decays -2/tick;
  steer is clamped to a speed-proportional window `(window × speed)/2` with ±2/tick
  decay outside it.
- **Crash (non-lethal):** collision sets frame 0xb (col 12 "OUCH!") when upright,
  frame 0x11 (col 18) when crouched; a308 next = 0 (frozen) and `wproc_main_input`
  skips all controls for frames 0xb/0x11 — the crash is terminal; only Enter/F1/F2
  recover. **Death (lethal):** only yeti contact kills the player.
- Jump: only from speed 0 (Ctrl on frames 3/7/0xc → frame 9; on 6/8 → frame 10),
  sets speed = -4 (backward hop). Crouch slide: Alt (mode 0) → `+0x4a = 2`, frame 0xd,
  speed -= 4 if > 4.

### Monster (yeti) trigger, chase, end conditions (`game_gate_cruise` 0x404350)

Per tick, per yeti descriptor (types 5-8 in c720). A **self-gate** on the yeti's
own position (0x404471-0x4044d8) decides between a per-type **sentinel velocity**
(pass → chase immediately) and the **player threshold** (fail, 0x4044dd-0x404533,
which decides trigger vs idle):

- **Self-gate (disasm 0x404471-0x4044d8)** — pass condition and sentinel:
  - type 5: `y > -2000` (0xF830) (`cmp $0xf830,%ax; jle fail`, 0x404486) → vy = -10
  - type 6: `y < 32000` (0x7D00) (0x40449b) → vy = +26
  - type 7: `x > 49440` (0xC180) (0x4044b0) → vx = -16
  - type 8: `x < 16000` (0x3E80) (0x4044c9) → vx = +16
  (the other velocity component is 0; pass falls through to apply 0x4045d8)
- **Self-gate fail (0x4044dd):** if there is no live player (`c72c == 0`) → idle
  (vx = vy = 0). Otherwise the **player threshold** (player +0x40/+0x42, disasm
  0x4044f2-0x404533):
  - type 5: `player.y < -2000` (jl) — unreachable (player y starts at 0, grows +y)
  - type 6 (yeti at y=32060): `player.y > 32000` (jg, 0x404508) — the ~2000 m rule
  - type 7 (yeti at x=49476): `player.x < 49440` (jl) — true for the whole course
  - type 8 (yeti at x=16060): `player.x > 16000` (jle → idle, i.e. strictly >) —
    unreachable
  Threshold met → **trigger** (0x404539); else apply idle (vx = vy = 0).
- **On trigger (0x404539-0x4045d3):** dx_true = player.x − yeti.x, dy_true =
  player.y − yeti.y. Teleport only where the true separation exceeds a full view
  (viewW = `(short)c5f0`, viewH = `(short)c6d8`), independently per axis:
  |dx_true| > viewW → yeti.x = player.x ∓ viewW; |dy_true| > viewH → yeti.y =
  player.y ∓ viewH. Then `vx = clamp(dx_true, ±16)`, `vy = clamp(dy_true, ±26)`
  (0x40459c-0x4045c3), `fdelta = 1` (0x404617), `snd_play(c6f0)` (wave pair id 9 —
  silent, no WAVE resources exist). The "velocity-mixing" code at 0x4045f4-0x404613
  (ratio-rescales of desc+0x1a/+0x1c) is **dead** — the unconditional stores at
  0x404621/0x404628 overwrite it.
- **Frame selection on apply (0x40462c-0x4046c2):** walking frames toggle between
  a 2-frame pair each tick (result = N if desc+0x10 was already N, else N+1):
  vy < 0 → 48/49; vx < 0 → 44/45; vx > 0 || vy > 0 → 46/47 (live chase path);
  idle (0,0) → rand(10) (0x404677): 90% frame 42 (fdelta 1), 10% frame 43
  (fdelta 4).
- **Chase is continuous, not one-shot** — the trigger re-fires every tick while
  the player threshold holds:
  - **type 6** (spawn (0, 32060)): self-gate fails at spawn (32060 ≥ 32000) →
    dormant (idle bob) until the player passes y = 32000 (2000 m). Thereafter the
    threshold (`player.y > 32000`) stays true for the rest of the run, so the
    trigger re-arms every tick; |dy_true| is normally < viewH, so usually no
    teleport — the yeti just re-clamps to vy = ±26 / vx = ±16 (sign of the true
    separation, so it wraps around and tracks the player's y/x). If the player
    pulls a full view clear, it teleports to player.y − viewH and
    chases at +26/tick (faster than the player's max 24/tick).
  - **type 7** (spawn (49476, 0)): self-gate **passes** at spawn (49476 > 49440) →
    walks left at vx = -16 from the first tick; after ~2 ticks (x ≤ 49440) the
    self-gate fails and the trigger (player.x < 49440, always true) re-fires every
    tick — while |dx_true| > viewW it teleports the yeti to player.x + viewW, then
    chases at 16/tick (2× the player's max steer 8/tick). It permanently
    reappears one view to the right of the player.
  - **type 5** (spawn (0, −2060)): self-gate fails (−2060 ≤ −2000) and the trigger
    (`player.y < −2000`) can never fire → **permanently dormant** (idle-bobs at
    −128.75 m, 3.75 m above the −2000 threshold).
  - **type 8** (spawn (16060, 0)): self-gate fails (16060 ≥ 16000) and the trigger
    (`player.x > 16000`) can never fire → **permanently dormant**.
- **Idle bob:** vx = vy = 0 → z oscillates via fdelta (gate_step integrates
  z += fdelta each tick; cruise decrements fdelta while z > 0, else zeroes z/fdelta
  at 0x404378-0x40438c): fdelta 1 → z: 0,1,1,0 (4-tick cycle); fdelta 4 (frame 43)
  → parabolic z peaking at 10 over 10 ticks. Airborne (z ≠ 0) skips all ground
  logic (0x404390 → frame store + ret at 0x4046b8).
- **Wake-up animation (LIVE post-kill states)** — on a kill, `game_collide` sets
  desc+0x10 = 0x32 (50) and desc+0x20 = c698 (GetTickCount at death); states 50-55
  then run (0x404396-0x404470, jump table 0x4046c4; elapsed = c698 − desc+0x20 ms,
  so the thresholds are real milliseconds):
  50 →(immediate)→ 51 →(≥ 500 ms, else loops back to 50)→ 52 →(> 700 ms)→ 53 →
  (> 1000 ms)→ 54 →(immediate)→ 55 →(≥ 3000 ms, else loops back to 54)→ 42 (sleep).
- **Kill** (`game_collide` 0x403a00, case 5-8, `param_2 == c72c`): `snd_play(c6e0)`;
  group-split if flagged; **`game_entity_die(player)`**; yeti reset: desc frame = 0x32
  (50, wake anim), steer = 0, vy = 0, entity speed = 0, desc+0x20 = c698.

**End conditions (complete list):**

1. **Player killed (yeti contact)** — only lethal path: reaper 0x401390 clears
   `c72c` and `c64c`; the world keeps running; all input handlers no-op
   (gated by `c72c`); Enter or F1 → `game_restart` 0x406500 (`game_reset` +
   `game_start`); Esc minimizes; F2 toggles the title.
2. **Player crash (any other collision)** — terminal frozen state (frame 11/17);
   same recovery keys.
3. **Style run completion** (not game end): SS at y > 0x21c0 (540 m) →
   `score_show("SS", c944, is_time=1)`; GS at y > 0x4100 (1040 m) →
   `score_show("GS", c944, is_time=1)`; FS at y > 0x4100 (1040 m) →
   `score_show("FS", c6a8, 0)`.
   Each pops the modal "High Scores" MessageBox (the game keeps ticking under the
   modal loop — no pause call around it).
4. There is **no finish line** — the course is infinite; only the yetis, crashes, or
   the user end a run.

### High-score INI (`entpack.ini`)

`score_show` 0x402ec0, called with (key, value, is_time): key ∈ {"SS" (c0d8), "FS"
(c0f4), "GS" (c0f8)}; times are negated before storage (`if (is_time) value =
-value`).

- **File:** `entpack.ini` (c084) in the CWD; **section** `[Ski]` (c080).
- **Read:** `GetPrivateProfileStringA("Ski", key, "" (c788), buf, 0x100, "entpack.ini")`.
- **Format:** one line of space-separated signed longs, each entry emitted as
  `"%ld "` (c0ec) — e.g. `SS=-12345 -13000 ` (single spaces, trailing space; no
  names); max **10** entries, **ascending** in stored value (times stored negated
  → index 0 = fastest time).
- **Parse:** skip spaces, read until space/NUL, `_atoi` (0x406cf8), ≤10 values;
  each token is a value (no name handling).
- **Insert:** first index i where `stored[i] > new` (stored ascending). Fewer than
  10 stored → plain insert (count++); exactly 10 stored → shift right, drop the
  last, insert; new value worse than all (index 10) → **did not qualify** — nothing
  stored, dialog shows "try again".
- **Write:** `WritePrivateProfileStringA("Ski", key, buf, "entpack.ini")`.
- **Dialog:** MessageBoxA(owner = c6c8, body, "High Scores" (RT 15), 0); body =
  entries joined by `\n` (0x0a); times formatted with `util_fmt_time(-value)`, FS as
  `"%9ld"` (c0e4); the new entry gets RT 16 `" <-- that's you!"`; non-qualifying runs
  append `"\n\n"` (c0dc) + the new time + RT 17 `" <-- try again!"`.

### Input: exact key map and mechanism (Task 14 contract)

**Mechanism: 100% message-consumed. There is NO key-state global and NO
`GetKeyState`/`GetAsyncKeyState`/`ToAscii`/`MapVirtualKey` anywhere in the binary
(grep of all 168 decompiled functions + import table = zero hits).** Every key event
is handled exactly once, at message time, by the WndProc; nothing is polled between
ticks. For Task 14 injection, synthesize WM messages (or call the handlers directly):
`wproc_main` 0x405800 dispatches, gated by `c67c` (game active).

WM_KEYDOWN (0x406170) — `wParam` = VK code:

| VK | key | action |
|---|---|---|
| 0x0D | Enter | player present → ignored; no player → falls into F1 = `game_restart` |
| 0x71 | F1 | `game_restart` (0x406500) |
| 0x1B | Esc | `ShowWindow(c6c8, SW_MINIMIZE=6)` |
| 0x72 | F2 | `game_pause_toggle` (0x405760) |
| 0x21 | Up | if mode == 0 → frame 6 |
| 0x22 | Right | if mode == 0 → frame 4 |
| 0x23 | Left | if mode == 0 → frame 1 |
| 0x24 | Down | if mode == 0 → frame 3 |
| 0x25 | Space | left action: `a = a258[frame]`; if a == 7 → steer = clamp(steer-8, -8); else frame = a (facing rotate 0→1→2→3) |
| 0x27 | RShift | right action: `a = a25c[frame]`; if a == 8 → steer = clamp(steer+8, +8); else frame = a (facing rotate 0→4→5→6) |
| 0x26 | Ctrl | frame state machine: 3/7/0xc → 9 (only if speed == 0, then speed = -4); 6/8 → 10 (same); 0xd→0x12; 0xe→0x14; 0xf→0x15; 0x12→0x13; 0x13→0xd |
| 0x28 | Shift | if mode == 0 → frame 0; else 0xd→0x12; 0x12→0x13; 0x13→0xd; 0x14→0xe; 0x15→0xf |
| 0x2D | Alt | if mode == 0: `+0x4a = 2`, frame 0xd, speed -= 4 if > 4 (crouch) |

The lowercase-ASCII cases in the same switch (0x41-0x79 range: `I C A G D H F B `)
are **dead** — `wParam` of WM_KEYDOWN is a VK (letters are 0x41-0x5A), so they never
match. After any action: if the frame changed, `set_frame` + `if (c610) { render;
c610 = 0; }`.

WM_CHAR (0x406780) — debug keys:

| char | action |
|---|---|
| 'X' (0x58) | set_pos(player, x-2, y, mode) |
| 'Y' (0x59) | set_pos(player, x, y-2, mode) |
| 'x' (0x78) | set_pos(player, x+2, y, mode) |
| 'y' (0x79) | set_pos(player, x, y+2, mode) |
| 'f' (0x66) | `c670 = !c670` (double-step / turbo toggle — physics takes 2 steps/tick) |
| 'r' (0x72) | `game_render` |
| 't' (0x74) | `game_tick` (manual tick, independent of the timer) |

Mouse (all gated by `c67c`):

- WM_MOUSEMOVE (0x406550): dx = x - center, dy = y - center; upright →
  `util_facing_delta` 0x4065e0; crouched → `util_facing_crouch` 0x406670 → set frame.
- WM_LBUTTONDOWN / WM_RBUTTONDOWN (0x4066d0, same handler): no player →
  `game_restart`; else frame 0xb (crashed) → nothing; mode 0 → `+0x4a = 4`, frame 0xd
  (crouch); else crouch-frame rotate (0xd→0x12, 0xe→0x14, 0xf→0x15, 0x12→0x13,
  0x13→0xd).
- WM_MOUSEACTIVATE: returns MA_ACTIVATEANDEAT(2) when it would be MA_NOACTIVATE(1)
  (quirk: triggered when mouse screen-x == 1).

### Spawn zones and type pickers (`game_spawn` 0x4025c0)

For each 0x3c (60 unit = 3.75 m) of camera travel, `game_physics` 0x401e50 (tail,
disasm order) calls `game_spawn` for the crossed band edge, cursors `c5d8` (Y) /
`c714` (X) stepped by ±0x3c: cursor Y > 0 → dir 3 (bottom band, y = H + 0x3c),
cursor Y < -0x3c → dir 2 (top band, y = -0x3c), cursor X > 0 → dir 1 (right band,
x = W + 0x3c), cursor X < -0x3c → dir 0 (left band, x = -0x3c). Position:
`game_spawn_pos` 0x4024f0 (player-relative base + edge offset +
`rand(c6d8)`/`rand(c5f0)` across the band length). Zone of the **spawn position** selects the
picker; the area budget `c6fc` (total on-screen sprite area) gates each picker:

| zone (spawn pos) | picker | rule |
|---|---|---|
| SS lane: x ∈ [-576, -320] and y ∈ [640, 8640] ([40, 540] m) | `game_spawn_pick_speed` 0x402770 | deterministic: `c6fc <= c748/64` → type 11 (0xb), else none |
| GS lane: x ∈ [320, 512] and y ∈ [640, 16640] ([40, 1040] m) | `game_spawn_pick_narrow` 0x4027a0 | `c6fc > c748/16` → none; else r = rand(64): r == 0 → type 2 (dog) [1/64], else type 0xd (13, pine) [63/64] (`neg %ax; sbb %eax,%eax; and $0xb,%eax; add $0x2,%eax`, 0x4027be-0x4027d3) |
| FS lane: x ∈ [-160, 160] and y ∈ [640, 16640] ([40, 1040] m) | `game_spawn_pick_mid` 0x4027e0 | `c6fc > c748/32` → none; else r = rand(100): <2 → 10, <20 → 0xd, <50 → 0xf, <60 → 0xb, <80 → 0x10 (16, banner), else 0xe (14, rock) (`cmp $0x50; sbb; and $0xfe,%al; add $0x10`, 0x402838-0x402840) |
| everywhere else (off-course) | `game_spawn_pick_wide` 0x4026f0 | `c6fc > c748/32` → none; else r = rand(1000): <50 → 10 (pine), <500 → 0xd (pine13), <700 → 0xf (drift), <750 → 0xb, <950 → 0xe (rock), <970 → 0x10 (banner), <990 → 0x12 (18, no spawn), ≥990 → 2 (dog) — final band is `cmp $0x3de; sbb %eax,%eax; add $0x2,%eax` (0x402760-0x402769): r<990 → 0x12, else 2; **no AI-skier (type 1) outcome in this picker** (disasm-verified 2026-08-25) |

Type 0xa (10) is the green pine (berries) — a22c (type→initial frame, types 0-10:
[6,22,27,31,39,42,42,42,42,56,60]) gives frame 60 → col 49. Type 0xb (11) is **not**
a tree: it is the snowdrift-band background tile (col 27 — via the table below,
a22c only covers types 0-10). Types ≥ 0xb get their column from
`game_sprite_frame` 0x402850 (jump table 0x4028c8: 0xb → col 27; 0xc → assert 0x623
(unreachable); 0xd → rand(8): 0→50 (bare fir), 1→51 (large green pine), else 49
(green pine); 0xe → rand(4): 0→46 (mossy rock), else 47 (small snowdrift);
0xf → rand(3): 0→48 (large snowdrift), else 49 (green pine); 0x10 → col 52
(rainbow banner)).
Finally, once per tick: `if (rand_range(0x29a) == 0)` (1/666) → spawn a snowboarder
(type 3, frame 0x1f) via 0x402350(e, 2).

### View / camera globals

- `c704` = view center X (world units), `c5fc` = view center Y — set by
  `game_set_center` 0x406060 on WM_SIZE: `c704 = (c6b0 + c6b8)/2`,
  `c5fc = (c6b4 + c6bc)/3` (disasm 0x405fb4-0x405fe9: `imul 0x55555556` high-word
  divide = /3, not the /4 the decompiler rendered). `c6b0..c6bc` are the four spawn-
  band edge vars; `GetClientRect(&c6b0)` at WM_SIZE clobbers c6b0/c6b4 (the rect
  overlays them) while c6b8/c6bc are adjacent stale globals (0 at startup → center =
  (0, W/3)).
- Camera = player position: `c640` low16 = camera X, `c5f0` high16 = camera Y —
  updated by `game_world_shift` 0x402470 on every player `set_pos` (all non-player
  entities' rects shift by the delta; the player itself is skipped). `c5f0` low16 =
  spawn band width (from WM_SIZE, potentially stale).
- Extended view (spawn/cull bounds): `c680 = c6b0-0x78`, `c684 = c6b4-0x78`,
  `c688 = c6b8+0x78`, `c68c = c6bc+0x78` (±120 units); area `c748 = (H+240)(W+240)`.
- Spawn cursors: `c5d8` (Y), `c714` (X), step 0x3c; `c6fc` = on-screen area budget.
- `c610` = render-valid flag (set by game_tick after a render; input handlers re-
  render once when set, then clear it).

## M1 answers

### 1. RNG — algorithm, seed, call order

**The RNG is the MSVC CRT rand, statically linked into the image** (the function
map's "CRT" classification of 0x406cd0/0x406cda is **correct** — verified: the
import table contains no rand/srand; the constants 0x343fd = 214013 and
0x269ec3 = 2531011 are exactly the classic MSVC CRT rand constants, i.e. the
compiler inlined `_rand`/`_srand` from the static CRT):

- **Algorithm** (`FUN_00406cda` 0x406cda, disasm-verified —
  `imul $0x343fd,%eax,%eax; add $0x269ec3,%eax; mov %eax,0x40c16c; sar $0x10,%eax;
  and $0x7fff,%eax` = MSVC `seed = seed*214013L+2531011L; return
  (unsigned)(seed/65536)%32768`): `c16c = c16c * 0x343fd + 0x269ec3; return
  (c16c >> 16) & 0x7fff;` → 15-bit output, range 0..32767. Entire RNG state = the
  single u32 `c16c`.
- **Seed setter** (`FUN_00406cd0` 0x406cd0): `c16c = seed;`
- **Seed site** (`game_reset` 0x404970, first two lines):
  `c698 = GetTickCount(); FUN_00406cd0(c698);` — i.e. **seed = GetTickCount() at
  every reset** (startup via WinMain and every `game_restart`). For Task 8 (seed
  freeze): overwrite `c16c` (or the `mov [c16c], seed` at 0x404970's call) — the LCG
  then produces a fully deterministic stream.
- **Wrapper** (`util_rand_range` 0x4020b0): `r = rand(); return r % n;` (low16 of the
  returned dword; the high16 holds r/n and is unused by callers).

**Per-tick rand() call order** (fixed skeleton; exact count depends on live entities):

1. `game_physics` pass 1 — per entity in active-list order, `game_entity_activate`
   0x4028e0 → AI anim functions (only when their frame condition hits):
   type 1: rand(12) then rand(3); type 2 (dog): rand(3)/rand(0x20)/rand(100) by frame;
   type 3 (snowboarder): rand(10); type 10 (pine): rand(100), rand(2), rand(10).
2. Gate stepping — `game_gate_scan` ×4 (c630/c5e0/c658/c738) + `game_gate_update`
   (c720 moving list) → bench 0x404290: rand(1000) (only when a **spawned frame-39
   bench** is in the middle band — 0x404305); yeti cruise 0x404350: rand(10) (only
   in the idle, non-moving branch — 0x404677).
3. Collision pass — no rand.
4. Spawn bands — per 60 units crossed: picker rand (rand(1000) wide / rand(100) mid /
   rand(64) narrow / none for speed) + `game_spawn_pos` rand(c6d8)/rand(c5f0) +
   `game_sprite_frame` rand(8)/rand(4)/rand(3) for types 0xd/0xe/0xf.
5. Snowboarder roll — `rand_range(0x29a)` once per tick (1/666).

`game_render` and the status redraw perform no rand calls.

**Non-tick consumption (once per `game_start`):** `game_level_init` 0x404b50 runs in
the GS-gate loop a `game_sprite_frame(0xd)` (rand(8)) + `rand(0x20)` for a pine gate
that is never added to the list, plus a `rand(400)` whose result is discarded — 3
rand() calls × 39 gates = **117 calls per game_start**, immediately after the
GetTickCount seed. A seed-freeze reimplementation MUST consume these to stay in sync
with the original per-tick stream.

### 2. Timing — timer, tick site, input structure

- **Timer:** `SetTimer(c6c8, 0x29a, c678 & 0xffff, c940)` in `game_resume` 0x404ad0 —
  **timer ID 0x29a (666), period `c678` = 0x28 = 40 ms** (set in `game_reset`
  0x404970), callback `c940` → `game_tick_cb` 0x4047c0: `if (c67c) game_tick();
  return 1;`. `game_pause` 0x4057c0: `KillTimer(c6c8, 0x29a); c600 = now; c6d0 = 0`.
- **Tick site:** `game_tick` 0x401000:
  `c5f4 = now - c698 (delta); c708 = c698; c698 = now (GetTickCount);
  game_physics (0x401e50); game_render (0x401060); c610 = 1; if (now - c5dc > 0x147)
  draw_status_values(c6cc);`
- **Fixed-step, not dt-scaled:** movement is one (or two, when `c670` debug toggle)
  step per timer fire regardless of `c5f4`; the real delta `c5f4` is used only for
  (a) the Speed display `+0x48*1000/(c5f4<<4)`, (b) style-run timing via
  `util_lerp` on `c698`/`c708`, (c) the 327 ms status throttle. Physics does not
  scale with the delta — a slow system simply plays slower.
- **Input structure: message-consumed, no key-state global** — see Data model
  *Input*. All control is applied synchronously inside WndProc handlers (WM_KEYDOWN /
  WM_CHAR / WM_MOUSEMOVE / WM_LBUTTONDOWN / WM_RBUTTONDOWN), gated by `c67c`. No
  polling APIs exist in the binary. A WASM port must feed equivalent events; there is
  no GetKeyState-style state to snapshot.

### 3. Sound — sndPlaySoundA sites, WAVE resources, flags, default state

- **Sites:** `snd_init` 0x405620 sets `c790` = the `sndPlaySoundA` import; the only
  call site is `snd_play` 0x402ba0:
  `if (!c794) { if (!pair[1] && pair[0]) pair[1] = LockResource(pair[0]);
  if (pair[1]) c790(pair[1], 5); }` — **flags = 5 = SND_ASYNC (1) | SND_MEMORY (4)**
  (the pointer is the LockResource memory, not a resource name).
- **Loads:** 9 `snd_load_wave` 0x405640 calls in `game_create_windows` 0x4052d0:
  `FindResourceA(hInst, (LPCSTR)id, "WAVE"@c128)` → LoadResource → LockResource,
  into 8-byte pairs. Order and slots: id 1→c6c0, 2→c768, 3→c5d0, 4→c718, 5→c750,
  6→c628, **9→c6f0**, 7→c6e0, 8→c608 (id 9 loaded before 7/8).
- **WAVE resource list: NONE.** The PE `.rsrc` root directory (file offset 0xd000)
  contains exactly four type nodes: **2 = RT_BITMAP (89 entries), 3 = RT_ICON (6),
  6 = RT_STRING (2 groups), 14 = RT_GROUP_ICON (2 named: ICONSKI, ICONSKI2)**.
  There is **no RT_WAVE (type 15) node at all** — the earlier Resources section's
  "9 RT_WAVE resources (ids 1-9) in `.rsrc`" is incorrect. Consequently all 9
  FindResourceA calls return NULL at runtime, every pair is {NULL, 0}, and `snd_play`
  is a no-op at every site (crash, yeti wake, gate pass, snowboarder, …).
- **Default state:** sound is ON by default; `WinMain` 0x4047e0 sets `c794 = 1` only
  when the command line matches `"nosound"` (lstrcmpiA). But because no WAVE
  resources exist, **the game is silent in all configurations regardless of the
  flag.** A WASM port can either omit audio entirely (faithful) or wire real samples
  to the 9 pair slots (c6c0..c608) to realize the intended audio.
