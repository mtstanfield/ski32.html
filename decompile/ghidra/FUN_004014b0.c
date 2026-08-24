/* FUN_004014b0 @ 0x004014b0 size=143 */

void __fastcall FUN_004014b0(int *param_1,int param_2,short param_3,short param_4,short param_5)

{
  short sVar1;
  short sVar2;
  short sVar3;
  int iVar4;
  
  sVar1 = *(short *)(param_2 + 10);
  sVar2 = *(short *)(param_2 + 0xc);
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x38b);
  }
  if (param_2 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x38c);
  }
  sVar3 = (short)DAT_0040c5fc - DAT_0040c5f0._2_2_;
  iVar4 = (int)(short)(param_3 + (((short)DAT_0040c704 - sVar1 / 2) - (short)DAT_0040c640));
  *param_1 = iVar4;
  param_1[2] = iVar4 + sVar1;
  iVar4 = (int)(short)((param_4 + (sVar3 - param_5)) - sVar2);
  param_1[1] = iVar4;
  param_1[3] = sVar2 + iVar4;
  return;
}

