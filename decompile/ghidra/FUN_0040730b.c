/* FUN_0040730b @ 0x0040730b size=153 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0040730b(void)

{
  undefined4 *puVar1;
  byte *pbVar2;
  int local_c;
  int local_8;
  
  if (DAT_0040ccc8 == 0) {
    FUN_0040811f();
  }
  GetModuleFileNameA((HMODULE)0x0,&DAT_0040c7f4,0x104);
  _DAT_0040c7dc = &DAT_0040c7f4;
  pbVar2 = &DAT_0040c7f4;
  if (*DAT_0040ccd8 != 0) {
    pbVar2 = DAT_0040ccd8;
  }
  FUN_004073a4(pbVar2,(undefined4 *)0x0,(byte *)0x0,&local_8,&local_c);
  puVar1 = _malloc(local_c + local_8 * 4);
  if (puVar1 == (undefined4 *)0x0) {
    FUN_00406e79(8);
  }
  FUN_004073a4(pbVar2,puVar1,(byte *)(puVar1 + local_8),&local_8,&local_c);
  _DAT_0040c7c4 = puVar1;
  _DAT_0040c7c0 = local_8 + -1;
  return;
}

