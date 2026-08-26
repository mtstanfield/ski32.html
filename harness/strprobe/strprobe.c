/* Probe: load original ski32.exe as a module and dump LoadStringA(1..20).
 * Ground truth for the id->string mapping in the original's (quirky) resource section.
 */
#include <windows.h>
#include <stdio.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    (void)hInst; (void)hPrev; (void)cmd; (void)show;
    char path[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA(NULL), path, MAX_PATH);
    /* the probe is launched from a dir containing original\; try fixed names */
    const char *cands[] = {
        "original/ski32.exe",
        "C:\\original\\ski32.exe",
    };
    HMODULE h = NULL;
    for (int i = 0; i < 2 && !h; i++) h = LoadLibraryA(cands[i]);
    if (!h) { printf("LoadLibrary failed: %lu\n", GetLastError()); return 1; }
    for (UINT id = 0; id <= 20; id++) {
        char buf[512];
        int n = LoadStringA(h, id, buf, sizeof buf);
        if (n <= 0) printf("id=%2u: <fail %lu>\n", id, GetLastError());
        else printf("id=%2u: %d chars %s\n", id, n, buf);
    }
    return 0;
}
