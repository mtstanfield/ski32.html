/* FUN_004012f0 @ 0x004012f0 size=95 */

undefined4 __fastcall FUN_004012f0(int *param_1,int *param_2)

{
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x17d);
  }
  if (param_2 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x17e);
  }
  if ((((param_1[1] == param_2[1]) && (*param_1 == *param_2)) && (param_1[2] == param_2[2])) &&
     (param_1[3] == param_2[3])) {
    return 1;
  }
  return 0;
}

