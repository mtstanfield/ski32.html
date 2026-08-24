/* FUN_00406500 @ 0x00406500 size=76 */

void FUN_00406500(void)

{
  int iVar1;
  
  iVar1 = FUN_00404970();
  if (iVar1 != 0) {
    if (DAT_0040c650 != 0) {
      FUN_00405760();
    }
    InvalidateRect(DAT_0040c6c8,(RECT *)0x0,1);
    iVar1 = FUN_00404a80();
    if (iVar1 != 0) {
      UpdateWindow(DAT_0040c6c8);
      return;
    }
  }
  DestroyWindow(DAT_0040c6c8);
  return;
}

