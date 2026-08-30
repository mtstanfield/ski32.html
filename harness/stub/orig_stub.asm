; T14 original-side tick hook for ski32.exe (position-DEPENDENT patch)
;
; Patches (file offset == RVA; image base 0x400000):
;   1. 0x1000 (VA 0x401000, ski_tick entry):
;      ff 15 c0 a0 40 00  (call *0x40a0c0 GetTickCount)
;      ->  e9 <rel32> 90  (jmp orig_stub_entry; nop pad)
;   2. this code at 0x96ac (VA 0x4096ac; .text raw tail). NOTE: .text vsz
;      is 0x86ac, so the loader only commits up to 0x4096ac unless the
;      patcher grows .text VirtualSize to RawSize (0x9000) — it does.
;   3. home pointers: TWO 4-byte slots at 0x40c284/0x40c288 (.data).
;      Evidence in the HOME1/HOME2 comments below (zero-ref 0x40c200 page,
;      256 B block verified all-zero at runtime incl. mid-descent).
;
; Memory model (2026-08-29 rewrite, LocalAlloc 2026-08-29b): ALL mutable
; stub state lives in a LocalAlloc'd zeroed block (0x1100 B,
; LMEM_FIXED|LMEM_ZEROINIT) taken at first tick. A NULL-based
; VirtualAlloc(NULL, 0x1000, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE)
; returns NULL inside this process under wine 9.0 — verified 2026-08-29
; with correct args (gdb at the call site + strace: no mmap issued)
; while the identical call succeeds in a standalone PE; the CRT local
; heap (which the game itself uses for every entity) works, so the stub
; uses it. The block base is kept redundantly in HOME1+HOME2. Guard:
; HOME1==HOME2, nonzero, and [base+0]==MAGIC (0x534b4946 "SKIF"); any
; mismatch makes the stub re-initialize on the next tick (the orphaned
; block leaks — acceptable for a capture harness). No stub state besides
; the 8 home bytes is stored in .data, and those 8 bytes sit in a
; verified-dead block, so no collision with game globals.
;
; ebp is used as the base pointer throughout: pushad at entry saves the
; old ebp and popad restores it at exit; all calls honor the cdecl
; callee-saved ebp.
;
; Contract (mirrors the rebuild SKI_HARNESS hook at ski_tick top):
;   frame N (0-based) = client area captured AFTER tick body N, BEFORE
;   word[N] injection. Dump CWD/frame_%06u_main.ppm (P6 top-down 24bpp).
;   word N = LE u16 at 2*N in CWD/ski_in.bin; set bit k -> exactly one
;   WndProc(hwnd, WM_KEYDOWN, VK_k, 0) via direct 0x405800 call.
;   Inert (no ski_in.bin in CWD): replay tick count, jump on.
;
; The displaced first instruction (call *0x40a0c0) is REPLAYED verbatim:
; 0x401006 consumes eax (tick count) via mov %eax,%edx. pushad protects it.
;
; IAT slots verified byte-by-byte against ski32.exe's import directory
; (KERNEL32 thunks @0x40a03c, USER32 @0x40a100, GDI32 @0x40a000).
;
; Wine gotcha (NOTES T13): GetDIBits fails on window DCs (err 183) ->
; CreateDIBSection (top-down 32bpp, section address = raster) + BitBlt,
; then software 32->24 convert, WriteFile. CreateDIBSection is NOT in
; the import table (static CRT, no MSVCRT import): resolved at first
; tick via GetProcAddress(GetModuleHandleA(...)).
;
; FILE I/O (2026-08-29d): pure Win32 (CreateFileA/ReadFile/WriteFile/
; CloseHandle/SetFilePointerEx) resolved from the ALREADY-LOADED
; kernel32.dll. The game statically links its own MSVC CRT and imports
; NO msvcrt, so LoadLibraryA("msvcrt.dll") at first tick initializes a
; SECOND CRT inside the process and the two CRTs collide: wine raises
; STATUS_ASSERTION_FAILURE (0xC0000258) that the SEH chain cannot
; dispatch ("Exception frame is not in stack limits") -> the process
; dies ~0.3 s after start (exit 88). Loading no new DLL at all avoids
; the collision entirely.

bits 32
org 0x4096ac

; File-backed stage logger: appends one byte per stage transition to
; stg.bin (opened CREATE_ALWAYS at init). Survives the process crash,
; so the last byte after a crash = last stage reached. If the open
; failed (o_stgf = INVALID_HANDLE_VALUE) the WriteFile just returns
; FALSE — harmless, so no branch (this nasm build expands none of
; %i/%$/%{UniqueId} inside labels).
%macro LOGSTAGE 1
    mov  dword [ebp+o_stgbyte], %1
    push 0
    lea  ecx, [ebp+o_stgwr]
    push ecx
    lea  ecx, [ebp+o_stgbyte]
    push ecx
    push 1
    mov  eax, [ebp+o_stgf]
    push eax
    call [ebp+o_p_writef]
    mov  [ebp+o_stgret], eax
%endmacro

; ---------------------------------------------------------------- IAT slots
i_DeleteObject   equ 0x40a000
i_SelectObject   equ 0x40a004
i_BitBlt         equ 0x40a00c
i_DeleteDC       equ 0x40a028
i_CreateCompatibleDC equ 0x40a034
i_GetProcAddress equ 0x40a074
i_GetTickCount   equ 0x40a0c0
i_LocalAlloc     equ 0x40a0c4
i_GetModuleHandle equ 0x40a0c8
i_wsprintfA      equ 0x40a108
i_GetClientRect  equ 0x40a14c
i_SetWindowTextA equ 0x40a164
i_GetDC          equ 0x40a16c
i_ReleaseDC      equ 0x40a170
i_FindWindowA    equ 0x40a168
i_IsIconic       equ 0x40a17c

wproc_main       equ 0x405800      ; SkiMain WndProc (direct call)
c6c8             equ 0x40c6c8      ; main HWND global
c624             equ 0x40c624      ; status HWND global
HOME1            equ 0x40c284      ; page-base home #1 (4 B, .data)
HOME2            equ 0x40c288      ; page-base home #2 (redundant copy)
INITCNT          equ 0x40c28c      ; DEBUG: .init entry count
INITSTG          equ 0x40c290      ; DEBUG: .init progress stage
ENTRYESP         equ 0x40c294      ; DEBUG: esp at stub entry (after add esp,4)
ENTRYSTG         equ 0x40c298      ; DEBUG: entry/exit markers (.data, no page needed)
tick_body        equ 0x401006      ; first instruction after the jmp patch

; Home location evidence (2026-08-29):
;  - the 0x40c200 .data page has ZERO direct .text references (coarse 4-byte
;    LE scan of the whole virtual .text + .rdata);
;  - 0x40c284..0x40c383 (256 B) verified ALL ZERO at runtime, including
;    mid-descent (seeded 10 s gameplay snapshot); neighbors 0x40c280 and
;    0x40c384+ are live game data (block boundaries exact).
;  - The whole .data is otherwise live: 0x40c944..0x40c968+ are real
;    globals, 0x40c9e1..0x40ca25 are runtime-built letter tables
;    ("abcde..." / "ABCDE..." — this is what clobbered the first stub
;    revision's home at 0x40ca00), 0x40ca60+ ascending byte tables,
;    0x40cae0+ a 0x10 table, refs at 0x40caa0/0xcbxx/0xccxx.
;  - A fixed VirtualAlloc base was tried and rejected: 0x21000000 is
;    MEM_RESERVE (wine arena) in the game process (VirtualQuery state
;    0x2000, verified in-process), so "alloc-NULL-means-ours" is unsafe.
; If a home is ever clobbered the mismatch/magic check re-inits the page
; (orphaned page leaks; ski_in.bin reopens from frame 0 — acceptable).

MAGIC            equ 0x534b4946    ; "SKIF"

; ------------------------------------------------- base page offsets (ebp)
o_magic        equ 0x000
; strings @+0x100 (order MUST match the blob at the bottom of this file)
o_str_skiin equ 0x6c4           ; "ski_in.bin\0"            11
o_str_frame equ 0x6cf           ; "frame_%06u_main.ppm\0"   20
o_str_p6hdr equ 0x6e3           ; "P6\n%d %d\n255\n\0"      14
o_str_gdi32 equ 0x6f1           ; "gdi32.dll\0"             10
o_str_k32 equ 0x6fb           ; "kernel32.dll\0"          13
o_str_creatf equ 0x708           ; "CreateFileA\0"           12
o_str_readf equ 0x714           ; "ReadFile\0"               9
o_str_writef equ 0x71d           ; "WriteFile\0"             10
o_str_closeh equ 0x727           ; "CloseHandle\0"           12
o_str_setfp equ 0x733           ; "SetFilePointerEx\0"      17
o_str_localfree equ 0x744          ; "LocalFree\0"             10
o_str_dibsec equ 0x74e           ; "CreateDIBSection\0"      17 -> ends 0x19b
o_str_getlasterr equ 0x75f         ; "GetLastError\0"          13 -> ends 0x1a8
o_str_u32 equ 0x76c           ; "user32.dll\0"            11 -> ends 0x1b3
o_str_wsprintf equ 0x777           ; "wsprintfA\0"            10 -> ends 0x1bd
; (2026-08-29d: Win32 file I/O names replace the stdio names; msvcrt
;  and the "rb"/"wb" mode strings are gone. All offsets re-derived.)
; function pointers @+0x200 (all kernel32/gdi32, resolved by name)
o_p_creatf     equ 0x200
o_p_readf      equ 0x204
o_p_writef     equ 0x208
o_p_closeh     equ 0x20c
o_p_setfp      equ 0x210
o_p_dibsec     equ 0x214
o_p_localfree  equ 0x218
o_p_getlasterr equ 0x248           ; kernel32 GetLastError
o_p_wsprintf   equ 0x264           ; user32 wsprintfA (CDECL)
o_hdl_u32      equ 0x268           ; user32 module handle
o_p_ccbm       equ 0x26c           ; gdi32 CreateCompatibleBitmap
o_p_getdib     equ 0x270           ; gdi32 GetDIBits
o_p_getclass   equ 0x274           ; user32 GetClassNameA (CDECL? stdcall 12)
o_p_iswin      equ 0x278           ; user32 IsWindow
o_p_gfw        equ 0x27c           ; user32 GetForegroundWindow
o_p_gdw          equ 0x280           ; user32 GetDesktopWindow
o_p_gwr          equ 0x284           ; user32 GetWindowRect (was GetDeviceCaps slot)
o_str_ccbm equ 0x781           ; page: "CreateCompatibleBitmap\0" 23
o_str_getdib equ 0x798           ; page: "GetDIBits\0" 10
o_str_getclass equ 0x7a2           ; page: "GetClassNameA\0" 14
o_str_iswin equ 0x7b0           ; page: "IsWindow\0" 8
o_str_skimain equ 0x7b9           ; page: "SkiMain\0" 8
o_str_gfw equ 0x7c1           ; page: "GetForegroundWindow\0" 20
o_str_gdw equ 0x7d5           ; page: "GetDesktopWindow\0" 17
o_gpa_r1       equ 0x24c           ; DEBUG: retest page string
o_gpa_r2       equ 0x250           ; DEBUG: retest static string
o_gpa_r3       equ 0x254           ; DEBUG: retest page string (2nd)
o_gpa_r5       equ 0x258           ; DEBUG: lowercase variant
o_gpa_r8       equ 0x25c           ; DEBUG: page "ReadFile"
o_gpa_r9       equ 0x260           ; DEBUG: static "NoSuchFuncXYZ" (control)
o_hdl_gdi      equ 0x220           ; gdi32 module handle
o_hdl_k32      equ 0x224           ; kernel32 module handle
o_ppm_hnd      equ 0x22c           ; current PPM output handle
o_hdrptr       equ 0x230           ; header buf ptr (write phase)
o_hdrlen       equ 0x234           ; header len (write phase)
o_bitstk       equ 0x238           ; .bit loop saved ecx/edx/edi
o_bitstk_edx   equ 0x23c
o_bitstk_edi   equ 0x240
; DIB state @+0x300
o_memdc        equ 0x300
o_hbm          equ 0x304
o_bits         equ 0x308           ; 32bpp top-down raster
o_pix          equ 0x30c
o_conv         equ 0x310           ; 24bpp convert buffer (LocalAlloc)
o_convlen      equ 0x314
o_w            equ 0x318
o_h            equ 0x31c
o_ox           equ 0x320           ; client origin x in screen coords
o_oy           equ 0x324           ; client origin y in screen coords
; BITMAPINFO @+0x400 (40 B), DIBSection bits holder @+0x500
o_bmi          equ 0x400
o_bitsld       equ 0x500
; runtime @+0x600
o_file         equ 0x600           ; HANDLE ski_in.bin (0 = not open)
o_tick         equ 0x604           ; 0-based frame/word index
o_io32         equ 0x608           ; scratch DWORD (bytes-read/high dist)
o_cnt          equ 0x60c           ; DEBUG: stub entry count (every path)
o_stage        equ 0x610           ; DEBUG: .do_work progress marker
o_gcr_rc       equ 0x614           ; DEBUG: GetClientRect return value
o_gcr_rect     equ 0x618           ; DEBUG: copied RECT (4 dwords)
o_gcr_rect2    equ 0x628           ; DEBUG: 2nd GCR with page RECT
o_gcr_rc2      equ 0x638           ; DEBUG: 2nd GCR return value
o_frame_esp    equ 0x63c           ; DEBUG: esp after sub esp,80 (frame top B)
o_stgf         equ 0x640           ; DEBUG: stg.bin handle (0xffffffff = open failed)
o_stgbyte      equ 0x644           ; DEBUG: one-byte stage payload
o_stgwr        equ 0x648           ; DEBUG: bytes-written out-param
o_stgret       equ 0x64c           ; DEBUG: last WriteFile return (eax)
o_stgret_init  equ 0x650           ; DEBUG: init-time test WriteFile return
o_stgerr       equ 0x654           ; DEBUG: GetLastError after failed test write
o_stgftype     equ 0x658           ; DEBUG: GetFileType(stgf)
o_align_open   equ 0x65c           ; DEBUG: esp at stg-open call
o_align_wrt    equ 0x660           ; DEBUG: esp at test-write call
o_align_gle    equ 0x664           ; DEBUG: esp at getlasterr call
o_align_gft    equ 0x668           ; DEBUG: esp at getftype call
o_align_frame  equ 0x66c           ; DEBUG: esp after sub esp,80 in .do_work
o_biso         equ 0x670           ; BISECT gates: 1=GCR 2=DIB 4=BitBlt+PPM 8=inject
o_dirty        equ 0x674           ; size-changed flag (gate1 -> gate2)
o_getdc_rc     equ 0x67c           ; DEBUG: GetDC(c6c8) result in gate 2
o_iswin_rc     equ 0x680           ; DEBUG: IsWindow(c6c8)
o_clsbuf       equ 0x684           ; DEBUG: class name buffer (32 B)
o_iswin2_rc    equ 0x6a4           ; DEBUG: IsWindow(c624 status)
o_getdc2_rc    equ 0x6a8           ; DEBUG: GetDC(c624)
o_found        equ 0x6ac           ; DEBUG: FindWindowA("SkiMain")
o_iconic       equ 0x6b0           ; DEBUG: IsIconic(found)
o_dc_found     equ 0x6b4           ; DEBUG: GetDC(found)
o_match        equ 0x6b8           ; DEBUG: (found == c6c8)
o_gfw          equ 0x6bc           ; DEBUG: GetForegroundWindow
o_gdw          equ 0x6c0           ; DEBUG: GetDesktopWindow
o_iswin_gfw    equ 0x80c           ; DEBUG: IsWindow(gfw result)
o_read_c6c8    equ 0x7e8           ; DEBUG: value read from [c6c8] in-tick
o_read_c624    equ 0x7f0           ; DEBUG: value read from [c624] in-tick
o_dc1          equ 0x7f4           ; DEBUG: GetDC(c6c8) #1
o_dc2          equ 0x810           ; DEBUG: GetDC(c6c8) #2 (dc1 still open)
o_dc3          equ 0x814           ; DEBUG: GetDC(c6c8) #3 (after GetClientRect)
o_read2        equ 0x7f8           ; DEBUG: [c6c8] value at gate2 site
o_hdrbuf       equ 0x100           ; PPM header buffer (16B, stable page addr)
o_namebuf      equ 0x110           ; frame name buffer (24B, stable page addr)
o_hdr_chk      equ 0x81c           ; DEBUG: first dword of hdr buffer post-wsprintf
o_wsperr       equ 0x820           ; DEBUG: GetLastError after hdr wsprintf
o_hwrite       equ 0x824           ; DEBUG: header WriteFile bytes-written
o_hrct         equ 0x828           ; DEBUG: header WriteFile rc
o_ccdc_rc      equ 0x800           ; DEBUG: CreateCompatibleDC result
o_ccbm_rc      equ 0x804           ; DEBUG: CreateCompatibleBitmap result
o_ccbm_small   equ 0x808           ; DEBUG: ccbm(o_testw,o_testh) result
o_gdc_rc       equ 0x830           ; DEBUG: GetDeviceCaps(wnddc, o_gdc_idx)
o_blt_rc       equ 0x830           ; DEBUG: BitBlt(screen->memdc) return
o_dib_rc       equ 0x834           ; DEBUG: GetDIBits(memdc) return (lines)
o_gdc_idx      equ 0x678           ; DEBUG: GDC index (python-settable; was o_diberr)
o_testw        equ 0x618           ; python-set: test width
o_testh        equ 0x61c           ; python-set: test height
o_read2a       equ 0x818           ; DEBUG: [c6c8] right after ReleaseDC
o_read2b       equ 0x7fc           ; DEBUG: [c6c8] right after GetClientRect

; ------------------------------------------------------------------- entry
orig_stub_entry:
    call [i_GetTickCount]          ; replay displaced instruction #1
                                   ; never returns through the stack (it
                                   ; jmps to tick_body); leaving it would
                                   ; misalign every subsequent ret by 4 B
    pushad
    ; ---- fast path: both homes agree and the page magic is intact ----
    mov  eax, [HOME1]
    mov  ecx, [HOME2]
    cmp  eax, ecx
    jne  .init
    test eax, eax
    jz   .init
    cmp  dword [eax], MAGIC
    jne  .init
    mov  ebp, eax
    jmp  .body

; ------------------------------------------------- page init (first tick or re-home)
.init:
    add  dword [INITCNT], 1
    mov  dword [INITSTG], 1
    push 0x1100                     ; uBytes
    push 0x40                       ; uFlags = LMEM_FIXED | LMEM_ZEROINIT
    call [i_LocalAlloc]
    test eax, eax
    jz   .inert                     ; alloc failed: retry next tick
    mov  ebp, eax
    mov  dword [INITSTG], 2
    ; ---- copy the string blob (contiguous in code, offsets mirror it) ----
    cld
    lea  esi, b_skiin
    lea  edi, [ebp+o_str_skiin]
    mov  ecx, 0x122                 ; len(b_skiin..b_gdw) incl. NULs
    rep  movsb
    mov  dword [INITSTG], 3
    ; ---- resolve function pointers ----
    ; kernel32.dll and gdi32.dll are ALREADY loaded (PE imports) —
    ; GetModuleHandleA never loads a new module, so no second-CRT
    ; initialization can happen (see header: 2026-08-29d).
    ; Handles are stored on the page and RELOADED before every call:
    ; under cdecl ecx/edx/esi/edi are caller-saved, so wine's
    ; GetProcAddress clobbers them mid-sequence.
    push b_k32
    call [i_GetModuleHandle]
    test eax, eax
    jz   .init_fail                 ; retry next tick
    mov  [ebp+o_hdl_k32], eax
    mov  dword [INITSTG], 4
    push b_gdi32
    call [i_GetModuleHandle]
    test eax, eax
    jz   .init_fail
    mov  [ebp+o_hdl_gdi], eax
    mov  dword [INITSTG], 5
    ; NOTE: string args must be lea'd — `push dword [ebp+o_str_X]` would
    ; push the string CONTENTS (first 4 bytes) as the pointer (the page
    ; holds the strings inline, not a pointer table).
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_creatf]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_creatf], eax
    mov  dword [INITSTG], 6
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_readf]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_readf], eax
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_writef]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_writef], eax
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_closeh]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_closeh], eax
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_setfp]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_setfp], eax
    mov  edx, [ebp+o_hdl_k32]
    lea  eax, [ebp+o_str_localfree]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_localfree], eax
    ; user32.dll: wsprintfA (CDECL) for frame name/header formatting
    push b_u32
    call [i_GetModuleHandle]
    test eax, eax
    jz   .init_fail
    mov  [ebp+o_hdl_u32], eax
    mov  edx, [ebp+o_hdl_u32]
    lea  eax, [ebp+o_str_wsprintf]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_wsprintf], eax
    mov  edx, [ebp+o_hdl_gdi]
    lea  eax, [ebp+o_str_ccbm]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_ccbm], eax
    mov  edx, [ebp+o_hdl_gdi]
    lea  eax, [ebp+o_str_getdib]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_getdib], eax
    ; CreateDIBSection (gdi32): local 24bpp raster, no GetDIBits needed
    mov  edx, [ebp+o_hdl_gdi]
    lea  eax, [ebp+o_str_dibsec]
    push eax
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_dibsec], eax
    ; GetWindowRect (user32) for the screen-DC capture origin
    mov  edx, [ebp+o_hdl_u32]
    push b_gwr
    push edx
    call [i_GetProcAddress]
    mov  [ebp+o_p_gwr], eax
    mov  dword [INITSTG], 12
    ; ---- reset runtime state (fresh page is zeroed; a REBUILD must not
    ;      inherit stale FILE*/DC handles from the corrupt page) ----
    mov  dword [ebp+o_file], 0
    mov  dword [ebp+o_tick], 0
    mov  dword [ebp+o_memdc], 0
    mov  dword [ebp+o_hbm], 0
    mov  dword [ebp+o_conv], 0
    mov  dword [ebp+o_pix], 0
    mov  byte [ebp+o_biso], 0x1f    ; capture+inject by default
    ; ---- publish: magic LAST (magic up => page fully initialized),
    ;      then both homes (readers require HOME1==HOME2 AND magic) ----
    mov  dword [ebp+o_magic], MAGIC
    mov  [HOME1], ebp
    mov  [HOME2], ebp
    mov  dword [INITSTG], 13
    jmp  .body
.init_fail:
    mov  dword [ebp+o_magic], 0
    mov  dword [HOME1], 0           ; force full re-init next tick
    mov  dword [HOME2], 0
    jmp  .inert

; ------------------------------------------------- per-tick body
.body:
    add  dword [ebp+o_cnt], 1      ; DEBUG
    mov  eax, [ebp+o_file]
    test eax, eax
    jnz  .do_work
    ; CreateFileA(name, GENERIC_READ|GENERIC_WRITE, 0, NULL,
    ;             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
    lea  eax, [ebp+o_str_skiin]
    push 0                           ; hTemplate
    push 0x80                        ; dwFlagsAndAttributes = NORMAL
    push 3                           ; dwCreation = OPEN_EXISTING
    push 0                           ; lpSecurityAttributes
    push 0                           ; dwShareMode
    push 0xc0000000                  ; dwDesiredAccess
    push eax                         ; lpFileName
    call [ebp+o_p_creatf]
    cmp  eax, 0xffffffff             ; INVALID_HANDLE_VALUE
    je   .inert                      ; no input file -> inert (rebuild parity)
    test eax, eax
    jz   .inert
    mov  [ebp+o_file], eax

; ------------------------------------------------- capture + write + inject
; ------------------------------------------------- capture + write + inject
; biso gates (page byte o_biso): 1=GCR 2=DIB 4=BitBlt+PPM 8=inject
;                                  0x10=GetDIBits(srcdc,0) direct (no memdc)
; frame layout: [esp+0..15] RECT, [esp+16..63] name buf, [esp+64..79] hdr
.do_work:
    sub  esp, 80
    mov  [ebp+o_align_frame], esp   ; DEBUG
    mov  dword [ebp+o_stage], 16
    mov  dword [ebp+o_dirty], 0
    ; ---- gate 1: GetClientRect + size commit ----
    test byte [ebp+o_biso], 1
    jz   .g2
    lea  ecx, [esp]                  ; lpRect
    mov  edx, [c6c8]
    push ecx
    push edx
    call [i_GetClientRect]
    mov  [ebp+o_gcr_rc], eax        ; DEBUG
    mov  ecx, [esp+8]                ; w
    mov  edx, [esp+12]               ; h
    test ecx, ecx
    jle  .g2
    test edx, edx
    jle  .g2
    mov  [ebp+o_w], ecx
    mov  [ebp+o_h], edx
    mov  eax, [c6c8]
    mov  [ebp+o_read2b], eax
    mov  eax, [ebp+o_w]
    mov  edx, [ebp+o_h]
    imul edx, eax
    cmp  edx, [ebp+o_pix]
    je   .g2
    mov  [ebp+o_pix], edx            ; commit before bmi setup clobbers edx
    mov  dword [ebp+o_dirty], 1
.g2:
    ; ---- gate 2: (re)create offscreen: memdc + compatible bitmap ----
    test byte [ebp+o_biso], 2
    jz   .g3
    test dword [ebp+o_dirty], 0
    jnz  .g2go
    mov  eax, [ebp+o_memdc]
    test eax, eax
    jnz  .g3                  ; already have offscreen: keep it
.g2go:
    mov  eax, [ebp+o_memdc]
    test eax, eax
    jz   .no_dc
    push eax
    call [i_DeleteDC]
    mov  dword [ebp+o_memdc], 0
.no_dc:
    mov  eax, [ebp+o_hbm]
    test eax, eax
    jz   .no_hbm
    push eax
    call [i_DeleteObject]
    mov  dword [ebp+o_hbm], 0
.no_hbm:
    mov  eax, [ebp+o_conv]
    test eax, eax
    jz   .no_conv
    push eax
    call [ebp+o_p_localfree]
    mov  dword [ebp+o_conv], 0
.no_conv:
    ; offscreen = CreateCompatibleDC(NULL) + 24bpp top-down DIB section.
    ; The section raster is LOCAL memory (o_bits) — read it directly; no
    ; GetDIBits (it times out on this process's X-backed compatible DDBs).
    push 0
    call [i_CreateCompatibleDC]
    mov  [ebp+o_ccdc_rc], eax        ; DEBUG
    test eax, eax
    jz   .g3
    mov  ebx, eax                    ; memdc
    ; 24bpp BGR top-down descriptor in o_bmi (biHeight = -h)
    mov  eax, [ebp+o_w]
    mov  dword [ebp+o_bmi], 40       ; biSize
    mov  [ebp+o_bmi+4], eax          ; biWidth
    mov  eax, [ebp+o_h]
    neg  eax
    mov  [ebp+o_bmi+8], eax          ; biHeight (negative = top-down)
    mov  dword [ebp+o_bmi+12], 0x00180001  ; planes=1, bitcount=24
    mov  dword [ebp+o_bmi+20], 0     ; BI_RGB
    ; CreateDIBSection(NULL, &bmi, 0, &bits, NULL, 0)  ret 24
    push 0                           ; dwAccess
    push 0                           ; hSection
    lea  ecx, [ebp+o_bits]
    push ecx                         ; ppvBits
    push 0                           ; usage
    lea  ecx, [ebp+o_bmi]
    push ecx                         ; pbmi
    push 0                           ; hdc
    call [ebp+o_p_dibsec]
    mov  [ebp+o_ccbm_rc], eax        ; DEBUG: dibsec result
    test eax, eax
    jz   .g3
.ccbm_ok:
    mov  [ebp+o_hbm], eax            ; save (SelectObject clobbers)
    mov  [ebp+o_memdc], ebx
    push dword [ebp+o_hbm]
    push ebx
    call [i_SelectObject]
.g3:
    ; ---- gate 3: capture via the SCREEN DC + PPM write ----
    ; The window's own DC reads back a cleared scratch buffer between
    ; the game's continuous repaints (all-black captures); the screen
    ; DC at the client origin reads the real display content instead.
    test byte [ebp+o_biso], 4
    jz   .g4
    mov  eax, [ebp+o_bits]
    test eax, eax
    jz   .g4
    mov  eax, [c6c8]                 ; main HWND
    push eax
    call [i_GetDC]
    mov  esi, eax                    ; src dc = window dc (0 on fail)
    test esi, esi
    jz   .rel0
    ; ---- BitBlt(wnddc -> memdc) at (0,0): the DIB section is local
    ;      memory so the window-DC read lands in a stable buffer ----
    push 0x00cc0020                  ; SRCCOPY (rop)
    push 0                           ; ySrc
    push 0                           ; xSrc
    push esi                         ; hSrc = window dc
    push dword [ebp+o_h]             ; wHeight
    push dword [ebp+o_w]             ; wWidth
    push 0                           ; yDest
    push 0                           ; xDest
    push dword [ebp+o_memdc]         ; hDest
    call [i_BitBlt]
    mov  [ebp+o_blt_rc], eax         ; DEBUG: BitBlt rc
    mov  dword [ebp+o_stage], 5
.rel0:
    mov  eax, [c6c8]                 ; ReleaseDC(hwnd, hdc)
    push eax
    push esi                         ; src dc (0 on fail)
    call [i_ReleaseDC]
    ; ---- wsprintfA (CDECL, ret 0): header, then frame name ----
    lea  edi, [ebp+o_hdrbuf]
    push dword [ebp+o_h]
    push dword [ebp+o_w]
    lea  eax, [ebp+o_str_p6hdr]
    push eax
    push edi
    call [ebp+o_p_wsprintf]
    add  esp, 16
    mov  [ebp+o_hdrlen], eax
    mov  ecx, [ebp+o_hdrbuf]
    mov  [ebp+o_hdr_chk], ecx
    test eax, eax
    jnz  .wsp_ok
.wsp_ok:
    lea  edi, [ebp+o_namebuf]
    push dword [ebp+o_tick]
    lea  eax, [ebp+o_str_frame]
    push eax
    push edi
    call [ebp+o_p_wsprintf]
    add  esp, 12
    mov  dword [ebp+o_stage], 7
    ; CreateFileA(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, NORMAL, NULL)
    lea  eax, [ebp+o_namebuf]         ; frame name buffer
    push 0                           ; hTemplate
    push 0x80                        ; dwFlagsAndAttributes = NORMAL
    push 2                           ; dwCreation = CREATE_ALWAYS
    push 0                           ; lpSecurityAttributes
    push 0                           ; dwShareMode
    push 0x40000000                  ; dwDesiredAccess = GENERIC_WRITE
    push eax
    call [ebp+o_p_creatf]
    cmp  eax, 0xffffffff
    je   .g4
    test eax, eax
    jz   .g4
    mov  [ebp+o_ppm_hnd], eax
    ; WriteFile header (o_hdrbuf, o_hdrlen B) then the pixel raster from
    ; the DIB section (o_bits, w*3*h BGR) — two writes, no copy.
    lea  ecx, [ebp+o_io32]
    push 0
    push ecx                         ; lpNumberOfBytesWritten
    push dword [ebp+o_hdrlen]
    lea  eax, [ebp+o_hdrbuf]
    push eax                         ; lpBuffer = address (not contents)
    push dword [ebp+o_ppm_hnd]
    call [ebp+o_p_writef]
    mov  [ebp+o_hwrite], eax         ; DEBUG: header WriteFile rc
    mov  eax, [ebp+o_io32]
    mov  [ebp+o_stgret], eax         ; DEBUG: header bytes written
    mov  eax, [ebp+o_pix]
    lea  edx, [eax*3]                ; byte count = w*h*3
    lea  ecx, [ebp+o_io32]           ; ecx clobbered by the call above
    push 0
    push ecx                         ; lpNumberOfBytesWritten
    push edx
    push dword [ebp+o_bits]
    push dword [ebp+o_ppm_hnd]
    call [ebp+o_p_writef]
    mov  [ebp+o_hrct], eax           ; DEBUG: pixels WriteFile rc
    mov  dword [ebp+o_stage], 8
    ; CloseHandle
    push dword [ebp+o_ppm_hnd]
    call [ebp+o_p_closeh]
    mov  dword [ebp+o_ppm_hnd], 0
.g4:
    ; ---- gate 4: input read + inject ----
    test byte [ebp+o_biso], 8
    jz   .tail0
    ; SetFilePointerEx(h, dist, &high, method, flags) — 5 args, ret 20
    mov  eax, [ebp+o_tick]
    lea  edx, [eax*2]
    lea  ecx, [ebp+o_io32]
    mov  dword [ecx], 0              ; lpDistanceHigh (unused)
    push 0                           ; dwFileFlags
    push 0                           ; FILE_BEGIN
    push ecx
    push edx                         ; low distance
    push dword [ebp+o_file]
    call [ebp+o_p_setfp]
    ; ReadFile(h, buf, 2, &bytes, NULL)
    ; NB: reload &bytes after SetFilePointerEx (it clobbers ecx).
    lea  eax, [ebp+o_bitsld]
    lea  ecx, [ebp+o_io32]
    mov  dword [eax], 0              ; EOF guard (ReadFile may read 0)
    push 0
    push ecx                         ; lpNumberOfBytesRead
    push 2
    push eax
    push dword [ebp+o_file]
    call [ebp+o_p_readf]
    mov  dword [ebp+o_stage], 11
    movzx eax, word [ebp+o_bitsld]
    test eax, eax
    jz   .tail0
    ; VK table (seed.json bit order): left right up down crouch f1 f3 f7 f9 F2 F3 Enter
    mov  dword [esp+16], 0x25
    mov  dword [esp+20], 0x27
    mov  dword [esp+24], 0x26
    mov  dword [esp+28], 0x28
    mov  dword [esp+32], 0x60
    mov  dword [esp+36], 0x61
    mov  dword [esp+40], 0x63
    mov  dword [esp+44], 0x67
    mov  dword [esp+48], 0x69
    mov  dword [esp+52], 0x71
    mov  dword [esp+56], 0x72
    mov  dword [esp+60], 0x0d
    mov  ecx, 1
    mov  edi, 12
    lea  edx, [esp+16]
.bit:
    test eax, ecx
    jz   .nbit
    ; wproc_main is the game's WndProc: stdcall (ret 0x10), clobbers all
    ; caller-saved regs and ebp. Save loop state, recover ebp from HOME1.
    mov  [ebp+o_bitstk], ecx
    mov  [ebp+o_bitstk_edx], edx
    mov  [ebp+o_bitstk_edi], edi
    mov  ebx, [edx]
    push 0                           ; lparam
    push ebx                         ; vk
    push 0x100                       ; WM_KEYDOWN
    push dword [c6c8]                ; hwnd
    call [wproc_main]                ; stdcall (ret 0x10): esp auto-restored
    mov  ebp, [HOME1]                ; wproc clobbers ebp: recover page base
    mov  dword [ebp+o_stage], 12
    mov  ecx, [ebp+o_bitstk]
    mov  edx, [ebp+o_bitstk_edx]
    mov  edi, [ebp+o_bitstk_edi]
.nbit:
    shl  ecx, 1
    add  edx, 4
    dec  edi
    jnz  .bit
.tail0:
    inc  dword [ebp+o_tick]
    mov  dword [ebp+o_stage], 14
    add  esp, 80
    popad
    jmp  tick_body
.inert:
    popad
    jmp  tick_body

; ------------------------------------------------- string blob (read-only)
; b_skiin..b_dibsec is ONE contiguous 0x9b-byte region; the single
; `rep movsb` in .init_page copies it to [ebp+o_str_skiin].
b_skiin:    db "ski_in.bin", 0
b_frame:    db "frame_%06u_main.ppm", 0
b_p6hdr:    db "P6", 10, "%d %d", 10, "255", 10, 0
b_gdi32:    db "gdi32.dll", 0
b_k32:      db "kernel32.dll", 0
b_creatf:   db "CreateFileA", 0
b_readf:    db "ReadFile", 0
b_writef:   db "WriteFile", 0
b_closeh:   db "CloseHandle", 0
b_setfp:    db "SetFilePointerEx", 0
b_localfree: db "LocalFree", 0
b_dibsec:   db "CreateDIBSection", 0
b_getlasterr: db "GetLastError", 0
b_u32: db "user32.dll", 0
b_wsprintf: db "wsprintfA", 0
b_ccbm: db "CreateCompatibleBitmap", 0
b_getdib: db "GetDIBits", 0
b_getclass: db "GetClassNameA", 0
b_iswin: db "IsWindow", 0
b_skimain: db "SkiMain", 0
b_gfw: db "GetForegroundWindow", 0
b_gdw:     db "GetDesktopWindow", 0
b_gdc:     db "GetDeviceCaps", 0
b_gwr:     db "GetWindowRect", 0
