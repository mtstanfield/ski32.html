/* FUN_004060b0 @ 0x004060b0 size=71 */

void __fastcall FUN_004060b0(HWND param_1)

{
  tagPAINTSTRUCT local_40;
  
  BeginPaint(param_1,&local_40);
  FillRect(local_40.hdc,&local_40.rcPaint,DAT_0040c69c);
  FUN_00406100(local_40.hdc,&local_40.rcPaint.left);
  EndPaint(param_1,&local_40);
  return;
}

