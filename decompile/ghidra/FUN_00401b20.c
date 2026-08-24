/* FUN_00401b20 @ 0x00401b20 size=96 */

void __fastcall FUN_00401b20(int *param_1,int *param_2)

{
  if (param_2 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x16d);
  }
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x16e);
  }
  if (*param_2 < *param_1) {
    *param_1 = *param_2;
  }
  if (param_1[2] < param_2[2]) {
    param_1[2] = param_2[2];
  }
  if (param_2[1] < param_1[1]) {
    param_1[1] = param_2[1];
  }
  if (param_1[3] < param_2[3]) {
    param_1[3] = param_2[3];
  }
  return;
}

