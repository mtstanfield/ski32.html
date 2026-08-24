/* FUN_004027e0 @ 0x004027e0 size=100 */

int FUN_004027e0(void)

{
  ushort uVar1;
  undefined4 uVar2;
  
  if ((int)(DAT_0040c748 + (DAT_0040c748 >> 0x1f & 0x1fU)) >> 5 < DAT_0040c6fc) {
    return 0x12;
  }
  uVar2 = FUN_004020b0(100);
  uVar1 = (ushort)uVar2;
  if (uVar1 < 2) {
    return 10;
  }
  if (uVar1 < 0x14) {
    return 0xd;
  }
  if (uVar1 < 0x32) {
    return 0xf;
  }
  if (uVar1 < 0x3c) {
    return 0xb;
  }
  return (-(uint)(uVar1 < 0x50) & 0xfffffffe) + 0x10;
}

