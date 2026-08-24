/* FUN_00407a44 @ 0x00407a44 size=57 */

void FUN_00407a44(void)

{
  if ((DAT_0040c7a0 == 1) || ((DAT_0040c7a0 == 0 && (DAT_0040c174 == 1)))) {
    FUN_00407a7d(0xfc);
    if (DAT_0040c8fc != (code *)0x0) {
      (*DAT_0040c8fc)();
    }
    FUN_00407a7d(0xff);
  }
  return;
}

