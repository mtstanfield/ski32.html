/* FUN_00405760 @ 0x00405760 size=89 */

void FUN_00405760(void)

{
  char *pcVar1;
  
  DAT_0040c650 = DAT_0040c6d0;
  if (DAT_0040c6d0 != 0) {
    FUN_004057c0();
    pcVar1 = FUN_00401cf0(2);
    SetWindowTextA(DAT_0040c6c8,pcVar1);
    InvalidateRect(DAT_0040c6c8,(RECT *)0x0,0);
    return;
  }
  pcVar1 = FUN_00401cf0(1);
  SetWindowTextA(DAT_0040c6c8,pcVar1);
  FUN_00404ad0();
  return;
}

