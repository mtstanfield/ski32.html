/* FUN_004068d0 @ 0x004068d0 size=119 */

LRESULT FUN_004068d0(HWND param_1,UINT param_2,WPARAM param_3,LPARAM param_4)

{
  int iVar1;
  LRESULT LVar2;
  
  switch(param_2) {
  case 1:
    iVar1 = FUN_00406a70(param_1);
    if (iVar1 == 0) {
      return -1;
    }
switchD_004068ea_caseD_5:
    GetClientRect(param_1,(LPRECT)&DAT_0040c778);
    goto switchD_004068ea_caseD_3;
  case 2:
    FUN_00406c80(param_1);
    return 0;
  default:
switchD_004068ea_caseD_3:
    LVar2 = DefWindowProcA(param_1,param_2,param_3,param_4);
    return LVar2;
  case 5:
    goto switchD_004068ea_caseD_5;
  case 0xf:
    FUN_00406970(param_1);
    return 0;
  }
}

