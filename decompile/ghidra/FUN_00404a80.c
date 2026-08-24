/* FUN_00404a80 @ 0x00404a80 size=71 */

undefined4 FUN_00404a80(void)

{
  undefined4 *puVar1;
  short sVar2;
  short sVar3;
  
  sVar3 = 0;
  sVar2 = 0;
  puVar1 = FUN_004020d0(0,3);
  DAT_0040c64c = FUN_00402390(puVar1,0,sVar2,sVar3);
  DAT_0040c72c = DAT_0040c64c;
  if (DAT_0040c64c == (undefined4 *)0x0) {
    return 0;
  }
  FUN_004051e0();
  FUN_00404b50();
  DAT_0040c650 = 0;
  FUN_00404ad0();
  return 1;
}

