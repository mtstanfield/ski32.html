/* FUN_004052d0 @ 0x004052d0 size=842 */

/* WARNING: Removing unreachable block (ram,0x004053c9) */

undefined4 __fastcall FUN_004052d0(HINSTANCE param_1,int param_2,int param_3)

{
  bool bVar1;
  ATOM AVar2;
  short sVar3;
  HDC hdc;
  int iVar4;
  BOOL BVar5;
  undefined3 extraout_var;
  int X;
  char *lpWindowName;
  int nWidth;
  DWORD dwStyle;
  int Y;
  HWND hWndParent;
  HMENU hMenu;
  HINSTANCE hInstance;
  LPVOID lpParam;
  WNDCLASSA local_28;
  
  hdc = GetDC((HWND)0x0);
  if (hdc == (HDC)0x0) {
    return 0;
  }
  iVar4 = GetDeviceCaps(hdc,8);
  DAT_0040c6a0 = CONCAT22(DAT_0040c6a0._2_2_,(short)iVar4);
  iVar4 = GetDeviceCaps(hdc,10);
  DAT_0040c74c = CONCAT22(DAT_0040c74c._2_2_,(short)iVar4);
  ReleaseDC((HWND)0x0,hdc);
  DAT_0040c61c = param_1;
  DAT_0040c69c = GetStockObject(0);
  DAT_0040c6c8 = (HWND)0x0;
  DAT_0040c624 = (HWND)0x0;
  DAT_0040c6d0 = 0;
  DAT_0040c770 = 1;
  DAT_0040c694 = 0;
  DAT_0040c67c = 0;
  DAT_0040c704._0_2_ = 0;
  DAT_0040c5fc._0_2_ = 0;
  DAT_0040c6c8 = FindWindowA("SkiMain",(LPCSTR)0x0);
  if (DAT_0040c6c8 != (HWND)0x0) {
    SetWindowPos(DAT_0040c6c8,(HWND)0x0,0,0,0,0,3);
    BVar5 = IsIconic(DAT_0040c6c8);
    if (BVar5 != 0) {
      OpenIcon(DAT_0040c6c8);
    }
    DAT_0040c6c8 = (HWND)0x0;
    return 0;
  }
  DAT_0040c940 = &LAB_004047c0;
  if ((DAT_0040c794 == 0) && (bVar1 = FUN_00405620(), CONCAT31(extraout_var,bVar1) != 0)) {
    FUN_00405640(1,(undefined4 *)&DAT_0040c6c0);
    FUN_00405640(2,(undefined4 *)&DAT_0040c768);
    FUN_00405640(3,(undefined4 *)&DAT_0040c5d0);
    FUN_00405640(4,(undefined4 *)&DAT_0040c718);
    FUN_00405640(5,(undefined4 *)&DAT_0040c750);
    FUN_00405640(6,(undefined4 *)&DAT_0040c628);
    FUN_00405640(9,(undefined4 *)&DAT_0040c6f0);
    FUN_00405640(7,(undefined4 *)&DAT_0040c6e0);
    FUN_00405640(8,(undefined4 *)&DAT_0040c608);
  }
  if (param_2 == 0) {
    local_28.style = 0x2023;
    local_28.lpfnWndProc = (WNDPROC)&LAB_00405800;
    local_28.cbClsExtra = 0;
    local_28.cbWndExtra = 0;
    local_28.hInstance = param_1;
    local_28.hIcon = LoadIconA(param_1,s_iconSki_0040c120);
    local_28.hCursor = LoadCursorA((HINSTANCE)0x0,(LPCSTR)0x7f00);
    local_28.hbrBackground = DAT_0040c69c;
    local_28.lpszMenuName = (LPCSTR)0x0;
    local_28.lpszClassName = "SkiMain";
    AVar2 = RegisterClassA(&local_28);
    if (AVar2 == 0) {
      return 0;
    }
    local_28.lpfnWndProc = FUN_004068d0;
    local_28.hIcon = (HICON)0x0;
    local_28.hCursor = LoadCursorA((HINSTANCE)0x0,(LPCSTR)0x7f00);
    local_28.lpszClassName = "SkiStatus";
    local_28.hbrBackground = DAT_0040c69c;
    AVar2 = RegisterClassA(&local_28);
    if (AVar2 == 0) {
      return 0;
    }
  }
  sVar3 = (short)DAT_0040c6a0;
  if ((short)DAT_0040c74c <= (short)DAT_0040c6a0) {
    sVar3 = (short)DAT_0040c74c;
  }
  nWidth = (int)sVar3;
  lpParam = (LPVOID)0x0;
  iVar4 = (int)(short)DAT_0040c74c;
  hMenu = (HMENU)0x0;
  hWndParent = (HWND)0x0;
  Y = 0;
  X = ((short)DAT_0040c6a0 - nWidth) / 2;
  dwStyle = 0x2cf0000;
  hInstance = param_1;
  lpWindowName = FUN_00401cf0(1);
  DAT_0040c6c8 = CreateWindowExA(0,"SkiMain",lpWindowName,dwStyle,X,Y,nWidth,iVar4,hWndParent,hMenu,
                                 hInstance,lpParam);
  if (DAT_0040c6c8 != (HWND)0x0) {
    DAT_0040c624 = CreateWindowExA(0,"SkiStatus",&DAT_0040c788,0x40000000,0,0,0,0,DAT_0040c6c8,
                                   (HMENU)0x0,param_1,(LPVOID)0x0);
    if (DAT_0040c624 != (HWND)0x0) {
      ShowWindow(DAT_0040c6c8,param_3);
      UpdateWindow(DAT_0040c6c8);
      ShowWindow(DAT_0040c624,1);
      UpdateWindow(DAT_0040c624);
      return 1;
    }
    DestroyWindow(DAT_0040c6c8);
    return 0;
  }
  return 0;
}

