/* FUN_004046e0 @ 0x004046e0 size=222 */

void __fastcall FUN_004046e0(uint *param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  int *piVar4;
  int *piVar5;
  
  sVar2 = (short)DAT_0040c5fc;
  sVar1 = (short)DAT_0040c68c;
  sVar3 = ((short)DAT_0040c684 - sVar2) + -0x3c;
  piVar4 = (int *)param_1[2];
  if (param_1 == (uint *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb21);
  }
  if (piVar4 < (int *)*param_1) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb22);
  }
  if ((int *)param_1[1] < piVar4) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb23);
  }
  piVar5 = (int *)param_1[1];
  if (piVar4 < piVar5) {
    do {
      if ((int)sVar3 <= (int)*(short *)((int)piVar4 + 0x16) - (int)DAT_0040c5f0._2_2_) break;
      piVar4 = piVar4 + 9;
    } while (piVar4 < piVar5);
  }
  if ((int *)*param_1 < piVar4) {
    do {
      if ((int)*(short *)((int)piVar4 + 0x16) - (int)DAT_0040c5f0._2_2_ < (int)sVar3) break;
      piVar4 = piVar4 + -9;
    } while ((int *)*param_1 < piVar4);
  }
  param_1[2] = (uint)piVar4;
  if (piVar4 < piVar5) {
    do {
      if ((int)(short)((sVar1 - sVar2) + 0x3c) <=
          (int)*(short *)((int)piVar4 + 0x16) - (int)DAT_0040c5f0._2_2_) {
        return;
      }
      piVar5 = piVar4 + 9;
      FUN_00404130(piVar4);
      piVar4 = piVar5;
    } while (piVar5 < (int *)param_1[1]);
  }
  return;
}

