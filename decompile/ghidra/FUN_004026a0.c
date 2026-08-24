/* FUN_004026a0 @ 0x004026a0 size=78 */

undefined4 * __fastcall FUN_004026a0(int param_1,ushort param_2)

{
  undefined4 *puVar1;
  
  puVar1 = (undefined4 *)FUN_00402330();
  if (puVar1 != (undefined4 *)0x0) {
    if (param_1 < 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x57b);
    }
    if (0x11 < param_1) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x57c);
    }
    puVar1[6] = param_1;
    puVar1 = FUN_00402180(puVar1,param_2);
    return puVar1;
  }
  return (undefined4 *)0x0;
}

