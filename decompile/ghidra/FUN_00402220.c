/* FUN_00402220 @ 0x00402220 size=86 */

undefined4 * __fastcall FUN_00402220(undefined4 *param_1)

{
  undefined4 *puVar1;
  
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3b3);
  }
  if ((*(byte *)(param_1 + 0x13) & 1) == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3b5);
  }
  puVar1 = FUN_00402280(param_1,1);
  param_1[1] = puVar1;
  if (puVar1 != (undefined4 *)0x0) {
    puVar1[1] = param_1;
    puVar1[0x13] = puVar1[0x13] | 2;
    param_1[0x13] = param_1[0x13] & 0xfffffffe;
  }
  return param_1;
}

