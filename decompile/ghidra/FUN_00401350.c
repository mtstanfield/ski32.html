/* FUN_00401350 @ 0x00401350 size=52 */

void __fastcall FUN_00401350(int param_1)

{
  if (param_1 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x361);
  }
  if ((*(byte *)(param_1 + 0x4c) & 1) == 0) {
    if (*(int *)(param_1 + 4) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 4) + 4) = 0;
    }
    *(uint *)(param_1 + 0x4c) = *(uint *)(param_1 + 0x4c) | 8;
  }
  return;
}

