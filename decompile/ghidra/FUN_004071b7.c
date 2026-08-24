/* FUN_004071b7 @ 0x004071b7 size=67 */

int * __cdecl FUN_004071b7(int param_1)

{
  int *piVar1;
  
  piVar1 = &DAT_0040c390;
  if (DAT_0040c390 != param_1) {
    do {
      piVar1 = piVar1 + 3;
      if (&DAT_0040c390 + DAT_0040c410 * 3 <= piVar1) break;
    } while (*piVar1 != param_1);
  }
  if ((&DAT_0040c390 + DAT_0040c410 * 3 <= piVar1) || (*piVar1 != param_1)) {
    piVar1 = (int *)0x0;
  }
  return piVar1;
}

