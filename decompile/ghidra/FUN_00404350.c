/* FUN_00404350 @ 0x00404350 size=883 */

void __fastcall FUN_00404350(int param_1)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  uint uVar8;
  short sVar9;
  int iVar10;
  uint uVar11;
  short local_c;
  
  sVar9 = 0;
  iVar10 = *(int *)(param_1 + 0x10);
  iVar5 = *(int *)(param_1 + 0xc);
  if (param_1 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa68);
  }
  if (*(short *)(param_1 + 0x18) < 1) {
    *(undefined2 *)(param_1 + 0x1e) = 0;
    *(undefined2 *)(param_1 + 0x18) = 0;
  }
  else {
    *(short *)(param_1 + 0x1e) = *(short *)(param_1 + 0x1e) + -1;
  }
  if (*(short *)(param_1 + 0x18) != 0) goto LAB_004046b8;
  if ((0x31 < iVar10) && (iVar10 < 0x38)) {
    iVar5 = DAT_0040c698 - *(int *)(param_1 + 0x20);
    switch(iVar10) {
    case 0x32:
      *(undefined4 *)(param_1 + 0x10) = 0x33;
      return;
    case 0x33:
      *(uint *)(param_1 + 0x10) = ((499 < iVar5) - 1 & 0xfffffffe) + 0x34;
      return;
    case 0x34:
      if (700 < iVar5) {
        *(undefined4 *)(param_1 + 0x10) = 0x35;
        return;
      }
      break;
    case 0x35:
      if (1000 < iVar5) {
        *(undefined4 *)(param_1 + 0x10) = 0x36;
        return;
      }
      break;
    case 0x36:
      *(undefined4 *)(param_1 + 0x10) = 0x37;
      return;
    case 0x37:
      *(uint *)(param_1 + 0x10) = ((2999 < iVar5) - 1 & 0xc) + 0x2a;
      return;
    default:
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa76);
      *(int *)(param_1 + 0x10) = iVar10;
      return;
    }
    goto LAB_004046b8;
  }
  sVar1 = *(short *)(param_1 + 0x16);
  sVar2 = *(short *)(param_1 + 0x14);
  local_c = 0;
  if (iVar5 == 5) {
    if (sVar1 < -1999) goto LAB_004044dd;
    sVar9 = -10;
  }
  else if (iVar5 == 6) {
    if (31999 < sVar1) goto LAB_004044dd;
    sVar9 = 0x1a;
  }
  else if (iVar5 == 7) {
    if (sVar2 < -15999) {
LAB_004044dd:
      if (DAT_0040c72c != 0) {
        sVar3 = *(short *)(DAT_0040c72c + 0x40);
        sVar4 = *(short *)(DAT_0040c72c + 0x42);
        if (iVar5 == 5) {
          if (sVar4 < -2000) goto LAB_00404539;
        }
        else if (iVar5 == 6) {
          if (32000 < sVar4) goto LAB_00404539;
        }
        else if (iVar5 == 7) {
          if (sVar3 < -16000) {
LAB_00404539:
            iVar5 = (int)sVar3 - (int)sVar2;
            iVar7 = (int)sVar4 - (int)sVar1;
            sVar9 = (short)DAT_0040c5f0;
            if (sVar9 < iVar5) {
              sVar9 = -sVar9;
LAB_00404564:
              *(short *)(param_1 + 0x14) = sVar3 + sVar9;
            }
            else if (iVar5 < -(int)sVar9) goto LAB_00404564;
            if ((short)DAT_0040c6d8 < iVar7) {
              sVar9 = *(short *)(DAT_0040c72c + 0x42) - (short)DAT_0040c6d8;
LAB_00404598:
              *(short *)(param_1 + 0x16) = sVar9;
            }
            else if (iVar7 < -(int)(short)DAT_0040c6d8) {
              sVar9 = *(short *)(DAT_0040c72c + 0x42) + (short)DAT_0040c6d8;
              goto LAB_00404598;
            }
            if (iVar5 < 0x10) {
              if (iVar5 < -0xf) {
                iVar5 = -0x10;
              }
            }
            else {
              iVar5 = 0x10;
            }
            local_c = (short)iVar5;
            if (iVar7 < 0x1a) {
              if (iVar7 < -9) {
                iVar7 = -10;
              }
            }
            else {
              iVar7 = 0x1a;
            }
            sVar9 = (short)iVar7;
            FUN_00402ba0((undefined4 *)&DAT_0040c6f0);
          }
        }
        else if ((iVar5 == 8) && (16000 < sVar3)) goto LAB_00404539;
      }
    }
    else {
      local_c = -0x10;
    }
  }
  else {
    if ((iVar5 != 8) || (15999 < sVar2)) goto LAB_004044dd;
    local_c = 0x10;
  }
  uVar11 = (uint)local_c;
  uVar8 = (uint)sVar9;
  if ((int)((uVar8 ^ (int)uVar8 >> 0x1f) - ((int)uVar8 >> 0x1f)) <
      (int)((uVar11 ^ (int)uVar11 >> 0x1f) - ((int)uVar11 >> 0x1f))) {
    *(short *)(param_1 + 0x1c) =
         (short)((int)((int)*(short *)(param_1 + 0x1a) * uVar8) / (int)uVar11);
LAB_00404617:
    *(undefined2 *)(param_1 + 0x1e) = 1;
  }
  else if (sVar9 != 0) {
    *(short *)(param_1 + 0x1a) =
         (short)((int)((int)*(short *)(param_1 + 0x1c) * uVar11) / (int)uVar8);
    goto LAB_00404617;
  }
  *(short *)(param_1 + 0x1c) = sVar9;
  *(short *)(param_1 + 0x1a) = local_c;
  if (sVar9 < 0) {
    *(uint *)(param_1 + 0x10) = (iVar10 == 0x30) + 0x30;
    return;
  }
  if (local_c < 0) {
    *(uint *)(param_1 + 0x10) = (iVar10 == 0x2c) + 0x2c;
    return;
  }
  if ((local_c < 1) && (sVar9 < 1)) {
    uVar6 = FUN_004020b0(10);
    if ((short)uVar6 == 0) {
      *(undefined2 *)(param_1 + 0x1e) = 4;
      *(undefined4 *)(param_1 + 0x10) = 0x2b;
      return;
    }
    *(undefined4 *)(param_1 + 0x10) = 0x2a;
    return;
  }
  iVar10 = (iVar10 == 0x2e) + 0x2e;
LAB_004046b8:
  *(int *)(param_1 + 0x10) = iVar10;
  return;
}

