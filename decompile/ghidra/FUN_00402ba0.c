/* FUN_00402ba0 @ 0x00402ba0 size=58 */

void __fastcall FUN_00402ba0(undefined4 *param_1)

{
  LPVOID pvVar1;
  
  if (DAT_0040c794 == 0) {
    if ((param_1[1] == 0) && ((HGLOBAL)*param_1 != (HGLOBAL)0x0)) {
      pvVar1 = LockResource((HGLOBAL)*param_1);
      param_1[1] = pvVar1;
    }
    if ((param_1[1] != 0) && (DAT_0040c790 != (code *)0x0)) {
      (*DAT_0040c790)(param_1[1],5);
    }
  }
  return;
}

