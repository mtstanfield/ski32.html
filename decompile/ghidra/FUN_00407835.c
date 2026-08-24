/* FUN_00407835 @ 0x00407835 size=60 */

undefined4 __cdecl FUN_00407835(int param_1)

{
  int iVar1;
  
  DAT_0040cba8 = HeapCreate((uint)(param_1 == 0),0x1000,0);
  if (DAT_0040cba8 != (HANDLE)0x0) {
    iVar1 = FUN_00408695();
    if (iVar1 != 0) {
      return 1;
    }
    HeapDestroy(DAT_0040cba8);
  }
  return 0;
}

