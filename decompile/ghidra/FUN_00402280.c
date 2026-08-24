/* FUN_00402280 @ 0x00402280 size=130 */

undefined4 * __fastcall FUN_00402280(undefined4 *param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar1 = DAT_0040c744;
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x348);
  }
  if (puVar1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x359);
    return (undefined4 *)0x0;
  }
  DAT_0040c744 = (undefined4 *)*puVar1;
  puVar3 = param_1;
  puVar4 = puVar1;
  for (iVar2 = 0x14; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  }
  puVar1[3] = 0;
  if (param_2 != 0) {
    *puVar1 = *param_1;
    *param_1 = puVar1;
    return puVar1;
  }
  *puVar1 = DAT_0040c618;
  DAT_0040c618 = puVar1;
  return puVar1;
}

