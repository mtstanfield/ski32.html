/* FUN_00405800 @ 0x00405800 size=513 */

LRESULT FUN_00405800(HWND param_1,uint param_2,WPARAM param_3,int param_4)

{
  int iVar1;
  LRESULT LVar2;
  
  if (param_2 < 0x25) {
    if (param_2 == 0x24) {
      *(undefined4 *)(param_4 + 0x18) = 0x140;
      *(undefined4 *)(param_4 + 0x1c) = 300;
      return 0;
    }
    switch(param_2) {
    case 1:
      iVar1 = FUN_00405a40(param_1);
      if (iVar1 == 0) {
        return -1;
      }
      FUN_00405fa0(param_1);
      return 0;
    case 2:
      FUN_00405ec0(param_1);
      PostQuitMessage(0);
      return 0;
    default:
switchD_00405828_caseD_3:
      LVar2 = DefWindowProcA(param_1,param_2,param_3,param_4);
      return LVar2;
    case 5:
      FUN_00405fa0(param_1);
      if (DAT_0040c624 != 0) {
        FUN_00406890();
      }
      DAT_0040c770 = (uint)(param_3 == 1);
      FUN_00405a10();
      if (DAT_0040c67c != 0) {
        UpdateWindow(DAT_0040c6c8);
        return 0;
      }
      break;
    case 6:
      DAT_0040c694 = param_3;
      if (param_3 != 0) {
        SetFocus(param_1);
      }
      FUN_00405a10();
      return 0;
    case 0xf:
      FUN_004060b0(param_1);
      return 0;
    case 0x21:
      if ((short)param_4 == 1) {
        return 2;
      }
    }
  }
  else if (param_2 < 0x201) {
    if (param_2 == 0x200) {
      if (DAT_0040c67c != 0) {
        FUN_00406550((short)param_4,(short)((uint)param_4 >> 0x10));
        return 0;
      }
    }
    else if (param_2 == 0x100) {
      if (DAT_0040c67c != 0) {
        FUN_00406170(param_3);
        return 0;
      }
    }
    else {
      if (param_2 != 0x102) goto switchD_00405828_caseD_3;
      if (DAT_0040c67c != 0) {
        FUN_00406780(param_3);
        return 0;
      }
    }
  }
  else {
    if ((param_2 != 0x201) && (param_2 != 0x203)) goto switchD_00405828_caseD_3;
    if (DAT_0040c67c != 0) {
      FUN_004066d0();
    }
  }
  return 0;
}

