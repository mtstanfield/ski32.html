/* FUN_00408ede @ 0x00408ede size=137 */

int __cdecl FUN_00408ede(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  HMODULE hModule;
  int iVar1;
  
  iVar1 = 0;
  if (DAT_0040c908 == (FARPROC)0x0) {
    hModule = LoadLibraryA("user32.dll");
    if (hModule != (HMODULE)0x0) {
      DAT_0040c908 = GetProcAddress(hModule,"MessageBoxA");
      if (DAT_0040c908 != (FARPROC)0x0) {
        DAT_0040c90c = GetProcAddress(hModule,"GetActiveWindow");
        DAT_0040c910 = GetProcAddress(hModule,"GetLastActivePopup");
        goto LAB_00408f2d;
      }
    }
    iVar1 = 0;
  }
  else {
LAB_00408f2d:
    if (DAT_0040c90c != (FARPROC)0x0) {
      iVar1 = (*DAT_0040c90c)();
      if ((iVar1 != 0) && (DAT_0040c910 != (FARPROC)0x0)) {
        iVar1 = (*DAT_0040c910)(iVar1);
      }
    }
    iVar1 = (*DAT_0040c908)(iVar1,param_1,param_2,param_3);
  }
  return iVar1;
}

