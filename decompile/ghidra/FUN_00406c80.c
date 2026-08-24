/* FUN_00406c80 @ 0x00406c80 size=65 */

void __fastcall FUN_00406c80(HWND param_1)

{
  if (param_1 != DAT_0040c624) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x1123);
  }
  if (DAT_0040c664 != (HGDIOBJ)0x0) {
    SelectObject(DAT_0040c6cc,DAT_0040c664);
  }
  ReleaseDC(param_1,DAT_0040c6cc);
  return;
}

