/* FUN_0040829e @ 0x0040829e size=54 */

void __cdecl FUN_0040829e(uint *param_1)

{
  int *piVar1;
  
  if ((param_1 <= DAT_0040c5c0) && (piVar1 = FUN_00408a29(param_1), piVar1 != (int *)0x0)) {
    return;
  }
  if (param_1 == (uint *)0x0) {
    param_1 = (uint *)0x1;
  }
  HeapAlloc(DAT_0040cba8,0,(int)param_1 + 0xfU & 0xfffffff0);
  return;
}

