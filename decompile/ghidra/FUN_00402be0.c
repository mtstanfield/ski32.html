/* FUN_00402be0 @ 0x00402be0 size=125 */

void __fastcall FUN_00402be0(undefined4 *param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  
  sVar1 = *(short *)(param_1 + 0x10) + *(short *)((int)param_1 + 0x46);
  sVar2 = *(short *)((int)param_1 + 0x42) + *(short *)(param_1 + 0x12);
  sVar3 = *(short *)(param_1 + 0x11) + *(short *)((int)param_1 + 0x4a);
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x425);
  }
  if (DAT_0040c670 != 0) {
    sVar1 = sVar1 + *(short *)((int)param_1 + 0x46);
    sVar2 = sVar2 + *(short *)(param_1 + 0x12);
    sVar3 = sVar3 + *(short *)((int)param_1 + 0x4a);
  }
  if (0 < sVar3) {
    *(short *)((int)param_1 + 0x4a) = *(short *)((int)param_1 + 0x4a) + -1;
    FUN_00402390(param_1,sVar1,sVar2,sVar3);
    return;
  }
  *(undefined2 *)((int)param_1 + 0x4a) = 0;
  FUN_00402390(param_1,sVar1,sVar2,0);
  return;
}

