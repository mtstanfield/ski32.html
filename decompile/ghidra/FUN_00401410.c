/* FUN_00401410 @ 0x00401410 size=146 */

int * __fastcall FUN_00401410(int param_1)

{
  if (param_1 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3a3);
  }
  if ((*(byte *)(param_1 + 0x4c) & 4) != 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3a4);
  }
  if (*(short *)(param_1 + 0x10) == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3a5);
  }
  if ((uint)*(ushort *)(param_1 + 0x10) * 0x10 + DAT_0040c5f8 != *(int *)(param_1 + 0x14)) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x3a6);
  }
  FUN_004014b0((int *)(param_1 + 0x20),*(int *)(param_1 + 0x14),*(short *)(param_1 + 0x40),
               *(short *)(param_1 + 0x42),*(short *)(param_1 + 0x44));
  *(uint *)(param_1 + 0x4c) = *(uint *)(param_1 + 0x4c) | 4;
  return (int *)(param_1 + 0x20);
}

