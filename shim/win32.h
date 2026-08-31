/* Shim API — exact symbol set the ski32 rebuild links against (T16 probe).
 * Signatures: 32-bit Windows SDK shapes (under wasm32 all pointers are 32-bit).
 * Task 17–20 implement exactly this list (plus the EMSCRIPTEN_KEEPALIVE
 * debug hooks from Task 18/21 in canvas.c).
 */
#ifndef SHIM_WIN32_H
#define SHIM_WIN32_H
#include "types.h"

/* ---- user32: windows ---- */
ATOM           RegisterClassA(const WNDCLASSA *wc);
HWND           CreateWindowExA(DWORD ex, LPCSTR cls, LPCSTR title, DWORD style,
                               int x, int y, int w, int h,
                               HWND parent, void *menu, HINSTANCE inst, void *param);
HWND           FindWindowA(LPCSTR cls, LPCSTR title);
void           MoveWindow(HWND h, int x, int y, int w, int hh, BOOL repaint);
BOOL           SetWindowPos(HWND h, HWND after, int x, int y, int w, int hh, UINT flags);
BOOL           SetWindowTextA(HWND h, LPCSTR s);
BOOL           ShowWindow(HWND h, int cmd);
BOOL           UpdateWindow(HWND h);
HWND           SetFocus(HWND h);
BOOL           InvalidateRect(HWND h, const RECT *rc, BOOL erase);
BOOL           KillTimer(HWND h, UINT id);
UINT           SetTimer(HWND h, UINT id, UINT ms, TIMERPROC fn);
BOOL           BeginPaint(HWND h, PAINTSTRUCT *ps);
BOOL           EndPaint(HWND h, const PAINTSTRUCT *ps);
HDC            GetDC(HWND h);
int            ReleaseDC(HWND h, HDC dc);
BOOL           GetClientRect(HWND h, RECT *rc);
LRESULT        DefWindowProcA(HWND h, UINT msg, WPARAM wp, LPARAM lp);
void           PostQuitMessage(int code);
int            GetMessageA(MSG *msg, HWND q, UINT min, UINT max);
BOOL           TranslateMessage(const MSG *msg);
LRESULT        DispatchMessageA(const MSG *msg);
BOOL           DestroyWindow(HWND h);
BOOL           IsIconic(HWND h);
HICON          LoadIconA(HINSTANCE inst, LPCSTR name);
HCURSOR        LoadCursorA(HINSTANCE inst, LPCSTR name);
HICON          OpenIcon(HICON icon);
int            MessageBoxA(HWND owner, LPCSTR text, LPCSTR caption, UINT type);
int            LoadStringA(HINSTANCE inst, UINT id, LPSTR buf, int max);
void *         LoadBitmapA(HINSTANCE inst, LPCSTR name);
int            wsprintfA(LPSTR buf, LPCSTR fmt, ...);
BOOL           FillRect(HDC dc, const RECT *rc, HBRUSH brush);
BOOL           FrameRect(HDC dc, const RECT *rc, HBRUSH brush);

/* ---- gdi32: surfaces ---- */
HDC            CreateCompatibleDC(HDC ref);
BOOL           DeleteDC(HDC dc);
HBITMAP        CreateCompatibleBitmap(HDC ref, int w, int h);
HBITMAP        CreateBitmap(int w, int h, int planes, int bpp, const void *bits);
HBITMAP        CreateDIBSection(HDC dc, const BITMAPINFO *bmi, UINT usage,
                                void **bits, HANDLE file, DWORD off);
HGDIOBJ        SelectObject(HDC dc, HGDIOBJ obj);
BOOL           DeleteObject(HGDIOBJ obj);
BOOL           GetObjectA(HANDLE obj, int cnt, void *out);
BOOL           BitBlt(HDC dst, int x, int y, int w, int h,
                      HDC src, int sx, int sy, DWORD rop);
BOOL           PatBlt(HDC dc, int x, int y, int w, int h, DWORD rop);
HBRUSH         GetStockObject(int idx);
BOOL           TextOutA(HDC dc, int x, int y, LPCSTR s, int cnt);
BOOL           GetTextExtentPoint32A(HDC dc, LPCSTR s, int cnt, SIZE *sz);
BOOL           GetTextMetricsA(HDC dc, TEXTMETRICS *tm);
int            GetDeviceCaps(HDC dc, int index);

/* ---- kernel32 ---- */
DWORD          GetTickCount(void);
DWORD          GetProcessId(HANDLE proc);
HLOCAL         LocalAlloc(UINT flags, DWORD bytes);
BOOL           FreeLibrary(HMODULE mod);
HRSRC          FindResourceA(HMODULE mod, LPCSTR name, LPCSTR type);
HGLOBAL        LoadResource(HMODULE mod, HRSRC res);
void *         LockResource(HGLOBAL res);
void           FreeResource(HGLOBAL res);
int            GetPrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR def,
                                        LPSTR buf, int size, LPCSTR fname);
int            WritePrivateProfileStringA(LPCSTR section, LPCSTR key, LPCSTR val,
                                          LPCSTR fname);
LPSTR          lstrlenA(LPCSTR s);
LPSTR          lstrcpyA(LPSTR dst, LPCSTR src);
int            lstrcmpiA(LPCSTR a, LPCSTR b);

/* ---- winmm ---- */
BOOL           PlaySoundA(LPCSTR name, HMODULE mod, DWORD flags);

#endif /* SHIM_WIN32_H */
