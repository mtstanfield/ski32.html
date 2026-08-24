/* FUN_00407ef4 @ 0x00407ef4 size=74 */

int __cdecl FUN_00407ef4(int param_1)

{
  int iVar1;
  bool bVar2;
  
  if (param_1 == -2) {
    DAT_0040c904 = 1;
                    /* WARNING: Could not recover jumptable at 0x00407f0e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    iVar1 = GetOEMCP();
    return iVar1;
  }
  if (param_1 == -3) {
    DAT_0040c904 = 1;
                    /* WARNING: Could not recover jumptable at 0x00407f23. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    iVar1 = GetACP();
    return iVar1;
  }
  bVar2 = param_1 == -4;
  if (bVar2) {
    param_1 = DAT_0040c92c;
  }
  DAT_0040c904 = (uint)bVar2;
  return param_1;
}

