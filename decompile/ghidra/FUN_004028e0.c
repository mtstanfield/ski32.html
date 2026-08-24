/* FUN_004028e0 @ 0x004028e0 size=584 */

undefined4 * __fastcall FUN_004028e0(undefined4 *param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  undefined4 *puVar4;
  int iVar5;
  ushort uVar6;
  uint uVar7;
  
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x907);
  }
  if ((10 < (int)param_1[6]) || (param_1[3] != 0)) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x908);
  }
  switch(param_1[6]) {
  case 0:
    break;
  case 1:
    puVar4 = FUN_00403540(param_1);
    return puVar4;
  case 2:
    puVar4 = (undefined4 *)FUN_00403610(param_1);
    return puVar4;
  case 3:
    puVar4 = (undefined4 *)FUN_00403910(param_1);
    return puVar4;
  default:
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x91f);
    return param_1;
  case 9:
    puVar4 = (undefined4 *)FUN_00403750(param_1);
    return puVar4;
  case 10:
    puVar4 = (undefined4 *)FUN_004037b0(param_1);
    return puVar4;
  }
  sVar1 = *(short *)(param_1 + 0x10);
  sVar2 = *(short *)((int)param_1 + 0x42);
  uVar7 = param_1[7];
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7e6);
  }
  if (param_1[6] != 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7e7);
  }
  if (uVar7 == 0xb) {
    if (*(short *)(param_1 + 0x11) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7eb);
    }
    if (*(short *)((int)param_1 + 0x4a) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7ec);
    }
    sVar3 = *(short *)((int)param_1 + 0x46);
    if ((sVar3 == 0) && (*(short *)(param_1 + 0x12) == 0)) {
      uVar7 = 0xc;
    }
    if (sVar3 < 0) {
      uVar6 = 0xffff;
    }
    else {
      uVar6 = (ushort)(0 < sVar3);
    }
    *(ushort *)((int)param_1 + 0x46) = sVar3 - uVar6;
    sVar3 = *(short *)(param_1 + 0x12);
    if (sVar3 < 0) {
      *(short *)(param_1 + 0x12) = sVar3 + 1;
    }
    else {
      *(ushort *)(param_1 + 0x12) = sVar3 - (ushort)(0 < sVar3);
    }
  }
  else {
    iVar5 = FUN_00402be0(param_1);
    if (0x15 < uVar7) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7f8);
    }
    param_1 = (undefined4 *)FUN_00403430(iVar5,(short *)(&DAT_0040a308 + uVar7 * 0x10));
    switch(uVar7) {
    case 7:
    case 9:
      uVar7 = 3;
      break;
    case 8:
    case 10:
      uVar7 = 6;
      break;
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
      if (*(short *)(param_1 + 0x11) == 0) {
        if (8 < uVar7 - 0xd) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x812);
        }
        uVar7 = *(uint *)(&DAT_0040a434 + uVar7 * 4);
        if (uVar7 == 0x11) {
          FUN_00403420(-0x40);
          puVar4 = (undefined4 *)&DAT_0040c6c0;
        }
        else {
          puVar4 = (undefined4 *)&DAT_0040c718;
        }
        FUN_00402ba0(puVar4);
      }
    }
  }
  puVar4 = FUN_00402120(param_1,uVar7);
  switch(uVar7) {
  case 7:
  case 8:
  case 9:
  case 10:
    iVar5 = -1;
    break;
  default:
    goto switchD_00402b0b_caseD_b;
  case 0x10:
    iVar5 = 2;
    break;
  case 0x12:
  case 0x13:
    iVar5 = 4;
    break;
  case 0x14:
  case 0x15:
    iVar5 = 8;
  }
  FUN_00403420(iVar5);
switchD_00402b0b_caseD_b:
  FUN_00402c60((int)puVar4,sVar1,sVar2);
  FUN_00403180((int)puVar4,sVar1,sVar2);
  FUN_00403250((int)puVar4,sVar1,sVar2);
  return puVar4;
}

