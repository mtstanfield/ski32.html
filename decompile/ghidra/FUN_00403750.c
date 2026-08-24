/* FUN_00403750 @ 0x00403750 size=92 */

void __fastcall FUN_00403750(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = param_1[7];
  if (param_1[6] != 9) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x89c);
  }
  if (iVar1 < 0x38) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x89d);
  }
  if (0x3b < iVar1) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x89e);
  }
  uVar2 = iVar1 + 1;
  if (0x3b < (int)uVar2) {
    uVar2 = 0x38;
  }
  FUN_00402120(param_1,uVar2);
  return;
}

