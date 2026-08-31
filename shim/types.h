/* Win32 types for the ski32 emscripten shim. Pure types only (no
 * emscripten headers) so host-side unit tests can include surface.h.
 * Layouts match the 32-bit Windows SDK (IMAGE = 32-bit; the rebuilt game
 * code was compiled and diffed against mingw-w64 i686).
 */
#ifndef SHIM_TYPES_H
#define SHIM_TYPES_H
#include <stdint.h>

typedef int          BOOL;
typedef uint8_t      BYTE;
typedef uint16_t     WORD;
typedef uint32_t     DWORD;
typedef int32_t      LONG;
typedef uint32_t     UINT;
typedef DWORD        COLORREF;
typedef int32_t      LONG_PTR;
typedef uint32_t     DWORD_PTR;
typedef DWORD        UINT_PTR;
typedef LONG_PTR     LPARAM;
typedef DWORD_PTR    WPARAM;
typedef LONG         LRESULT;
typedef UINT         ATOM;
#define CALLBACK
#define WINAPI

typedef void *HWND;
typedef void *HDC;
typedef void *HBITMAP;
typedef void *HFONT;
typedef void *HBRUSH;
typedef void *HICON;
typedef void *HCURSOR;
typedef void *HINSTANCE;
typedef void *HMODULE;
typedef void *HANDLE;
typedef void *HGDIOBJ;
typedef void *HRSRC;
typedef void *HGLOBAL;
typedef void *HLOCAL;
typedef void *LPVOID;
typedef void **LPVOIDP;

typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL (CALLBACK *TIMERPROC)(HWND, UINT, UINT, DWORD);

typedef char *LPSTR;
typedef const char *LPCSTR;

typedef struct { int x, y; } POINT;
typedef struct { int cx, cy; } SIZE;
typedef struct { int left, top, right, bottom; } RECT;
typedef struct { HDC hdc; RECT rcPaint; BOOL fErase; } PAINTSTRUCT;
typedef struct { HWND hwnd; UINT msg; WPARAM wParam; LPARAM lParam; } MSG;

typedef struct { DWORD bmType; int bmWidth; int bmHeight; int bmWidthBytes;
                 WORD bmPlanes; WORD bmBitsPixel; void *bmBits; } BITMAP;
typedef struct { int biSize; int biWidth; int biHeight; WORD biPlanes;
                 WORD biBitCount; DWORD biCompression; DWORD biSizeImage;
                 int biXPelsPerMeter; int biYPelsPerMeter; DWORD biClrUsed;
                 DWORD biClrImportant; } BITMAPINFOHEADER;
typedef struct { BITMAPINFOHEADER bmiHeader; DWORD bmiColors[1]; } BITMAPINFO;

typedef struct {
    int tmHeight; int tmAscent; int tmDescent; int tmInternalLeading;
    int tmExternalLeading; int tmAveCharWidth; int tmMaxCharWidth;
    int tmWeight; int tmOverhang; int tmDigitAlignmentAspect;
    BYTE tmFirstChar; BYTE tmLastChar; BYTE tmDefaultChar; BYTE tmBreakChar;
    BYTE tmItalic; BYTE tmUnderlined; BYTE tmStruckOut;
} TEXTMETRICS;

typedef struct {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASSA;

typedef TEXTMETRICS TEXTMETRICA;

/* --- constants (only those the game uses) --- */
#define TRUE  1
#define FALSE 0

#define WM_NULL         0x0000
#define WM_CREATE       0x0001
#define WM_DESTROY      0x0002
#define WM_SIZE         0x0005
#define WM_CLOSE        0x0010
#define WM_QUIT         0x0012
#define WM_PAINT        0x000F
#define WM_TIMER        0x0113
#define WM_COMMAND      0x0111
#define WM_KEYDOWN      0x0100
#define WM_KEYUP        0x0101
#define WM_CHAR         0x0102
#define WM_MOUSEMOVE    0x0200
#define WM_LBUTTONDOWN  0x0201
#define WM_LBUTTONUP    0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_MOUSEACTIVATE 0x0021
#define WM_GETMINMAXINFO 0x0024
#define WM_ACTIVATE     0x0006
#define WM_NCCREATE     0x0081
#define WA_ACTIVATE     1

#define SW_HIDE         0
#define SW_SHOWNORMAL   1
#define SW_NORMAL       1
#define SW_SHOW         5
#define SW_MINIMIZE     6

#define WS_VISIBLE      0x10000000
#define CS_HREDRAW      0x0002
#define CS_VREDRAW      0x0001
#define CS_DBLCLKS      0x0008

#define SWP_NOSIZE      0x0001
#define SWP_NOMOVE      0x0002
#define SWP_NOZORDER    0x0004
#define SWP_SHOWWINDOW  0x0040

#define SRCCOPY         0x00CC0020
#define BLACKNESS       0x00000042
#define WHITENESS       0x000000FF
#define SRCINVERT       0x00550009
#define MERGECOPY       0x00C000CA
#define PATINVERT       0x005A0049
#define DSTINVERT       0x550009
#define NOTSRCCOPY      0x330008
#define NOTDESTINATION  0x00550009
#define CAPTUREBLT      0x40000000

#define DIB_RGB_COLORS  0
#define DIB_PAL_COLORS  1

/* Stock objects: real Win32 indices (5..14). The game passes RAW literals
 * 0, 4, 10 (ski_win.c:775/729/686) — 0 and 4 are OUT of range, so real GDI
 * returns NULL for them (the FillRect/FrameRect calls no-op; the M2
 * reference has no status-panel frame); 10 = BLACK_BRUSH (SelectObject'd
 * into the status DC, never drawn with). */
#define WHITE_BRUSH     6
#define LTGRAY_BRUSH    7
#define GRAY_BRUSH      8
#define DKGRAY_BRUSH    9
#define BLACK_BRUSH     10
#define NULL_BRUSH      5
#define HOLLOW_BRUSH    5
#define WHITE_PEN       11
#define BLACK_PEN       12
#define NULL_PEN        13
#define DEFAULT_GUI_FONT 13

#define HORZRES         8
#define VERTRES         10
#define LOGPIXELSX      88
#define LOGPIXELSY      90
#define BITSPIXEL       12
#define PLANESPIXEL     11

#define MB_OK           0x00000000
#define MB_ICONINFORMATION 0x00000040
#define IDOK            1
#define IDCANCEL        2
#define IDABORT         3
#define IDRETRY         4
#define IDYES           6
#define IDNO            7

#define GDI_ERROR       ((UINT)-1)
#define INVALID_HANDLE_VALUE ((HANDLE)(long long)-1)

#define SND_SYNC        0x0000
#define SND_ASYNC       0x0001
#define SND_NODEFAULT   0x0002
#define SND_LOOP        0x0008
#define SND_NOSTOP      0x0010

#define GMEM_MOVEABLE   0x0002
#define GMEM_ZEROINIT   0x0040
#define LMEM_FIXED      0x0000
#define LMEM_MOVEABLE   0x0002
#define LMEM_NOCLEAR    0x0064

#define IMAGE_BITMAP    0
#define LR_LOADFROMFILE 0x10

#define MAKEINTRESOURCEA(i) ((LPCSTR)(uintptr_t)(unsigned int)(i))
#define MAKEINTRESOURCE(i)  MAKEINTRESOURCEA(i)

#endif /* SHIM_TYPES_H */
