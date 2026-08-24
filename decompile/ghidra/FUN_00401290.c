/* FUN_00401290 @ 0x00401290 size=95 */

undefined4 __fastcall FUN_00401290(int *param_1,int *param_2)

{
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x160);
  }
  if (param_2 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x161);
  }
  if ((((*param_2 < param_1[2]) && (*param_1 < param_2[2])) && (param_2[1] < param_1[3])) &&
     (param_1[1] < param_2[3])) {
    return 1;
  }
  return 0;
}

