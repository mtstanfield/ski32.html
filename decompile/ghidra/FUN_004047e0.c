/* FUN_004047e0 @ 0x004047e0 size=213 */

undefined4 FUN_004047e0(HINSTANCE param_1,int param_2,LPCSTR param_3,int param_4)

{
  int iVar1;
  tagMSG local_1c;
  
  iVar1 = lstrcmpiA(param_3,s_nosound_0040c0fc);
  if (iVar1 == 0) {
    DAT_0040c794 = 1;
  }
  iVar1 = FUN_004048c0();
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_00404970();
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_004052d0(param_1,param_2,param_4);
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_00404a80();
  if (iVar1 == 0) {
    DestroyWindow(DAT_0040c6c8);
    FUN_004056a0();
    return 0;
  }
  iVar1 = GetMessageA(&local_1c,(HWND)0x0,0,0);
  while (iVar1 != 0) {
    TranslateMessage(&local_1c);
    DispatchMessageA(&local_1c);
    iVar1 = GetMessageA(&local_1c,(HWND)0x0,0,0);
  }
  FUN_004056a0();
  return local_1c.wParam;
}

