/* FUN_00405120 @ 0x00405120 size=187 */

undefined4 * __fastcall FUN_00405120(int *param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  
  uVar2 = (uint)DAT_0040c702;
  DAT_0040c702 = DAT_0040c702 + 1;
  puVar1 = (undefined4 *)(DAT_0040c758 + uVar2 * 0x24);
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa1b);
  }
  if (param_2 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa1c);
  }
  if (0x100 < DAT_0040c702) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa1d);
  }
  if (*param_1 == 0) {
    param_1[2] = (int)puVar1;
    param_1[1] = (int)puVar1;
    *param_1 = (int)puVar1;
  }
  if ((undefined4 *)param_1[1] != puVar1) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa20);
  }
  param_1[1] = param_1[1] + 0x24;
  puVar4 = puVar1;
  for (iVar3 = 9; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = *param_2;
    param_2 = param_2 + 1;
    puVar4 = puVar4 + 1;
  }
  *puVar1 = 0;
  puVar1[1] = (uint)*(ushort *)(puVar1 + 2) * 0x10 + DAT_0040c5f8;
  return puVar1;
}

