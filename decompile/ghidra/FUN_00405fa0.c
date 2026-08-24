/* FUN_00405fa0 @ 0x00405fa0 size=179 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00405fa0(HWND param_1)

{
  DAT_0040c760 = 0;
  GetClientRect(param_1,(LPRECT)&DAT_0040c6b0);
  FUN_00406060((short)((DAT_0040c6b0 + DAT_0040c6b8) / 2),
               (short)((ulonglong)((longlong)(DAT_0040c6bc + DAT_0040c6b4) * 0x55555556) >> 0x20) -
               (short)(DAT_0040c6bc + DAT_0040c6b4 >> 0x1f));
  DAT_0040c684 = DAT_0040c6b4 + -0x78;
  DAT_0040c6d8._0_2_ = (short)DAT_0040c6bc - (short)DAT_0040c6b4;
  _DAT_0040c688 = DAT_0040c6b8 + 0x78;
  DAT_0040c68c = DAT_0040c6bc + 0x78;
  _DAT_0040c680 = DAT_0040c6b0 + -0x78;
  DAT_0040c5f0._0_2_ = (short)DAT_0040c6b8 - (short)DAT_0040c6b0;
  DAT_0040c748 = ((DAT_0040c6bc + 0x78) - (DAT_0040c6b4 + -0x78)) *
                 ((DAT_0040c6b8 + 0x78) - (DAT_0040c6b0 + -0x78));
  return;
}

