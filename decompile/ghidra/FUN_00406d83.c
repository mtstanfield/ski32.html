/* entry @ 0x00406d83 size=235 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void entry(void)

{
  DWORD DVar1;
  int iVar2;
  byte *pbVar3;
  uint uVar4;
  HMODULE pHVar5;
  UINT UVar6;
  _STARTUPINFOA local_60;
  undefined1 *local_1c;
  _EXCEPTION_POINTERS *local_18;
  void *pvStack_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_0040a560;
  puStack_10 = &LAB_0040796c;
  pvStack_14 = ExceptionList;
  local_1c = &stack0xffffff88;
  ExceptionList = &pvStack_14;
  DVar1 = GetVersion();
  _DAT_0040c7bc = DVar1 >> 8 & 0xff;
  _DAT_0040c7b8 = DVar1 & 0xff;
  _DAT_0040c7b4 = _DAT_0040c7b8 * 0x100 + _DAT_0040c7bc;
  _DAT_0040c7b0 = DVar1 >> 0x10;
  iVar2 = FUN_00407835(0);
  if (iVar2 == 0) {
    FUN_00406e9e(0x1c);
  }
  local_8 = 0;
  FUN_0040768a();
  DAT_0040ccd8 = GetCommandLineA();
  DAT_0040c798 = FUN_00407558();
  FUN_0040730b();
  FUN_00407252();
  FUN_00406f74();
  local_60.dwFlags = 0;
  GetStartupInfoA(&local_60);
  pbVar3 = FUN_004071fa();
  if ((local_60.dwFlags & 1) == 0) {
    uVar4 = 10;
  }
  else {
    uVar4 = (uint)local_60.wShowWindow;
  }
  iVar2 = 0;
  pHVar5 = GetModuleHandleA((LPCSTR)0x0);
  UVar6 = FUN_004047e0(pHVar5,iVar2,(LPCSTR)pbVar3,uVar4);
  FUN_00406fa1(UVar6);
  FUN_00407076(local_18->ExceptionRecord->ExceptionCode,local_18);
  return;
}

