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
; Memory model (2026-08-29 rewrite): ALL mutable stub state lives in a
; VirtualAlloc'd page (0x1000 B, PAGE_READWRITE) allocated at first tick.
; The page base is kept redundantly in HOME1+HOME2. Guard: HOME1==HOME2,
; nonzero, and [base+0]==MAGIC (0x534b4946 "SKIF"); any mismatch makes
; the stub re-initialize on the next tick (the orphaned page leaks —
; acceptable for a capture harness). No stub state besides the 8 home
; bytes is stored in .data, and those 8 bytes sit in a verified-dead
; block, so no collision with game globals.
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
; then software 32->24 convert, one fwrite. CreateDIBSection is NOT in
; the import table (static CRT, no MSVCRT import): resolved at first
; tick via GetProcAddress(GetModuleHandleA(...)).

bits 32
org 0x4096ac

; ---------------------------------------------------------------- IAT slots
i_DeleteObject   equ 0x40a000
i_SelectObject   equ 0x40a004
i_BitBlt         equ 0x40a00c
i_DeleteDC       equ 0x40a028
i_CreateCompatibleDC equ 0x40a034
i_LoadLibraryA   equ 0x40a068
i_GetProcAddress equ 0x40a074
i_VirtualAlloc   equ 0x40a080
i_GetTickCount   equ 0x40a0c0
i_LocalAlloc     equ 0x40a0c4
i_GetModuleHandle equ 0x40a0c8
i_wsprintfA      equ 0x40a108
i_GetClientRect  equ 0x40a14c
i_SetWindowTextA equ 0x40a164
i_GetDC          equ 0x40a16c
i_ReleaseDC      equ 0x40a170

wproc_main       equ 0x405800      ; SkiMain WndProc (direct call)
c6c8             equ 0x40c6c8      ; main HWND global
HOME1            equ 0x40c284      ; page-base home #1 (4 B, .data)
HOME2            equ 0x40c288      ; page-base home #2 (redundant copy)
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
o_str_skiin    equ 0x100           ; "ski_in.bin\0"            11
o_str_frame    equ 0x10b           ; "frame_%06u_main.ppm\0"   20
o_str_wb       equ 0x11f           ; "wb\0"                    3
o_str_rb       equ 0x122           ; "rb\0"                    3
o_str_p6hdr    equ 0x125           ; "P6\n%d %d\n255\n\0"      13
o_str_msvcrt   equ 0x132           ; "msvcrt.dll\0"            11
o_str_gdi32    equ 0x13d           ; "gdi32.dll\0"             10
o_str_fopen    equ 0x147           ; "fopen\0"                 6
o_str_fread    equ 0x14d           ; "fread\0"                 6
o_str_fseek    equ 0x153           ; "fseek\0"                 6
o_str_fwrite   equ 0x159           ; "fwrite\0"                7
o_str_fclose   equ 0x160           ; "fclose\0"                7
o_str_localfree equ 0x167          ; "LocalFree\0"             10
o_str_dibsec   equ 0x173           ; "CreateDIBSection\0"      17 -> ends 0x184
; function pointers @+0x200
o_p_fopen      equ 0x200
o_p_fread      equ 0x204
o_p_fseek      equ 0x208
o_p_fwrite     equ 0x20c
o_p_fclose     equ 0x210
o_p_dibsec     equ 0x214
o_p_localfree  equ 0x218
; DIB state @+0x300
o_memdc        equ 0x300
o_hbm          equ 0x304
o_bits         equ 0x308           ; 32bpp top-down raster
o_pix          equ 0x30c
o_conv         equ 0x310           ; 24bpp convert buffer (LocalAlloc)
o_convlen      equ 0x314
o_w            equ 0x318
o_h            equ 0x31c
; BITMAPINFO @+0x400 (40 B), DIBSection bits holder @+0x500
o_bmi          equ 0x400
o_bitsld       equ 0x500
; runtime @+0x600
o_file         equ 0x600           ; FILE* ski_in.bin (0 = not open)
o_tick         equ 0x604           ; 0-based frame/word index

; ------------------------------------------------------------------- entry
orig_stub_entry:
    call [i_GetTickCount]          ; replay displaced instruction #1
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
    push 0x04                       ; PAGE_READWRITE
    push 0x11000                    ; MEM_COMMIT|MEM_RESERVE
    push 0x1000                     ; size
    push 0                          ; any address
    call [i_VirtualAlloc]
    add  esp, 16
    test eax, eax
    jz   .inert                     ; alloc failed: retry next tick
    mov  ebp, eax
    ; ---- copy the string blob (contiguous in code, offsets mirror it) ----
    cld
    lea  esi, b_skiin
    lea  edi, [ebp+o_str_skiin]
    mov  ecx, 0x82                  ; len(b_skiin..b_dibsec) incl. NULs
    rep  movsb
    ; ---- resolve function pointers ----
    push b_msvcrt
    call [i_LoadLibraryA]
    add  esp, 4
    test eax, eax
    jz   .init_fail                 ; retry next tick
    mov  ebx, eax                   ; msvcrt handle
    push b_gdi32
    call [i_GetModuleHandle]
    add  esp, 4
    test eax, eax
    jz   .init_fail
    mov  ecx, eax                   ; gdi32 handle
    push dword [ebp+o_str_fopen]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_fopen], eax
    push dword [ebp+o_str_fread]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_fread], eax
    push dword [ebp+o_str_fseek]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_fseek], eax
    push dword [ebp+o_str_fwrite]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_fwrite], eax
    push dword [ebp+o_str_fclose]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_fclose], eax
    push dword [ebp+o_str_localfree]
    push ebx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_localfree], eax
    push dword [ebp+o_str_dibsec]
    push ecx
    call [i_GetProcAddress]
    add  esp, 8
    mov  [ebp+o_p_dibsec], eax
    ; ---- reset runtime state (fresh page is zeroed; a REBUILD must not
    ;      inherit stale FILE*/DC handles from the corrupt page) ----
    mov  dword [ebp+o_file], 0
    mov  dword [ebp+o_tick], 0
    mov  dword [ebp+o_memdc], 0
    mov  dword [ebp+o_hbm], 0
    mov  dword [ebp+o_conv], 0
    mov  dword [ebp+o_pix], 0
    ; ---- publish: magic LAST (magic up => page fully initialized),
    ;      then both homes (readers require HOME1==HOME2 AND magic) ----
    mov  dword [ebp+o_magic], MAGIC
    mov  [HOME1], ebp
    mov  [HOME2], ebp
    jmp  .body
.init_fail:
    mov  dword [ebp+o_magic], 0
    mov  dword [HOME1], 0           ; force full re-init next tick
    mov  dword [HOME2], 0
    jmp  .inert

; ------------------------------------------------- per-tick body
.body:
    mov  eax, [ebp+o_file]
    test eax, eax
    jnz  .do_work
    push dword [ebp+o_str_rb]
    push dword [ebp+o_str_skiin]
    call [ebp+o_p_fopen]
    add  esp, 8
    mov  [ebp+o_file], eax
    test eax, eax
    jz   .inert                      ; no input file -> inert (rebuild parity)

; ------------------------------------------------- capture + write + inject
.do_work:
    sub  esp, 80
    ; [esp+0..15] RECT, [esp+16..63] name buf, [esp+64..79] header buf
    lea  ecx, [esp]                  ; lpRect
    mov  eax, [c6c8]
    push ecx
    push eax
    call [i_GetClientRect]
    add  esp, 8
    mov  ecx, [esp+8]                ; w
    mov  edx, [esp+12]               ; h
    test ecx, ecx
    jle  .inject
    test edx, edx
    jle  .inject
    mov  [ebp+o_w], ecx
    mov  [ebp+o_h], edx
    imul edx, ecx                    ; pix
    cmp  edx, [ebp+o_pix]
    je   .blt
    mov  [ebp+o_pix], edx            ; commit before bmi setup clobbers edx
    ; ---- (re)create DIB section for the new size ----
    mov  eax, [ebp+o_memdc]
    test eax, eax
    jz   .no_dc
    push eax
    call [i_DeleteDC]
    add  esp, 4
.no_dc:
    mov  eax, [ebp+o_hbm]
    test eax, eax
    jz   .no_hbm
    push eax
    call [i_DeleteObject]
    add  esp, 4
.no_hbm:
    mov  eax, [ebp+o_conv]
    test eax, eax
    jz   .no_conv
    push eax
    call [ebp+o_p_localfree]
    add  esp, 4
    mov  dword [ebp+o_conv], 0
.no_conv:
    lea  eax, [ebp+o_bmi]
    mov  dword [eax], 40             ; biSize
    mov  [eax+4], ecx                ; biWidth
    neg  edx
    mov  [eax+8], edx                ; biHeight = -h (top-down)
    mov  dword [eax+12], 0x00200001  ; planes=1, bitcount=32
    mov  dword [eax+20], 0           ; BI_RGB
    lea  edx, [ebp+o_bitsld]
    push 0                           ; dwOffset
    push 0                           ; hSection
    push edx                         ; lppBits
    lea  ecx, [ebp+o_bmi]
    push ecx                         ; pbmi
    push 0                           ; hdc
    call [ebp+o_p_dibsec]
    add  esp, 20
    test eax, eax
    jz   .inject                     ; capture failed: skip frame, still inject
    mov  [ebp+o_hbm], eax
    mov  esi, [ebp+o_bitsld]
    mov  [ebp+o_bits], esi
    push 0
    call [i_CreateCompatibleDC]
    add  esp, 4
    test eax, eax
    jz   .inject
    mov  [ebp+o_memdc], eax
    push dword [ebp+o_hbm]
    push eax
    call [i_SelectObject]
    add  esp, 8
    mov  ecx, [ebp+o_pix]
    lea  edx, [ecx*3]
    push edx                         ; bytes
    push 0                           ; LPTR
    call [i_LocalAlloc]
    add  esp, 8
    test eax, eax
    jz   .inject
    mov  [ebp+o_conv], eax
    mov  [ebp+o_convlen], edx
.blt:
    mov  eax, [ebp+o_memdc]
    test eax, eax
    jz   .inject
    mov  edx, [c6c8]
    push edx
    call [i_GetDC]
    add  esp, 4
    mov  esi, eax                    ; src dc (0 on fail)
    test esi, esi
    jz   .rel0
    push 0x00cc0020                  ; SRCCOPY
    push 0
    push 0
    push esi
    push dword [ebp+o_h]
    push dword [ebp+o_w]
    push 0
    push 0
    push dword [ebp+o_memdc]
    call [i_BitBlt]
    add  esp, 36
.rel0:
    push esi                         ; src dc (0 on fail)
    mov  edx, [c6c8]
    push edx
    call [i_ReleaseDC]
    add  esp, 8
    ; ---- build name, open, write ----
    lea  eax, [esp+16]
    push dword [ebp+o_tick]
    push dword [ebp+o_str_frame]
    push eax
    call [i_wsprintfA]
    add  esp, 12
    lea  esi, [esp+64]
    push dword [ebp+o_h]
    push dword [ebp+o_w]
    push dword [ebp+o_str_p6hdr]
    push esi
    call [i_wsprintfA]
    add  esp, 16
    push dword [ebp+o_str_wb]
    push dword [esp+16]
    call [ebp+o_p_fopen]
    add  esp, 8
    test eax, eax
    jz   .no_ppm
    mov  ebx, eax                    ; FILE* (survives the convert loop)
    push esi                         ; buf (header ptr)
    push eax                         ; len (wsprintf return)
    push 1
    push ebx                         ; FILE*
    call [ebp+o_p_fwrite]
    add  esp, 16
    ; ---- 32bpp -> 24bpp convert (top-down, BGR order kept) ----
    mov  esi, [ebp+o_bits]
    mov  edi, [ebp+o_conv]
    mov  ecx, [ebp+o_pix]
    test ecx, ecx
    jz   .wrote
.cvt:
    lodsb
    mov  [edi], al
    lodsb
    mov  [edi+1], al
    lodsb
    mov  [edi+2], al
    lodsb                            ; drop alpha
    add  edi, 3
    loop .cvt
.wrote:
    push dword [ebp+o_conv]          ; buf
    push dword [ebp+o_convlen]
    push 1
    push ebx                         ; FILE*
    call [ebp+o_p_fwrite]
    add  esp, 16
    push ebx
    call [ebp+o_p_fclose]
    add  esp, 4
.no_ppm:
.inject:
    ; ---- inject word[s_tick] ----
    mov  eax, [ebp+o_tick]
    lea  edx, [eax*2]
    push 0                           ; SEEK_SET
    push edx
    push dword [ebp+o_file]
    call [ebp+o_p_fseek]
    add  esp, 12
    lea  eax, [ebp+o_bitsld]
    mov  dword [eax], 0              ; EOF guard (fread may read 0)
    push eax                         ; buf
    push 2                           ; size
    push 1                           ; nmemb
    push dword [ebp+o_file]          ; FILE*
    call [ebp+o_p_fread]
    add  esp, 16
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
    mov  ebx, [edx]
    push 0                           ; lparam
    push ebx                         ; vk
    push 0x100                       ; WM_KEYDOWN
    push dword [c6c8]                ; hwnd
    call wproc_main                  ; cdecl: ebp (base) preserved
    add  esp, 16
.nbit:
    shl  ecx, 1
    add  edx, 4
    dec  edi
    jnz  .bit
.tail0:
    inc  dword [ebp+o_tick]
    add  esp, 80
    popad
    jmp  tick_body
.inert:
    popad
    jmp  tick_body

; ------------------------------------------------- string blob (read-only)
; b_skiin..b_dibsec is ONE contiguous 0x82-byte region; the single
; `rep movsb` in .init_page copies it to [ebp+o_str_skiin]. The two
; bootstrap strings below it are used at code addresses only and are
; NOT part of the copied region.
b_skiin:    db "ski_in.bin", 0
b_frame:    db "frame_%06u_main.ppm", 0
b_wb:       db "wb", 0
b_rb:       db "rb", 0
b_p6hdr:    db "P6", 10, "%d %d", 10, "255", 10, 0
b_msvcrt:   db "msvcrt.dll", 0
b_gdi32:    db "gdi32.dll", 0
b_fopen:    db "fopen", 0
b_fread:    db "fread", 0
b_fseek:    db "fseek", 0
b_fwrite:   db "fwrite", 0
b_fclose:   db "fclose", 0
b_localfree: db "LocalFree", 0
b_dibsec:   db "CreateDIBSection", 0
