/* FUN_00402850 @ 0x00402850 size=118 */

uint __fastcall FUN_00402850(int param_1)

{
  short sVar1;
  undefined2 uVar5;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  
  uVar5 = (undefined2)((uint)(param_1 + -0xb) >> 0x10);
  switch(param_1 + -0xb) {
  case 0:
    return CONCAT22(uVar5,0x1b);
  default:
    uVar4 = FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x623);
    return uVar4 & 0xffff0000;
  case 2:
    break;
  case 3:
    uVar3 = FUN_004020b0(4);
    return 0x2e - ((short)uVar3 != 0);
  case 4:
    uVar3 = FUN_004020b0(3);
    return 0x30 - ((short)uVar3 != 0);
  case 5:
    return CONCAT22(uVar5,0x34);
  }
  uVar3 = FUN_004020b0(8);
  sVar1 = (short)uVar3;
  if (sVar1 == 0) {
    return CONCAT22(sVar1 >> 0xf,0x32);
  }
  iVar2 = sVar1 + -1;
  if (iVar2 != 0) {
    return CONCAT22((short)((uint)iVar2 >> 0x10),0x31);
  }
  return 0x33;
}

