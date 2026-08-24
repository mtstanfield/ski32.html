/* FUN_00403a00 @ 0x00403a00 size=1492 */

undefined4 * __fastcall FUN_00403a00(undefined4 *param_1,undefined4 *param_2)

{
  short sVar1;
  short sVar2;
  short sVar3;
  bool bVar4;
  int iVar5;
  undefined4 *puVar6;
  uint uVar7;
  short sVar8;
  uint local_c;
  
  if (param_1 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x92e);
  }
  if (param_2 == (undefined4 *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x92f);
  }
  if (10 < (int)param_1[6]) {
    return param_1;
  }
  sVar1 = *(short *)((int)param_1 + 0x42);
  sVar8 = *(short *)((int)param_2 + 0x42);
  iVar5 = FUN_00404070((int)param_1);
  sVar2 = *(short *)(iVar5 + 0x42);
  iVar5 = FUN_00404070((int)param_2);
  sVar3 = *(short *)(iVar5 + 0x42);
  if ((((sVar1 < sVar8) || (sVar3 < sVar2)) && ((sVar8 < sVar1 || (sVar2 < sVar3)))) ||
     ((sVar1 == sVar8 && (sVar2 == sVar3)))) {
    bVar4 = false;
  }
  else {
    bVar4 = true;
  }
  iVar5 = param_2[6];
  local_c = param_1[7];
  sVar1 = *(short *)(param_1 + 0x11);
  sVar8 = *(short *)(param_2[5] + 0xc) + *(short *)(param_2 + 0x11);
  switch(param_1[6]) {
  case 0:
    if (local_c == 0x11) break;
    switch(iVar5) {
    case 2:
    case 0xc:
    case 0x11:
      if (bVar4) {
        *(short *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) / 2;
      }
      if (*(short *)(param_2 + 4) == 0x52) {
        FUN_00403420(-0x10);
        puVar6 = FUN_00402120(param_1,local_c);
        return puVar6;
      }
      break;
    case 0xb:
      if (local_c == 0) {
        local_c = 0xd;
        *(undefined2 *)((int)param_1 + 0x4a) = 1;
        if (4 < *(short *)(param_1 + 0x12)) {
          *(short *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) / 2;
          puVar6 = FUN_00402120(param_1,0xd);
          return puVar6;
        }
      }
      break;
    case 0xe:
      if (0 < sVar1) {
        if (sVar8 < sVar1) {
          if (*(short *)(param_2 + 4) == 0x56) {
            if ((*(byte *)(param_2 + 0x13) & 1) != 0) {
              param_2 = FUN_00402220(param_2);
            }
            FUN_00401350((int)param_2);
            FUN_00403420(100);
            puVar6 = FUN_00402120(param_1,local_c);
            return puVar6;
          }
          break;
        }
        if (!bVar4) break;
        goto LAB_00403bcc;
      }
    case 1:
    case 3:
    case 4:
    case 9:
    case 10:
    case 0xd:
      if ((sVar8 < sVar1) ||
         ((short)(*(short *)(param_1[5] + 0xc) + sVar1) < *(short *)(param_2 + 0x11))) {
        if (iVar5 == 9) {
          FUN_00403420(1000);
          param_2[6] = 0xd;
          FUN_00402180(param_2,0x32);
          puVar6 = FUN_00402120(param_1,local_c);
          return puVar6;
        }
        FUN_00403420(6);
        puVar6 = FUN_00402120(param_1,local_c);
        return puVar6;
      }
      if (bVar4) {
        if (iVar5 == 0xd) {
          sVar8 = *(short *)(param_2[5] + 10);
          if (sVar8 < *(short *)(param_1[5] + 10)) {
            sVar8 = *(short *)(param_1[5] + 10);
          }
          uVar7 = (int)*(short *)(param_1 + 0x10) - (int)*(short *)(param_2 + 0x10) >> 0x1f;
          if ((int)sVar8 / 2 <
              (int)(((int)*(short *)(param_1 + 0x10) - (int)*(short *)(param_2 + 0x10) ^ uVar7) -
                   uVar7)) {
            *(short *)(param_1 + 0x12) = *(short *)(param_1 + 0x12) / 2;
            puVar6 = FUN_00402120(param_1,local_c);
            return puVar6;
          }
        }
        if ((sVar1 == 0) && (*(short *)((int)param_1 + 0x4a) == 0)) {
          local_c = 0xb;
        }
        else {
          local_c = 0x11;
          if (*(short *)(param_2 + 4) == 0x32) {
            param_2[6] = 9;
            FUN_00402120(param_2,0x38);
            FUN_00403420(0x10);
            puVar6 = FUN_00402120(param_1,0x11);
            return puVar6;
          }
        }
        if ((*(short *)(param_1 + 0x12) < 0) && (*(short *)(param_2 + 4) == 0x2e)) {
          FUN_00402180(param_2,0x56);
          puVar6 = FUN_00402120(param_1,local_c);
          return puVar6;
        }
        FUN_00403420(-0x20);
        FUN_00402ba0((undefined4 *)&DAT_0040c6c0);
        puVar6 = FUN_00402120(param_1,local_c);
        return puVar6;
      }
      break;
    case 0xf:
      if (sVar1 < 1) {
        *(undefined2 *)((int)param_1 + 0x4a) = 4;
LAB_00403cb4:
        FUN_00403420(1);
        FUN_00402ba0((undefined4 *)&DAT_0040c768);
        puVar6 = FUN_00402120(param_1,0xd);
        return puVar6;
      }
      if (sVar8 <= sVar1) break;
LAB_00403bcc:
      *(short *)((int)param_1 + 0x4a) = *(short *)(param_1 + 0x12) / 2;
      FUN_00403420(1);
      puVar6 = (undefined4 *)&DAT_0040c768;
LAB_00403be8:
      FUN_00402ba0(puVar6);
      puVar6 = FUN_00402120(param_1,local_c);
      return puVar6;
    case 0x10:
      if (((bVar4) && ((int)sVar1 < (int)sVar8 / 2)) && (0 < *(short *)(param_1 + 0x12))) {
        *(short *)((int)param_1 + 0x4a) = *(short *)(param_1 + 0x12);
        goto LAB_00403cb4;
      }
    }
    break;
  case 1:
    if (0x18 < (int)local_c) break;
    if (iVar5 == 0) {
      FUN_00403420(0x14);
    }
    puVar6 = (undefined4 *)&DAT_0040c628;
    local_c = (0 < *(short *)(param_2 + 0x11)) + 0x19;
    goto LAB_00403be8;
  case 2:
    if (((int)local_c < 0x1d) &&
       ((*(short *)((int)param_2 + 0x46) != 0 || (*(short *)(param_2 + 0x12) != 0)))) {
      if (iVar5 == 0) {
        FUN_00403420(3);
      }
      local_c = 0x1d;
      puVar6 = (undefined4 *)&DAT_0040c5d0;
      goto LAB_00403be8;
    }
    break;
  case 3:
    switch(iVar5) {
    case 0:
      FUN_00403420(0x14);
    case 1:
    case 3:
    case 0xd:
    case 0xe:
      if ((sVar1 < sVar8) && (local_c != 0x22)) {
        puVar6 = FUN_00402120(param_1,0x22);
        return puVar6;
      }
      break;
    case 0xf:
    case 0x10:
      if (sVar1 < sVar8) {
        *(short *)((int)param_1 + 0x4a) = *(short *)(param_1 + 0x12) / 2;
        FUN_00402ba0((undefined4 *)&DAT_0040c750);
        puVar6 = FUN_00402120(param_1,0x21);
        return puVar6;
      }
    }
    break;
  case 4:
  case 9:
    break;
  case 5:
  case 6:
  case 7:
  case 8:
    if (param_2 == DAT_0040c72c) {
      if (iVar5 != 0) {
        FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x959);
      }
      FUN_00402ba0((undefined4 *)&DAT_0040c6e0);
      if ((*(byte *)(param_2 + 0x13) & 1) != 0) {
        param_2 = FUN_00402220(param_2);
      }
      FUN_00401350((int)param_2);
      if (param_1[3] == 0) {
        FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x95c);
      }
      *(undefined4 *)(param_1[3] + 0x10) = 0x32;
      *(undefined2 *)((int)param_1 + 0x46) = 0;
      *(undefined2 *)(param_1[3] + 0x1a) = 0;
      *(undefined2 *)(param_1 + 0x12) = 0;
      *(undefined2 *)(param_1[3] + 0x1c) = 0;
      *(undefined4 *)(param_1[3] + 0x20) = DAT_0040c698;
      puVar6 = FUN_00402120(param_1,0x32);
      return puVar6;
    }
    break;
  case 10:
    *(undefined2 *)((int)param_1 + 0x46) = 0;
    puVar6 = FUN_00402120(param_1,0x3c);
    return puVar6;
  default:
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x948);
  }
  puVar6 = FUN_00402120(param_1,local_c);
  return puVar6;
}

