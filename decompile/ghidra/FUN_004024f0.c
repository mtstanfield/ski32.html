/* FUN_004024f0 @ 0x004024f0 size=192 */

void __fastcall FUN_004024f0(int param_1,short *param_2,short *param_3)

{
  short sVar1;
  undefined4 uVar2;
  
  *param_2 = (short)DAT_0040c640 - (short)DAT_0040c704;
  *param_3 = DAT_0040c5f0._2_2_ - (short)DAT_0040c5fc;
  switch(param_1) {
  case 0:
  case 1:
    if (param_1 == 0) {
      sVar1 = (short)DAT_0040c6b0 + -0x3c;
    }
    else {
      sVar1 = (short)DAT_0040c6b8 + 0x3c;
    }
    *param_2 = *param_2 + sVar1;
    uVar2 = FUN_004020b0((short)DAT_0040c6d8);
    *param_3 = *param_3 + (short)uVar2 + (short)DAT_0040c6b4;
    return;
  case 2:
  case 3:
    break;
  default:
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x5ae);
    return;
  }
  uVar2 = FUN_004020b0((short)DAT_0040c5f0);
  *param_2 = *param_2 + (short)uVar2 + (short)DAT_0040c6b0;
  if (param_1 != 2) {
    *param_3 = *param_3 + (short)DAT_0040c6bc + 0x3c;
    return;
  }
  *param_3 = *param_3 + (short)DAT_0040c6b4 + -0x3c;
  return;
}

