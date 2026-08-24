/* FUN_00407f71 @ 0x00407f71 size=41 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00407f71(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_0040caa0;
  for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined1 *)puVar2 = 0;
  DAT_0040c984 = 0;
  _DAT_0040c99c = 0;
  DAT_0040cba4 = 0;
  DAT_0040c990 = 0;
  DAT_0040c994 = 0;
  DAT_0040c998 = 0;
  return;
}

