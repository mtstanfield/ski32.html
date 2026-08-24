/* FUN_00406970 @ 0x00406970 size=253 */

void __fastcall FUN_00406970(HWND param_1)

{
  HBRUSH hbr;
  char *pcVar1;
  int iVar2;
  short sVar3;
  short *psVar4;
  short local_44 [2];
  tagPAINTSTRUCT local_40;
  
  local_44[0] = 2;
  local_44[1] = 0;
  BeginPaint(param_1,&local_40);
  hbr = GetStockObject(4);
  FrameRect(local_40.hdc,(RECT *)&DAT_0040c778,hbr);
  pcVar1 = FUN_00401cf0(3);
  iVar2 = lstrlenA(pcVar1);
  psVar4 = local_44;
  sVar3 = 2;
  pcVar1 = FUN_00401cf0(3);
  FUN_00401e20(local_40.hdc,pcVar1,sVar3,psVar4,iVar2);
  pcVar1 = FUN_00401cf0(4);
  iVar2 = lstrlenA(pcVar1);
  psVar4 = local_44;
  sVar3 = 2;
  pcVar1 = FUN_00401cf0(4);
  FUN_00401e20(local_40.hdc,pcVar1,sVar3,psVar4,iVar2);
  pcVar1 = FUN_00401cf0(5);
  iVar2 = lstrlenA(pcVar1);
  psVar4 = local_44;
  sVar3 = 2;
  pcVar1 = FUN_00401cf0(5);
  FUN_00401e20(local_40.hdc,pcVar1,sVar3,psVar4,iVar2);
  pcVar1 = FUN_00401cf0(6);
  iVar2 = lstrlenA(pcVar1);
  psVar4 = local_44;
  sVar3 = 2;
  pcVar1 = FUN_00401cf0(6);
  FUN_00401e20(local_40.hdc,pcVar1,sVar3,psVar4,iVar2);
  FUN_00401b80(local_40.hdc);
  EndPaint(param_1,&local_40);
  return;
}

