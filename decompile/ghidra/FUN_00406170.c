/* FUN_00406170 @ 0x00406170 size=565 */

void __fastcall FUN_00406170(undefined4 param_1)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  
  switch(param_1) {
  case 0xd:
    if (DAT_0040c72c != (undefined4 *)0x0) {
      return;
    }
  case 0x71:
    FUN_00406500();
    return;
  case 0x1b:
    ShowWindow(DAT_0040c6c8,6);
    return;
  case 0x72:
    FUN_00405760();
    return;
  }
  if (DAT_0040c72c == (undefined4 *)0x0) {
    return;
  }
  uVar3 = DAT_0040c72c[7];
  sVar1 = *(short *)(DAT_0040c72c + 0x11);
  if ((uVar3 == 0xb) || (uVar3 == 0x11)) goto switchD_004061ec_caseD_29;
  switch(param_1) {
  case 0x21:
  case 0x69:
    if (sVar1 == 0) {
      uVar3 = 6;
    }
    break;
  case 0x22:
  case 99:
    if (sVar1 == 0) {
      uVar3 = 4;
    }
    break;
  case 0x23:
  case 0x61:
    if (sVar1 == 0) {
      uVar3 = 1;
    }
    break;
  case 0x24:
  case 0x67:
    if (sVar1 == 0) {
      uVar3 = 3;
    }
    break;
  case 0x25:
  case 100:
    if (0x15 < uVar3) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xf63);
    }
    uVar3 = *(uint *)(&DAT_0040a258 + uVar3 * 8);
    if (uVar3 == 7) {
      iVar2 = *(short *)((int)DAT_0040c72c + 0x46) + -8;
      if (iVar2 < -7) {
        iVar2 = -8;
      }
      *(short *)((int)DAT_0040c72c + 0x46) = (short)iVar2;
    }
    break;
  case 0x26:
  case 0x68:
    switch(uVar3) {
    case 3:
    case 7:
    case 0xc:
      if (*(short *)(DAT_0040c72c + 0x12) == 0) {
        uVar3 = 9;
        *(undefined2 *)(DAT_0040c72c + 0x12) = 0xfffc;
      }
      break;
    case 6:
    case 8:
      if (*(short *)(DAT_0040c72c + 0x12) == 0) {
        uVar3 = 10;
        *(undefined2 *)(DAT_0040c72c + 0x12) = 0xfffc;
      }
      break;
    case 0xd:
switchD_004062bd_caseD_d:
      uVar3 = 0x12;
      break;
    case 0xe:
      uVar3 = 0x14;
      break;
    case 0xf:
      uVar3 = 0x15;
      break;
    case 0x12:
switchD_004062bd_caseD_12:
      uVar3 = 0x13;
      break;
    case 0x13:
switchD_004062bd_caseD_13:
      uVar3 = 0xd;
    }
    break;
  case 0x27:
  case 0x66:
    if (0x15 < uVar3) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xf6b);
    }
    uVar3 = *(uint *)(&DAT_0040a25c + uVar3 * 8);
    if (uVar3 == 8) {
      iVar2 = *(short *)((int)DAT_0040c72c + 0x46) + 8;
      if (7 < iVar2) {
        iVar2 = 8;
      }
      *(short *)((int)DAT_0040c72c + 0x46) = (short)iVar2;
    }
    break;
  case 0x28:
  case 0x62:
    if (sVar1 == 0) {
      uVar3 = 0;
      break;
    }
    switch(uVar3) {
    case 0xd:
      goto switchD_004062bd_caseD_12;
    case 0x12:
      goto switchD_004062bd_caseD_13;
    case 0x13:
      goto switchD_004062bd_caseD_d;
    case 0x14:
      uVar3 = 0xe;
      break;
    case 0x15:
      uVar3 = 0xf;
    }
    break;
  case 0x2d:
  case 0x60:
    if (sVar1 == 0) {
      *(undefined2 *)((int)DAT_0040c72c + 0x4a) = 2;
      uVar3 = 0xd;
      if (4 < *(short *)(DAT_0040c72c + 0x12)) {
        *(short *)(DAT_0040c72c + 0x12) = *(short *)(DAT_0040c72c + 0x12) + -4;
      }
    }
  }
switchD_004061ec_caseD_29:
  if ((uVar3 != DAT_0040c72c[7]) && (FUN_00402120(DAT_0040c72c,uVar3), DAT_0040c610 != 0)) {
    FUN_00401060(DAT_0040c63c,&DAT_0040c6b0);
    DAT_0040c610 = 0;
  }
  return;
}

