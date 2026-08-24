/* FUN_00405a10 @ 0x00405a10 size=48 */

void FUN_00405a10(void)

{
  if ((DAT_0040c694 != 0) && (DAT_0040c770 == 0)) {
    DAT_0040c67c = 1;
    FUN_00404ad0();
    return;
  }
  DAT_0040c67c = 0;
  FUN_004057c0();
  return;
}

