# NOTES — SkiFree (ski32.exe) decompilation

Source: `original/ski32.exe` (i386, PE32, MSVC6-era, single source file `V:\hack\ski32\ski2.c`),
decompiled with Ghidra 12.1.3 (project `ski32`, program `ski32.exe`) into `decompile/ghidra/*.c` —
**163 functions** (`FUN_XXXXXXXX.c`), cross-referenced against the binary.

## Classification summary

- **104 GAME** (ski2.c logic — all renamed below with `ghidra_name` preserved)
- **59 CRT** (MSVC6 C runtime + PE entry/exit/SEH — left unrenamed)

Method: `harness/triage.py` heuristically classifies by scanning each `.c` for `DAT_0040XXXX`
references into verified game-owned data ranges, then every classification was hand-verified by
reading the decompiled C of all 163 functions (see *Triage adjustment* below).

## Naming convention

| prefix | meaning |
|---|---|
| `main_*` | process entry (WinMain) |
| `wproc_main_*` | SkiMain window handlers (the WndProc dispatcher itself at 0x405800 was **not** decompiled — see *Missing code*) |
| `wproc_status_*` | SkiStatus window handlers (0x4068d0 is the real WndProc) |
| `game_*` | game logic (entities, gates, spawning, style, pause, level, windows, sprites) |
| `draw_*` | rendering (entities, scene, status values, text, offscreen) |
| `score_*` | high-score dialog + style score |
| `snd_*` | WAVE resource loading/playing (winmm `sndPlaySoundA`) |
| `util_*` | helpers (rects, lerp, time fmt, string cache, bitmap load, asserts, facing math) |

## Full function map (163)

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
| 0x402180 | game_entity_set_col | FUN_00402180 | 152 | GAME | set sprite column (+0x10) and adjust gate count (c6fc) |
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
| 0x4026f0 | game_spawn_pick_wide | FUN_004026f0 | 122 | GAME | type pick (wide zone): weighted rand(1000) over {2,10,11,13,14,15,16}; 0x12 when view full (c748/32 < c6fc) |
| 0x402770 | game_spawn_pick_speed | FUN_00402770 | 37 | GAME | type pick (speed zone): 11 or 0x12 from speed threshold (c748/64 vs c6fc) |
| 0x4027a0 | game_spawn_pick_narrow | FUN_004027a0 | 52 | GAME | type pick (center line): 50/50 {2,11}; 0x12 when view full |
| 0x4027e0 | game_spawn_pick_mid | FUN_004027e0 | 100 | GAME | type pick (mid zone): rand(100) over {10,13,15,11,16,0x12} |
| 0x402850 | game_sprite_frame | FUN_00402850 | 118 | GAME | sprite column from entity type (type switch) |
| 0x4028e0 | game_entity_activate | FUN_004028e0 | 584 | GAME | entity activate: per-type dispatch (activate_sound_table a308, activate_anim_table a434), sound via c6c0/c718 |
| 0x402ba0 | snd_play | FUN_00402ba0 | 58 | GAME | play WAVE: LockResource + sndPlaySoundA (c790), gated by sound_disabled (c794) |
| 0x402be0 | game_entity_step | FUN_00402be0 | 125 | GAME | entity step: per-type movement/velocity substep |
| 0x402c60 | game_style_ss | FUN_00402c60 | 452 | GAME | SS style check: INI "Ski"/"SS" (c0d8); style tick c944/c948/c94c/c95c/c964 |
| 0x402e30 | util_lerp | FUN_00402e30 | 66 | GAME | lerp: (p1 - (p1-p2)*(p3-p5))/(p3-p4); asserts p3 != p4 |
| 0x402e80 | game_player_face | FUN_00402e80 | 56 | GAME | player facing-frame update (c72c type, +0x11 sign -> frame 3-13 via game_entity_set_frame) + status redraw |
| 0x402ec0 | score_show | FUN_00402ec0 | 619 | GAME | high-score dialog: INI read ("Ski"/"entpack.ini"), parse <=10 scores, insert new (time, negated) into DESCENDING list, write back "%ld "/"%9ld"/"%s", MessageBox "High Scores" + "that's you!"/"try again!"; uVar7==10 = did not qualify |
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
| 0x4041c0 | game_gate_advance | FUN_004041c0 | 194 | GAME | advance gate descriptor (next 36B record) |
| 0x404290 | game_gate_type4 | FUN_00404290 | 177 | GAME | gate type-4 behavior |
| 0x404350 | game_gate_cruise | FUN_00404350 | 883 | GAME | gate/level entity update (types 5-8 crash objects; client height c6d8) |
| 0x4046e0 | game_gate_scan | FUN_004046e0 | 222 | GAME | gate array scan: spawn within view band (c5fc +/- 0x3c, c684/c68c) |
| 0x4047e0 | main_winmain | FUN_004047e0 | 213 | GAME | WinMain: lstrcmpiA "nosound" -> c794; game_init_mem, game_reset, game_create_windows, game_start; GetMessage/TranslateMessage/DispatchMessage loop; snd_shutdown; returns msg.wParam |
| 0x4048c0 | game_init_mem | FUN_004048c0 | 138 | GAME | LocalAlloc pools: c674 str cache (0x50), c5f8 sprite cols (0x5a0), c648 entity pool (8000=100x80B), c758 gate descs (0x2400=100x36B); fatal "Insufficient local memory." on failure |
| 0x404950 | util_fatal_msg | FUN_00404950 | 27 | GAME | fatal message: MessageBoxA(NULL, msg, "SkiFree" (RT 1), MB_ICONERROR=0x30) |
| 0x404970 | game_reset | FUN_00404970 | 143 | GAME | reset all game state: srand(GetTickCount), freelist, zero ~15 counters/flags, c678=0x28 (40ms timer), c610=1 |
| 0x404a00 | game_freelist_init | FUN_00404a00 | 100 | GAME | init 100-entity freelist (c648 pool, 80B slots) -> c744 |
| 0x404a70 | game_gate_idx_reset | FUN_00404a70 | 10 | GAME | clear gate descriptor index (c702 = 0) |
| 0x404a80 | game_start | FUN_00404a80 | 71 | GAME | game_start: create player (type 0, frame 3) at origin -> c64c/c72c; spawn start poles; level init; clear game_over; game_resume |
| 0x404ad0 | game_resume | FUN_00404ad0 | 128 | GAME | game_resume: guards c6c8/c6d0/c650; c6d0=1, c698=now; style-tick adjust (c948 += now - c600) if c95c||c958; SetTimer(c6c8, 0x29a, c678, c940) |
| 0x404b50 | game_level_init | FUN_00404b50 | 1448 | GAME | level init: build 4 gate arrays (c5e0/c630/c658/c738), gate list (c720 via game_gate_list_add), style reset |
| 0x405100 | game_gate_list_clear | FUN_00405100 | 30 | GAME | gate list clear (free descriptors, c720=NULL, c702=0) |
| 0x405120 | game_gate_list_add | FUN_00405120 | 187 | GAME | gate list add: 36B descriptor from c758 (max 0x100), append to c720 |
| 0x4051e0 | game_startpoles_spawn | FUN_004051e0 | 226 | GAME | spawn start flagpoles (type 0x11, frames 0x35-0x38) |
| 0x4052d0 | game_create_windows | FUN_004052d0 | 842 | GAME | create_windows: GetDeviceCaps HORZRES/VERTRES (c6a0/c74c), c61c=hInstance, white brush c69c, FindWindowA("SkiMain") single-instance check, load 9 WAVE resources (gated by !c794), RegisterClassA SkiMain (WndProc = 0x405800, NOT decompiled) + SkiStatus (wproc_status) + button, CreateWindowExA both, icon (c120) |
| 0x405620 | snd_init | FUN_00405620 | 20 | GAME | snd_init: c790 = sndPlaySoundA import |
| 0x405640 | snd_load_wave | FUN_00405640 | 83 | GAME | snd_load_wave: FindResourceA(hInst, id, "WAVE") -> pair {HGLOBAL, LockResource(pData)} |
| 0x4056a0 | snd_shutdown | FUN_004056a0 | 131 | GAME | snd_shutdown: stop sound, FreeLibrary(c78c), free all 9 WAVE pairs |
| 0x405730 | snd_free | FUN_00405730 | 38 | GAME | snd_free: FreeResource(pair[0]) |
| 0x405760 | game_pause_toggle | FUN_00405760 | 89 | GAME | pause/resume toggle: running -> game_pause + title "Ski Paused ... Press F3 to continue" (RT 2); else title "SkiFree" (RT 1) + game_resume |
| 0x4057c0 | game_pause | FUN_004057c0 | 52 | GAME | game_pause: KillTimer(0x29a), c600 = now, c6d0 = 0 |
| 0x405a10 | game_pause_auto | FUN_00405a10 | 48 | GAME | auto pause/resume: if c694 && !c770 -> c67c=1 + game_resume; else c67c=0 + game_pause |
| 0x405a40 | wproc_main_create | FUN_00405a40 | 111 | GAME | wproc_main WM_CREATE: GetDC (c63c), reset GDI objects, game_sprites_load; fatal "Whoa, like, can't load bitmaps!" on failure |
| 0x405ab0 | game_sprites_load | FUN_00405ab0 | 996 | GAME | sprites_load: LoadBitmapA ids 1-0x59 (util_load_bitmap), build c5f8 column table, mask DCs (c710/c6a4/c730/c6ec) + offscreen bitmap |
| 0x405ea0 | util_load_bitmap | FUN_00405ea0 | 20 | GAME | LoadBitmapA wrapper |
| 0x405ec0 | wproc_main_destroy | FUN_00405ec0 | 214 | GAME | wproc_main WM_DESTROY: restore+delete GDI objects (DCs, bitmaps) |
| 0x405fa0 | wproc_main_size | FUN_00405fa0 | 179 | GAME | wproc_main WM_SIZE: client rect (c6b0/c6b4/c6b8/c6bc), view bounds (c680-c68c), view width/area (c5f0/c748), client height (c6d8), center via game_set_center |
| 0x406060 | game_set_center | FUN_00406060 | 73 | GAME | reset entity rect-cache flags (list walk), set view center c5fc/c704 |
| 0x4060b0 | wproc_main_paint | FUN_004060b0 | 71 | GAME | wproc_main WM_PAINT: BeginPaint, FillRect (white brush), draw_scene, EndPaint |
| 0x406100 | draw_scene | FUN_00406100 | 112 | GAME | draw_scene: render pass (c618 list, offscreen c614) |
| 0x406170 | wproc_main_input | FUN_00406170 | 565 | GAME | wproc_main input: WM_KEYDOWN/WM_CHAR -> action maps (a258/a25c); mouse aim (wproc_main_aim); wproc_main_turn / wproc_main_key |
| 0x406500 | game_restart | FUN_00406500 | 76 | GAME | game_restart: if game_over -> game_reset + game_start + InvalidateRect/UpdateWindow, else DestroyWindow(main) |
| 0x406550 | wproc_main_aim | FUN_00406550 | 136 | GAME | wproc_main aim: mouse pos -> facing via util_facing_delta/crouch -> set frame; store c700/c70c, c760=1 |
| 0x4065e0 | util_facing_delta | FUN_004065e0 | 132 | GAME | facing delta: upright orientation from mouse delta |
| 0x406670 | util_facing_crouch | FUN_00406670 | 85 | GAME | facing crouch: crouched orientation from mouse delta |
| 0x4066d0 | wproc_main_turn | FUN_004066d0 | 148 | GAME | wproc_main turn: key-repeat facing change; slide/jump states |
| 0x406780 | wproc_main_key | FUN_00406780 | 203 | GAME | wproc_main key: X/Y/Z + numpad 1/2/3 = move +/-2, F = speed toggle (c670), R = redraw, T = manual tick (game_tick) |
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
| 0x4078b6 | __local_unwind2 | — | 104 | CRT | CRT __local_unwind2 |
| 0x40794a | FUN_0040794a | — | 24 | CRT | CRT exception table lookup |
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
### GAME functions (104) — renamed

See the table above for the per-function role. Key subsystems:

- **Timing/pause**: `game_tick` (0x401000, driven by `SetTimer(hwnd, 0x29a, 40ms, timer_proc)`) →
  `game_physics` (0x401e50) + `game_render` (0x401060). Pause: `game_pause` (0x4057c0, `KillTimer`),
  resume: `game_resume` (0x404ad0). `game_pause_toggle` (0x405760) is F3 and the post-assert path;
  `game_pause_auto` (0x405a10) pauses around the score dialog.
- **Entities**: 80B structs from a 100-slot pool (`c648`), freelist `c744`; types 0-17
  (0 = player, 1-10 spawnable obstacles/gates, 11-17 gate variants; 0x12 = "no spawn").
- **Gates**: four arrays (`c5e0/c630/c658/c738`) + descriptor list (`c720`, 36B records in `c758`,
  max 100); spawned by `game_gate_scan`/`game_gate_update` within the view band
  (client rect extended by 0x78px).
- **Style score** (FS/SS/GS bonuses, INI-gated via keys at 0x40c0d8/c0f4/c0f8): accumulators at
  `c944-c968`, displayed as `Style:` on the status panel.
- **High scores**: `score_show` (0x402ec0) reads/writes section `Ski` of `entpack.ini`
  (0x40c084), keeps up to 10 scores **descending** (index 0 = best), stored as `"%ld "` items with
  times negated; box caption `High Scores` (RT_STRING 15), suffixes `that's you!`/`try again!` (16/17).
- **Sound**: 9 WAVE resources (ids 1-9) loaded at startup (`snd_load_wave`) into 8-byte
  `{HGLOBAL, pData}` pairs; played via `sndPlaySoundA` (`c790`); disabled by the `nosound`
  cmdline flag (`c794`); pair map: id1→c6c0, id2→c768, id3→c5d0, id4→c718, id5→c750,
  id6→c628, id9→c6f0, id7→c6e0, id8→c608.
- **Windows**: `main_winmain` → `game_create_windows` (0x4052d0) registers classes `SkiMain` /
  `SkiStatus` / `button` and creates both windows (status window initial text = `c788`, an empty
  64B buffer). The main window title is RT_STRING 1 (`SkiFree`); the paused title is RT_STRING 2.

### CRT functions (59) — left unrenamed

All MSVC6 CRT: PE entry `0x406d83`, heap (`0x40813b-0x408de3`, heap state `c96c-c980`, threshold
`c5c0`), string/locale/ctype (`0x40768a`, `0x407bd0`, `0x407d19-0x407f9a`, `0x408ede`, `0x4090ff`,
`0x409370`, `0x408f70`, `0x409070`, `0x4082e0`, `0x408260`), exit/SEH (`0x406e79-0x407a7d`,
`0x407874`, `0x4078b6`, `0x4096a6`), and `srand`/`rand`/`_atoi` (`0x406cd0-0x406cf8`).
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
were reclassified to GAME by hand review — final truth: **104 GAME / 59 CRT**.
Missed-by-heuristic list: 1240, 1290, 12f0, 1350, 1540, 1a60, 1b20, 1d70, 20b0, 20d0, 2220, 2310,
2350, 26a0, 2850, 2e30, 3430, 3750, 37b0, 4070, 41c0, 4290, 4950, 5100, 5730, 65e0, 6670, 6c50
(all addresses 0x40-prefixed).

## Missing code regions (concern for later tasks)

Two code regions referenced by the decompiled code are **not** among the 163 decompiled functions
(no `.c` files exist for them; both fall in Ghidra's label gaps):

1. **0x4047c0-0x4047e0 (~32B timer callback)** — `DAT_0040c940 = &LAB_004047c0` is installed as the
   `SetTimer` callback in `game_create_windows`; it is the per-tick entry point (presumably calls
   `game_tick`/`game_pause_auto`). A later port task must re-decompile this to get the exact
   timer semantics.
2. **0x405800-0x405a10 (~528B main WndProc)** — `RegisterClassA("SkiMain", ..., lpfnWndProc =
   &LAB_0x405800)` in `game_create_windows`. This is the SkiMain message dispatcher (routes to
   `wproc_main_create`/`wproc_main_size`/`wproc_main_paint`/`wproc_main_input`/
   `wproc_main_destroy`). Must be re-decompiled before the WASM port.

The class-name copies at `a190/a198/a1a4` (`SkiMain`/`SkiStatus`/`button`) are likewise only
referenced from code outside the 163-fn set (0x4052d0 uses identical inline literals).

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
  player `c64c`/`c72c`, sprite column table `c5f8`, gate arrays `c5e0/c630/c658/c738` +
  descriptors `c758` (100×36B) + list `c720` + index `c702` + count `c6fc`, view rect
  `c6b0-c6bc` (client) / `c680-c68c` (extended by 0x78) + area `c748`, spawn cursors
  `c5d8/c714` (step 0x3c), ticks (`c698` current, `c5f4` delta, `c708` resume, `c5dc` last
  status, `c600` pause, `c6f8` style), timer (`c678` interval 40ms, `c6d0` active, `c940` proc
  ptr), sound (`c790` fn ptr, `c794` disabled, 9 WAVE pairs, `c78c` HMODULE), style
  accumulators `c944-c968`, score `c6a8`, string cache `c674`, screen `c6a0/c74c` (HORZ/VERTRES).
- Resources: RT_STRING STRINGTABLE at VA 0x41c718 (17 length-prefixed UTF-16 entries, see below);
  9 RT_WAVE resources (ids 1-9) in `.rsrc`; icon `ICONSKI`/`ICONSKI2`.

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
