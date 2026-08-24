/* FUN_00404950 @ 0x00404950 size=27 */

void __fastcall FUN_00404950(LPCSTR param_1)

{
  char *lpCaption;
  UINT uType;
  
  uType = 0x30;
  lpCaption = FUN_00401cf0(1);
  MessageBoxA((HWND)0x0,param_1,lpCaption,uType);
  return;
}

