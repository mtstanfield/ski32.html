#include "ski_game.h"
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show)
{
    (void)hInst; (void)hPrev; (void)cmd; (void)show;
    ski_init();
    ski_run();
    return 0;
}
