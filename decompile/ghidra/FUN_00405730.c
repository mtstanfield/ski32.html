/* FUN_00405730 @ 0x00405730 size=38 */

void __fastcall FUN_00405730(undefined4 *param_1)

{
  if (param_1[1] != 0) {
    param_1[1] = 0;
  }
  if ((HGLOBAL)*param_1 != (HGLOBAL)0x0) {
    FreeResource((HGLOBAL)*param_1);
    *param_1 = 0;
  }
  return;
}

