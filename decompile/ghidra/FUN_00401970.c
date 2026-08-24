/* FUN_00401970 @ 0x00401970 size=240 */

undefined4 __fastcall FUN_00401970(ushort param_1,ushort param_2)

{
  HGDIOBJ ho;
  HBITMAP h;
  
  if (((short)DAT_0040c690 < (short)param_1) || ((short)DAT_0040c6e8 < (short)param_2)) {
    DAT_0040c690 = (param_1 & 0xffc0) + 0x40;
    DAT_0040c6e8 = (param_2 & 0xffc0) + 0x40;
    if (DAT_0040c614 != (HGDIOBJ)0x0) {
      ho = SelectObject(DAT_0040c5ec,DAT_0040c614);
      DeleteObject(ho);
      DAT_0040c614 = (HGDIOBJ)0x0;
    }
    h = CreateCompatibleBitmap(DAT_0040c63c,(int)(short)DAT_0040c690,(int)(short)DAT_0040c6e8);
    while (h == (HBITMAP)0x0) {
      if ((DAT_0040c690 == param_1) && (DAT_0040c6e8 == param_2)) {
        DAT_0040c6e8 = 0;
        DAT_0040c690 = 0;
        return 0;
      }
      DAT_0040c690 = param_1;
      DAT_0040c6e8 = param_2;
      h = CreateCompatibleBitmap(DAT_0040c63c,(int)(short)param_1,(int)(short)param_2);
    }
    DAT_0040c614 = SelectObject(DAT_0040c5ec,h);
  }
  return 1;
}

