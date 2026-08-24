/* FUN_00401390 @ 0x00401390 size=117 */

void FUN_00401390(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar2 = &DAT_0040c618;
  puVar1 = DAT_0040c618;
  while (puVar1 != (undefined4 *)0x0) {
    puVar3 = puVar1;
    if ((*(byte *)(puVar1 + 0x13) & 8) != 0) {
      if ((undefined4 *)puVar1[3] != (undefined4 *)0x0) {
        if (*(undefined4 **)puVar1[3] != puVar1) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x376);
        }
        *(undefined4 *)puVar1[3] = 0;
      }
      if (puVar1 == DAT_0040c72c) {
        DAT_0040c72c = (undefined4 *)0x0;
      }
      if (puVar1 == DAT_0040c64c) {
        DAT_0040c64c = (undefined4 *)0x0;
      }
      *puVar2 = *puVar1;
      *puVar1 = DAT_0040c744;
      puVar3 = puVar2;
      DAT_0040c744 = puVar1;
    }
    puVar2 = puVar3;
    puVar1 = (undefined4 *)*puVar3;
  }
  return;
}

