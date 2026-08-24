/* FUN_00401a60 @ 0x00401a60 size=180 */

void __fastcall FUN_00401a60(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  if (param_1 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4e4);
  }
  if (param_2 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4e5);
  }
  if ((*(byte *)(param_1 + 0x4c) & 0x10) == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4e6);
  }
  if ((*(byte *)(param_2 + 0x4c) & 0x10) == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4e7);
  }
  if (param_1 == param_2) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4e8);
  }
  iVar1 = *(int *)(param_1 + 8);
  iVar2 = param_1;
  while (iVar1 != 0) {
    iVar2 = *(int *)(iVar2 + 8);
    if ((*(byte *)(iVar2 + 0x4c) & 0x10) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4ec);
    }
    iVar1 = *(int *)(iVar2 + 8);
  }
  *(int *)(iVar2 + 8) = param_2;
  FUN_00401b20((int *)(param_1 + 0x30),(int *)(param_2 + 0x30));
  *(uint *)(param_2 + 0x4c) = *(uint *)(param_2 + 0x4c) & 0xffffffef;
  return;
}

