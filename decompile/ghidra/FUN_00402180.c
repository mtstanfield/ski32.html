/* FUN_00402180 @ 0x00402180 size=152 */

undefined4 * __fastcall FUN_00402180(undefined4 *param_1,ushort param_2)

{
  int iVar1;
  uint uVar2;
  
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3d3);
  }
  if (param_2 != *(ushort *)(param_1 + 4)) {
    DAT_0040c6fc = DAT_0040c6fc - *(short *)(param_1[5] + 0xe);
    if ((*(byte *)(param_1 + 0x13) & 1) != 0) {
      param_1 = FUN_00402220(param_1);
    }
    *(ushort *)(param_1 + 4) = param_2;
    iVar1 = DAT_0040c5f8 + (uint)param_2 * 0x10;
    param_1[5] = iVar1;
    DAT_0040c6fc = DAT_0040c6fc + *(short *)(iVar1 + 0xe);
    param_1[0x13] = param_1[0x13] & 0xfffffffb | 0x20;
    uVar2 = FUN_00402310(param_2);
    param_1[0x13] = (uVar2 & 1) << 6 | param_1[0x13] & 0xffffffbf;
  }
  return param_1;
}

