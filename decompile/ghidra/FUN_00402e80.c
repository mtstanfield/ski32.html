/* FUN_00402e80 @ 0x00402e80 size=56 */

void FUN_00402e80(void)

{
  uint uVar1;
  
  if (DAT_0040c72c != (undefined4 *)0x0) {
    uVar1 = DAT_0040c72c[7];
    if ((uVar1 != 0xb) && (uVar1 != 0x11)) {
      uVar1 = ((*(short *)(DAT_0040c72c + 0x11) < 1) - 1 & 0xb) + 3;
    }
    FUN_00402120(DAT_0040c72c,uVar1);
    FUN_00401b80(DAT_0040c6cc);
    return;
  }
  return;
}

