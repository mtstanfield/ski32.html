/* FUN_00402390 @ 0x00402390 size=216 */

undefined4 * __fastcall FUN_00402390(undefined4 *param_1,short param_2,short param_3,short param_4)

{
  short sVar1;
  uint uVar2;
  bool bVar3;
  byte bVar4;
  bool bVar5;
  
  if ((*(short *)(param_1 + 0x10) == param_2) && (*(short *)((int)param_1 + 0x42) == param_3)) {
    bVar3 = false;
  }
  else {
    bVar3 = true;
  }
  sVar1 = *(short *)(param_1 + 0x11);
  bVar5 = param_1 != DAT_0040c64c;
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x40d);
  }
  if (bVar5) {
LAB_004023f8:
    if (bVar3) goto LAB_00402404;
  }
  else if (bVar3) {
    FUN_00402470(param_2,param_3);
    goto LAB_004023f8;
  }
  if (sVar1 == param_4) {
    return param_1;
  }
LAB_00402404:
  uVar2 = param_1[0x13];
  if ((uVar2 & 1) != 0) {
    param_1 = FUN_00402220(param_1);
  }
  if (((-1 < (int)(uVar2 << 0x1d)) || (bVar5)) || (sVar1 != param_4)) {
    bVar4 = 0;
  }
  else {
    bVar4 = 1;
  }
  *(short *)((int)param_1 + 0x42) = param_3;
  *(short *)(param_1 + 0x10) = param_2;
  *(short *)(param_1 + 0x11) = param_4;
  param_1[0x13] = (uint)(bVar4 | 8) << 2 | param_1[0x13] & 0xfffffffb;
  return param_1;
}

