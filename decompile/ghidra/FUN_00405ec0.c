/* FUN_00405ec0 @ 0x00405ec0 size=214 */

void __fastcall FUN_00405ec0(HWND param_1)

{
  HGDIOBJ pvVar1;
  
  ReleaseDC(param_1,DAT_0040c63c);
  FUN_004057c0();
  if (DAT_0040c620 != (HGDIOBJ)0x0) {
    pvVar1 = SelectObject(DAT_0040c710,DAT_0040c620);
    DeleteObject(pvVar1);
  }
  if (DAT_0040c644 != (HGDIOBJ)0x0) {
    pvVar1 = SelectObject(DAT_0040c730,DAT_0040c644);
    DeleteObject(pvVar1);
  }
  if (DAT_0040c6d4 != (HGDIOBJ)0x0) {
    pvVar1 = SelectObject(DAT_0040c6a4,DAT_0040c6d4);
    DeleteObject(pvVar1);
  }
  if (DAT_0040c75c != (HGDIOBJ)0x0) {
    pvVar1 = SelectObject(DAT_0040c6ec,DAT_0040c75c);
    DeleteObject(pvVar1);
  }
  if (DAT_0040c614 != (HGDIOBJ)0x0) {
    pvVar1 = SelectObject(DAT_0040c5ec,DAT_0040c614);
    DeleteObject(pvVar1);
  }
  if (DAT_0040c710 != (HDC)0x0) {
    DeleteDC(DAT_0040c710);
  }
  if (DAT_0040c730 != (HDC)0x0) {
    DeleteDC(DAT_0040c730);
  }
  if (DAT_0040c6a4 != (HDC)0x0) {
    DeleteDC(DAT_0040c6a4);
  }
  if (DAT_0040c6ec != (HDC)0x0) {
    DeleteDC(DAT_0040c6ec);
  }
  if (DAT_0040c5ec != (HDC)0x0) {
    DeleteDC(DAT_0040c5ec);
  }
  return;
}

