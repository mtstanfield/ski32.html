/* FUN_00401270 @ 0x00401270 size=30 */

void __fastcall FUN_00401270(LPCSTR param_1,LPCSTR param_2)

{
  int iVar1;
  
  iVar1 = MessageBoxA((HWND)0x0,param_2,param_1,0x31);
  if (iVar1 == 2) {
    DestroyWindow(DAT_0040c6c8);
  }
  return;
}

