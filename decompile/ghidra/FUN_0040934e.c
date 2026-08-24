/* FUN_0040934e @ 0x0040934e size=27 */

undefined4 __cdecl FUN_0040934e(undefined4 param_1)

{
  int iVar1;
  
  if (DAT_0040c93c != (code *)0x0) {
    iVar1 = (*DAT_0040c93c)(param_1);
    if (iVar1 != 0) {
      return 1;
    }
  }
  return 0;
}

