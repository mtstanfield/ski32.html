/* FUN_004057c0 @ 0x004057c0 size=52 */

void FUN_004057c0(void)

{
  if ((DAT_0040c6c8 != (HWND)0x0) && (DAT_0040c6d0 != 0)) {
    DAT_0040c6d0 = 0;
    KillTimer(DAT_0040c6c8,0x29a);
    DAT_0040c600 = DAT_0040c698;
  }
  return;
}

