/* FUN_004048c0 @ 0x004048c0 size=138 */

undefined4 FUN_004048c0(void)

{
  int iVar1;
  int iVar2;
  
  DAT_0040c674 = LocalAlloc(0,0x50);
  DAT_0040c5f8 = LocalAlloc(0,0x5a0);
  DAT_0040c648 = LocalAlloc(0,8000);
  DAT_0040c758 = LocalAlloc(0,0x2400);
  if ((((DAT_0040c674 != (HLOCAL)0x0) && (DAT_0040c648 != (HLOCAL)0x0)) &&
      (DAT_0040c5f8 != (HLOCAL)0x0)) && (DAT_0040c758 != (HLOCAL)0x0)) {
    iVar1 = 0;
    do {
      iVar2 = iVar1 + 4;
      *(undefined4 *)(iVar1 + (int)DAT_0040c674) = 0;
      iVar1 = iVar2;
    } while (iVar2 < 0x50);
    return 1;
  }
  FUN_00404950(s_Insufficient_local_memory__0040c104);
  return 0;
}

