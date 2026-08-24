/* FUN_00405640 @ 0x00405640 size=83 */

undefined4 __fastcall FUN_00405640(uint param_1,undefined4 *param_2)

{
  HRSRC hResInfo;
  HGLOBAL pvVar1;
  LPVOID pvVar2;
  
  hResInfo = FindResourceA(DAT_0040c61c,(LPCSTR)(param_1 & 0xffff),&DAT_0040c128);
  *param_2 = hResInfo;
  if (hResInfo != (HRSRC)0x0) {
    pvVar1 = LoadResource(DAT_0040c61c,hResInfo);
    *param_2 = pvVar1;
  }
  if ((HGLOBAL)*param_2 != (HGLOBAL)0x0) {
    pvVar2 = LockResource((HGLOBAL)*param_2);
    param_2[1] = pvVar2;
    return 1;
  }
  param_2[1] = 0;
  return 0;
}

