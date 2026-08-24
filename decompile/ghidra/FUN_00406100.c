/* FUN_00406100 @ 0x00406100 size=112 */

void __fastcall FUN_00406100(HDC param_1,int *param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  int iVar3;
  
  if (param_1 == (HDC)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x543);
  }
  puVar1 = DAT_0040c618;
  if (param_2 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x544);
    puVar1 = DAT_0040c618;
  }
  for (; puVar1 != (undefined4 *)0x0; puVar1 = (undefined4 *)*puVar1) {
    if ((*(byte *)(puVar1 + 0x13) & 4) == 0) {
      piVar2 = FUN_00401410((int)puVar1);
    }
    else {
      piVar2 = puVar1 + 8;
    }
    iVar3 = FUN_00401290(piVar2,param_2);
    if (iVar3 != 0) {
      puVar1[0x13] = puVar1[0x13] & 0xfffffffe;
    }
  }
  FUN_00401060(param_1,param_2);
  return;
}

