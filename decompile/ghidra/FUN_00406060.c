/* FUN_00406060 @ 0x00406060 size=73 */

void __fastcall FUN_00406060(undefined2 param_1,undefined2 param_2)

{
  uint uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  for (puVar2 = DAT_0040c618; puVar2 != (undefined4 *)0x0; puVar2 = (undefined4 *)*puVar2) {
    uVar1 = puVar2[0x13];
    if (((uVar1 & 4) != 0) && ((uVar1 & 2) == 0)) {
      puVar3 = puVar2;
      if ((uVar1 & 1) != 0) {
        puVar3 = FUN_00402220(puVar2);
      }
      puVar3[0x13] = puVar3[0x13] & 0xfffffffb;
    }
  }
  DAT_0040c5fc._0_2_ = param_2;
  DAT_0040c704._0_2_ = param_1;
  return;
}

