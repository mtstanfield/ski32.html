#include <windows.h>
#include <stdio.h>
int WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, int s) {
    FILE *f = fopen("gwrtest.txt", "wb");
    HMODULE u = GetModuleHandle("user32.dll");
    FARPROC gwr = GetProcAddress(u, "GetWindowRect");
    fprintf(f, "user32=%p GetWindowRect=%p\n", (void*)u, (void*)gwr);
    HWND h = FindWindow("GdTest", 0);
    if (h) {
        RECT r; GetWindowRect(h, &r);
        fprintf(f, "GdTest rect: %d %d %d %d\n", r.left, r.top, r.right, r.bottom);
    }
    /* also call via the raw pointer like the stub does */
    if (gwr) {
        RECT r2; ((void(*)(HWND,LPRECT))gwr)(h, &r2);
        fprintf(f, "via ptr: %d %d %d %d\n", r2.left, r2.top, r2.right, r2.bottom);
    }
    fclose(f);
    return 0;
}
