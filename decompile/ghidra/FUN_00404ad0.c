/* FUN_00404ad0 @ 0x00404ad0 size=128 */

void FUN_00404ad0(void)

{
  if (((DAT_0040c6c8 != (HWND)0x0) && (DAT_0040c6d0 == 0)) && (DAT_0040c650 == 0)) {
    DAT_0040c6d0 = 1;
    DAT_0040c698 = GetTickCount();
    if ((DAT_0040c95c != 0) || (DAT_0040c958 != 0)) {
      DAT_0040c948 = DAT_0040c948 + (DAT_0040c698 - DAT_0040c600);
    }
    SetTimer(DAT_0040c6c8,0x29a,DAT_0040c678 & 0xffff,DAT_0040c940);
  }
  return;
}

