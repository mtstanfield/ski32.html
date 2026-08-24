/* FUN_00403610 @ 0x00403610 size=289 */

void __fastcall FUN_00403610(undefined4 *param_1)

{
  short sVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  short sVar4;
  uint uVar5;
  short sVar6;
  
  uVar5 = param_1[7];
  if (param_1[6] != 2) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x872);
  }
  switch(uVar5) {
  case 0x1b:
    uVar2 = FUN_004020b0(3);
    *(short *)(param_1 + 0x12) = (short)uVar2 + -1;
    puVar3 = (undefined4 *)FUN_00402be0(param_1);
    FUN_00402120(puVar3,0x1c);
    return;
  case 0x1c:
    *(undefined2 *)((int)param_1 + 0x46) = 4;
    puVar3 = (undefined4 *)FUN_00402be0(param_1);
    FUN_00402120(puVar3,0x1b);
    return;
  case 0x1d:
    *(undefined2 *)(param_1 + 0x12) = 0;
    *(undefined2 *)((int)param_1 + 0x46) = 0;
    uVar2 = FUN_004020b0(0x20);
    puVar3 = (undefined4 *)FUN_00402be0(param_1);
    FUN_00402120(puVar3,(-(uint)((short)uVar2 != 0) & 3) + 0x1b);
    return;
  case 0x1e:
    uVar2 = FUN_004020b0(100);
    if ((short)uVar2 != 0) {
      puVar3 = (undefined4 *)FUN_00402be0(param_1);
      FUN_00402120(puVar3,0x1d);
      return;
    }
    sVar6 = *(short *)(param_1 + 0x11);
    sVar1 = *(short *)(param_1 + 0x10);
    sVar4 = *(short *)((int)param_1 + 0x42) + -2;
    puVar3 = FUN_004026a0(0x11,0x52);
    FUN_00402390(puVar3,sVar1 + -4,sVar4,sVar6);
    uVar5 = 0x1b;
    FUN_00402ba0((undefined4 *)&DAT_0040c608);
  }
  puVar3 = (undefined4 *)FUN_00402be0(param_1);
  FUN_00402120(puVar3,uVar5);
  return;
}

