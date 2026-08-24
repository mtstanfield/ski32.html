/* FUN_004041c0 @ 0x004041c0 size=194 */

/* WARNING: Removing unreachable block (ram,0x00404227) */

void __fastcall FUN_004041c0(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xae7);
  }
  *(short *)(param_1 + 5) = (short)param_1[5] + *(short *)((int)param_1 + 0x1a);
  iVar1 = param_1[3];
  *(short *)((int)param_1 + 0x16) = *(short *)((int)param_1 + 0x16) + (short)param_1[7];
  *(short *)(param_1 + 6) = (short)param_1[6] + *(short *)((int)param_1 + 0x1e);
  if (iVar1 == 4) {
    FUN_00404290(param_1);
  }
  else if ((iVar1 < 5) || (8 < iVar1)) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xaf9);
  }
  else {
    FUN_00404350((int)param_1);
  }
  puVar2 = (undefined4 *)*param_1;
  if (puVar2 != (undefined4 *)0x0) {
    if (puVar2[3] == 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xaff);
    }
    if ((int *)puVar2[3] != param_1) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xb00);
    }
    puVar2 = FUN_00402390(puVar2,(short)param_1[5],*(short *)((int)param_1 + 0x16),(short)param_1[6]
                         );
    FUN_00402120(puVar2,param_1[4]);
  }
  return;
}

