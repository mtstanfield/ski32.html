/* FUN_004026f0 @ 0x004026f0 size=122 */

int FUN_004026f0(void)

{
  ushort uVar1;
  undefined4 uVar2;
  
  if ((int)(DAT_0040c748 + (DAT_0040c748 >> 0x1f & 0x1fU)) >> 5 < DAT_0040c6fc) {
    return 0x12;
  }
  uVar2 = FUN_004020b0(1000);
  uVar1 = (ushort)uVar2;
  if (uVar1 < 0x32) {
    return 10;
  }
  if (uVar1 < 500) {
    return 0xd;
  }
  if (uVar1 < 700) {
    return 0xf;
  }
  if (uVar1 < 0x2ee) {
    return 0xb;
  }
  if (uVar1 < 0x3b6) {
    return 0xe;
  }
  if (uVar1 < 0x3ca) {
    return 0x10;
  }
  return 2 - (uint)(uVar1 < 0x3de);
}

