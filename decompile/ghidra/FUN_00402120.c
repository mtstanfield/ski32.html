/* FUN_00402120 @ 0x00402120 size=96 */

undefined4 * __fastcall FUN_00402120(undefined4 *param_1,uint param_2)

{
  undefined4 *puVar1;
  
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x43c);
  }
  if (0x3f < (int)param_2) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x43d);
  }
  if (param_1[7] != param_2) {
    if (0x3f < param_2) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x440);
    }
    puVar1 = FUN_00402180(param_1,*(ushort *)(&DAT_0040a1ac + param_2 * 2));
    puVar1[7] = param_2;
    return puVar1;
  }
  return param_1;
}

