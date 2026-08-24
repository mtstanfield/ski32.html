/* FUN_00403130 @ 0x00403130 size=66 */

void __fastcall FUN_00403130(undefined4 *param_1,ushort param_2)

{
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x6ed);
  }
  *(ushort *)(param_1 + 2) = param_2;
  param_1[1] = (uint)param_2 * 0x10 + DAT_0040c5f8;
  if ((undefined4 *)*param_1 != (undefined4 *)0x0) {
    FUN_00402180((undefined4 *)*param_1,param_2);
  }
  return;
}

