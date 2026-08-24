/* FUN_00406780 @ 0x00406780 size=203 */

void __fastcall FUN_00406780(undefined4 param_1)

{
  switch(param_1) {
  case 0x58:
    if (DAT_0040c72c != (undefined4 *)0x0) {
      FUN_00402390(DAT_0040c72c,*(short *)(DAT_0040c72c + 0x10) + -2,
                   *(short *)((int)DAT_0040c72c + 0x42),*(short *)(DAT_0040c72c + 0x11));
      return;
    }
    break;
  case 0x59:
    if (DAT_0040c72c != (undefined4 *)0x0) {
      FUN_00402390(DAT_0040c72c,*(short *)(DAT_0040c72c + 0x10),
                   *(short *)((int)DAT_0040c72c + 0x42) + -2,*(short *)(DAT_0040c72c + 0x11));
    }
    break;
  case 0x66:
    DAT_0040c670 = (uint)(DAT_0040c670 == 0);
    return;
  case 0x72:
    FUN_00401060(DAT_0040c63c,&DAT_0040c6b0);
    return;
  case 0x74:
    FUN_00401000();
    return;
  case 0x78:
    if (DAT_0040c72c != (undefined4 *)0x0) {
      FUN_00402390(DAT_0040c72c,*(short *)(DAT_0040c72c + 0x10) + 2,
                   *(short *)((int)DAT_0040c72c + 0x42),*(short *)(DAT_0040c72c + 0x11));
      return;
    }
    break;
  case 0x79:
    if (DAT_0040c72c != (undefined4 *)0x0) {
      FUN_00402390(DAT_0040c72c,*(short *)(DAT_0040c72c + 0x10),
                   *(short *)((int)DAT_0040c72c + 0x42) + 2,*(short *)(DAT_0040c72c + 0x11));
      return;
    }
  }
  return;
}

