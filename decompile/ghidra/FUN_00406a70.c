/* FUN_00406a70 @ 0x00406a70 size=480 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00406a70(HWND param_1)

{
  char *pcVar1;
  int iVar2;
  short local_40 [4];
  tagTEXTMETRICA local_38;
  
  local_40[0] = 0;
  local_40[1] = 0;
  local_40[2] = 0;
  local_40[3] = 0;
  DAT_0040c6cc = GetDC(param_1);
  if (DAT_0040c6cc == (HDC)0x0) {
    return 0;
  }
  DAT_0040c664 = GetStockObject(10);
  if (DAT_0040c664 != (HGDIOBJ)0x0) {
    DAT_0040c664 = SelectObject(DAT_0040c6cc,DAT_0040c664);
  }
  GetTextMetricsA(DAT_0040c6cc,&local_38);
  _DAT_0040c668 = CONCAT22(_DAT_0040c66a,(undefined2)local_38.tmHeight);
  pcVar1 = FUN_00401cf0(3);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(3);
  FUN_00406c50(DAT_0040c6cc,local_40,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(4);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(4);
  FUN_00406c50(DAT_0040c6cc,local_40,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(5);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(5);
  FUN_00406c50(DAT_0040c6cc,local_40,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(6);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(6);
  FUN_00406c50(DAT_0040c6cc,local_40,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(7);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(7);
  FUN_00406c50(DAT_0040c6cc,local_40 + 2,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(8);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(8);
  FUN_00406c50(DAT_0040c6cc,local_40 + 2,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(9);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(9);
  FUN_00406c50(DAT_0040c6cc,local_40 + 2,pcVar1,iVar2);
  pcVar1 = FUN_00401cf0(10);
  iVar2 = lstrlenA(pcVar1);
  pcVar1 = FUN_00401cf0(10);
  FUN_00406c50(DAT_0040c6cc,local_40 + 2,pcVar1,iVar2);
  DAT_0040c66c._2_2_ = (short)local_40._0_4_;
  _DAT_0040c668 = CONCAT22((short)_DAT_0040c668 * 4,(short)_DAT_0040c668);
  DAT_0040c66c._0_2_ = (short)local_40._4_4_ + (short)local_40._0_4_;
  return 1;
}

