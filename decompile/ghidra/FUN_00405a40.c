/* FUN_00405a40 @ 0x00405a40 size=111 */

undefined4 __fastcall FUN_00405a40(HWND param_1)

{
  int iVar1;
  
  DAT_0040c63c = GetDC(param_1);
  if (DAT_0040c63c == (HDC)0x0) {
    return 0;
  }
  DAT_0040c710 = 0;
  DAT_0040c6a4 = 0;
  DAT_0040c730 = 0;
  DAT_0040c6ec = 0;
  DAT_0040c5ec = 0;
  DAT_0040c620 = 0;
  DAT_0040c6d4 = 0;
  DAT_0040c644 = 0;
  DAT_0040c75c = 0;
  DAT_0040c614 = 0;
  iVar1 = FUN_00405ab0(DAT_0040c63c);
  if (iVar1 == 0) {
    FUN_00404950(s_Whoa__like__can_t_load_bitmaps__Y_0040c130);
    return 0;
  }
  return 1;
}

