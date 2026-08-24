/* FUN_00403540 @ 0x00403540 size=201 */

undefined4 * __fastcall FUN_00403540(undefined4 *param_1)

{
  short sVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  uint uVar5;
  
  uVar5 = param_1[7];
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x852);
  }
  if (param_1[6] != 1) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x853);
  }
  if (0x18 < (int)uVar5) {
    return param_1;
  }
  iVar2 = FUN_00402be0(param_1);
  if (4 < uVar5 - 0x16) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x857);
  }
  puVar3 = (undefined4 *)FUN_00403430(iVar2,(short *)(&DAT_0040a490 + (uVar5 - 0x16) * 0x10));
  uVar4 = FUN_004020b0(0xc);
  if ((short)uVar4 == 0) {
    uVar4 = FUN_004020b0(3);
    sVar1 = (short)uVar4;
    if (sVar1 == 0) {
      uVar5 = 0x16;
    }
    else {
      if (sVar1 == 1) {
        puVar3 = FUN_00402120(puVar3,0x17);
        return puVar3;
      }
      if (sVar1 == 2) {
        puVar3 = FUN_00402120(puVar3,0x18);
        return puVar3;
      }
    }
  }
  puVar3 = FUN_00402120(puVar3,uVar5);
  return puVar3;
}

