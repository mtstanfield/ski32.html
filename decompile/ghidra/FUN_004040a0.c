/* FUN_004040a0 @ 0x004040a0 size=129 */

void __fastcall FUN_004040a0(uint *param_1)

{
  int *piVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  int *piVar6;
  
  sVar4 = (short)DAT_0040c5fc;
  sVar3 = (short)DAT_0040c68c;
  sVar2 = (short)DAT_0040c684;
  piVar6 = (int *)*param_1;
  if (param_1 == (uint *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb0d);
  }
  piVar1 = (int *)param_1[1];
  if (piVar1 < piVar6) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb0e);
    piVar1 = (int *)param_1[1];
  }
  if (piVar6 < piVar1) {
    do {
      FUN_004041c0(piVar6);
      sVar5 = *(short *)((int)piVar6 + 0x16) - DAT_0040c5f0._2_2_;
      if (((short)((sVar2 - sVar4) + -0x3c) <= sVar5) && (sVar5 < (short)((sVar3 - sVar4) + 0x3c)))
      {
        FUN_00404130(piVar6);
      }
      piVar6 = piVar6 + 9;
    } while (piVar6 < (int *)param_1[1]);
  }
  return;
}

