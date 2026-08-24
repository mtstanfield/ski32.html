/* FUN_00401cf0 @ 0x00401cf0 size=123 */

char * __fastcall FUN_00401cf0(UINT param_1)

{
  LPSTR lpString1;
  int iVar1;
  HLOCAL pvVar2;
  CHAR local_100 [256];
  
  if (*(int *)(DAT_0040c674 + param_1 * 4) == 0) {
    iVar1 = LoadStringA(DAT_0040c61c,param_1,local_100,0xff);
    local_100[iVar1] = '\0';
    pvVar2 = LocalAlloc(0,iVar1 + 1);
    *(HLOCAL *)(DAT_0040c674 + param_1 * 4) = pvVar2;
    lpString1 = *(LPSTR *)(DAT_0040c674 + param_1 * 4);
    if (lpString1 == (LPSTR)0x0) {
      return s__out_o__memory__0040c0c8;
    }
    lstrcpyA(lpString1,local_100);
  }
  return *(char **)(DAT_0040c674 + param_1 * 4);
}

