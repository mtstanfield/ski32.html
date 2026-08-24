/* FUN_00403910 @ 0x00403910 size=231 */

void __fastcall FUN_00403910(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  uint uVar4;
  
  uVar4 = param_1[7];
  if (param_1[6] != 3) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8e2);
  }
  iVar1 = FUN_00402be0(param_1);
  if (7 < uVar4 - 0x1f) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8e5);
  }
  puVar2 = (undefined4 *)FUN_00403430(iVar1,(short *)(&DAT_0040a4e0 + (uVar4 - 0x1f) * 0x10));
  if (uVar4 == 0x1f) {
    uVar3 = FUN_004020b0(10);
    if ((short)uVar3 == 0) {
      uVar4 = 0x20;
    }
  }
  else if (uVar4 == 0x20) {
    uVar3 = FUN_004020b0(10);
    if ((short)uVar3 == 0) {
      FUN_00402120(puVar2,0x1f);
      return;
    }
  }
  else if (uVar4 == 0x21) {
    if (*(short *)(puVar2 + 0x11) == 0) {
      FUN_00402120(puVar2,0x20);
      return;
    }
  }
  else {
    if (((int)uVar4 < 0x22) || (0x26 < (int)uVar4)) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x8fa);
    }
    uVar4 = uVar4 + 1;
    if (uVar4 == 0x27) {
      FUN_00402120(puVar2,0x20);
      return;
    }
  }
  FUN_00402120(puVar2,uVar4);
  return;
}

