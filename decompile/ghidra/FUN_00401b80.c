/* FUN_00401b80 @ 0x00401b80 size=354 */

void __fastcall FUN_00401b80(HDC param_1)

{
  short sVar1;
  uint uVar2;
  char *pcVar3;
  int iVar4;
  short sVar5;
  short sVar6;
  undefined4 uVar7;
  short local_18 [2];
  CHAR local_14 [20];
  
  sVar6 = DAT_0040c66c._2_2_ + 2;
  sVar5 = 0;
  sVar1 = 0;
  local_18[0] = 2;
  local_18[1] = 0;
  if (DAT_0040c72c != 0) {
    if (DAT_0040c5f4 == 0) {
      sVar5 = 0;
    }
    else {
      sVar5 = (short)((*(short *)(DAT_0040c72c + 0x48) * 1000) / (DAT_0040c5f4 << 4));
    }
    sVar1 = *(short *)(DAT_0040c72c + 0x42);
    if (DAT_0040c95c == 0) {
      if (DAT_0040c954 == 0) {
        if (DAT_0040c958 != 0) {
          sVar1 = 0x4100 - sVar1;
        }
      }
      else {
        sVar1 = 0x4100 - sVar1;
      }
    }
    else {
      sVar1 = 0x21c0 - sVar1;
    }
  }
  uVar2 = FUN_00401d70(DAT_0040c944,local_14);
  FUN_00401e20(param_1,local_14,sVar6,local_18,uVar2 & 0xffff);
  iVar4 = (int)(short)((int)((int)sVar1 + ((int)sVar1 >> 0x1f & 0xfU)) >> 4);
  pcVar3 = FUN_00401cf0(0xc);
  iVar4 = wsprintfA(local_14,pcVar3,iVar4);
  FUN_00401e20(param_1,local_14,sVar6,local_18,iVar4);
  iVar4 = (int)sVar5;
  pcVar3 = FUN_00401cf0(0xd);
  iVar4 = wsprintfA(local_14,pcVar3,iVar4);
  FUN_00401e20(param_1,local_14,sVar6,local_18,iVar4);
  uVar7 = DAT_0040c6a8;
  pcVar3 = FUN_00401cf0(0xe);
  iVar4 = wsprintfA(local_14,pcVar3,uVar7);
  FUN_00401e20(param_1,local_14,sVar6,local_18,iVar4);
  DAT_0040c5dc = DAT_0040c698;
  return;
}

