/* FUN_00404290 @ 0x00404290 size=177 */

void __fastcall FUN_00404290(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  short sVar4;
  short sVar5;
  
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa49);
  }
  if (param_1[3] != 4) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa4a);
  }
  if (*(short *)((int)param_1 + 0x16) < -0x3ff) {
    param_1[4] = 0x29;
    *(undefined2 *)(param_1 + 7) = 2;
    *(undefined2 *)(param_1 + 5) = 0xff70;
    return;
  }
  if (0x5bff < *(short *)((int)param_1 + 0x16)) {
    param_1[4] = 0x27;
    *(undefined2 *)(param_1 + 7) = 0xfffe;
    *(undefined2 *)(param_1 + 5) = 0xff90;
    return;
  }
  if ((*param_1 != 0) && (param_1[4] == 0x27)) {
    uVar2 = FUN_004020b0(1000);
    if ((short)uVar2 == 0) {
      sVar5 = (short)param_1[6];
      sVar4 = *(short *)((int)param_1 + 0x16);
      iVar1 = param_1[5];
      puVar3 = FUN_004020d0(3,0x21);
      FUN_00402390(puVar3,(short)iVar1,sVar4,sVar5);
      param_1[4] = 0x28;
    }
  }
  return;
}

