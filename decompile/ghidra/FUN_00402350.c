/* FUN_00402350 @ 0x00402350 size=58 */

undefined4 * __fastcall FUN_00402350(undefined4 *param_1,int param_2)

{
  undefined4 *puVar1;
  short local_8 [2];
  short local_4 [2];
  
  if (param_1 != (undefined4 *)0x0) {
    FUN_004024f0(param_2,local_4,local_8);
    puVar1 = FUN_00402390(param_1,local_4[0],local_8[0],0);
    return puVar1;
  }
  return (undefined4 *)0x0;
}

