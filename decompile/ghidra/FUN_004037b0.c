/* FUN_004037b0 @ 0x004037b0 size=331 */

void __fastcall FUN_004037b0(undefined4 *param_1)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  uint uVar4;
  
  uVar4 = param_1[7];
  if (param_1[6] != 10) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8a9);
  }
  if ((int)uVar4 < 0x3c) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8aa);
  }
  if (0x3f < (int)uVar4) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8ab);
  }
  switch(uVar4) {
  case 0x3c:
    if (*(short *)((int)param_1 + 0x46) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8af);
    }
    uVar3 = FUN_004020b0(100);
    if ((short)uVar3 == 0) {
      uVar3 = FUN_004020b0(2);
      *(short *)((int)param_1 + 0x46) = (short)uVar3 * 2 + -1;
      puVar2 = (undefined4 *)FUN_00402be0(param_1);
      FUN_00402120(puVar2,0x3d);
      return;
    }
    goto switchD_00403802_default;
  case 0x3d:
    if (*(short *)((int)param_1 + 0x46) == 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8b8);
    }
    uVar3 = FUN_004020b0(10);
    if ((short)uVar3 == 0) {
      *(undefined2 *)((int)param_1 + 0x46) = 0;
      puVar2 = (undefined4 *)FUN_00402be0(param_1);
      FUN_00402120(puVar2,0x3c);
      return;
    }
    sVar1 = *(short *)((int)param_1 + 0x46);
    puVar2 = (undefined4 *)FUN_00402be0(param_1);
    FUN_00402120(puVar2,(-1 < sVar1) + 0x3e);
    return;
  case 0x3e:
    if (-1 < *(short *)((int)param_1 + 0x46)) {
      uVar3 = 0x8c3;
LAB_004038da:
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,uVar3);
    }
    break;
  case 0x3f:
    if (*(short *)((int)param_1 + 0x46) < 1) {
      uVar3 = 0x8c8;
      goto LAB_004038da;
    }
    break;
  default:
    goto switchD_00403802_default;
  }
  uVar4 = 0x3d;
switchD_00403802_default:
  puVar2 = (undefined4 *)FUN_00402be0(param_1);
  FUN_00402120(puVar2,uVar4);
  return;
}

