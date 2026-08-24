/* FUN_004066d0 @ 0x004066d0 size=148 */

void FUN_004066d0(void)

{
  uint uVar1;
  
  if (DAT_0040c72c == (undefined4 *)0x0) {
    FUN_00406500();
    return;
  }
  uVar1 = DAT_0040c72c[7];
  if (uVar1 != 0xb) {
    if (*(short *)(DAT_0040c72c + 0x11) == 0) {
      *(undefined2 *)((int)DAT_0040c72c + 0x4a) = 4;
      uVar1 = 0xd;
    }
    else if (uVar1 != 0x11) {
      switch(uVar1) {
      case 0xd:
        uVar1 = 0x12;
        break;
      case 0xe:
        uVar1 = 0x14;
        break;
      case 0xf:
        uVar1 = 0x15;
        break;
      case 0x12:
        uVar1 = 0x13;
        break;
      case 0x13:
        uVar1 = 0xd;
      }
    }
  }
  if ((uVar1 != DAT_0040c72c[7]) && (FUN_00402120(DAT_0040c72c,uVar1), DAT_0040c610 != 0)) {
    FUN_00401060(DAT_0040c63c,&DAT_0040c6b0);
    DAT_0040c610 = 0;
  }
  return;
}

