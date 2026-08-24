/* FUN_00401540 @ 0x00401540 size=1058 */

void __fastcall FUN_00401540(HDC param_1,int param_2)

{
  int *piVar1;
  short sVar2;
  short sVar3;
  undefined4 *puVar4;
  bool bVar5;
  int iVar6;
  uint uVar7;
  int *piVar8;
  short sVar9;
  int y1;
  ushort uVar10;
  short sVar11;
  ushort uVar12;
  short sVar13;
  undefined4 unaff_ESI;
  int iVar14;
  short sVar15;
  undefined4 unaff_EDI;
  HDC hdcSrc;
  DWORD rop;
  int local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  short local_10;
  int local_c;
  undefined4 local_8;
  int *local_4;
  
  sVar9 = *(short *)(param_2 + 0x30);
  local_14 = CONCAT22((short)((uint)unaff_EDI >> 0x10),sVar9);
  sVar2 = *(short *)(param_2 + 0x34);
  uVar10 = *(short *)(param_2 + 0x38) - sVar9;
  uVar12 = *(short *)(param_2 + 0x3c) - sVar2;
  local_18 = CONCAT22((short)((uint)unaff_ESI >> 0x10),uVar12);
  local_c = 0;
  local_20 = param_2;
  if (param_2 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x46d);
  }
  if (param_1 == (HDC)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x46e);
  }
  if ((*(byte *)(param_2 + 0x4c) & 0x10) == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x46f);
  }
  iVar6 = param_2;
  if (param_2 != 0) {
    while ((*(uint *)(iVar6 + 0x4c) & 1) << 1 != (*(uint *)(iVar6 + 0x4c) & 2)) {
      iVar6 = *(int *)(iVar6 + 8);
      if (iVar6 == 0) {
        return;
      }
    }
    local_1c = 0;
    iVar6 = FUN_00401970(uVar10,uVar12);
    if (iVar6 == 0) {
      PatBlt(param_1,(int)sVar9,(int)sVar2,(int)(short)uVar10,(int)(short)uVar12,0xff0062);
      do {
        uVar7 = *(uint *)(param_2 + 0x4c);
        if (((uVar7 & 1) == 0) || ((uVar7 & 2) == 0)) {
          if (((uVar7 & 1) == 0) && ((uVar7 & 2) == 0)) {
            puVar4 = *(undefined4 **)(param_2 + 0x14);
            if ((uVar7 & 4) == 0) {
              piVar8 = FUN_00401410(param_2);
            }
            else {
              piVar8 = (int *)(param_2 + 0x20);
            }
            BitBlt(param_1,*piVar8,piVar8[1],(int)*(short *)((int)puVar4 + 10),
                   (int)*(short *)(puVar4 + 3),(HDC)*puVar4,0,(int)*(short *)(puVar4 + 2),0x8800c6);
            uVar7 = *(uint *)(param_2 + 0x4c) | 1;
            goto LAB_00401684;
          }
        }
        else {
          uVar7 = uVar7 & 0xfffffffe;
LAB_00401684:
          *(uint *)(param_2 + 0x4c) = uVar7;
        }
        param_2 = *(int *)(param_2 + 8);
        if (param_2 == 0) {
          return;
        }
      } while( true );
    }
    do {
      iVar14 = 0;
      local_24 = 0;
      iVar6 = param_2;
      piVar8 = &local_20;
      if (param_2 == 0) break;
      do {
        uVar7 = *(uint *)(param_2 + 0x4c);
        piVar1 = (int *)(param_2 + 8);
        if ((uVar7 & 2) == 0) {
          if ((uVar7 & 0x40) == 0) {
            sVar9 = 0;
          }
          else {
            uVar7 = *(uint *)(param_2 + 0x14);
            sVar9 = *(short *)(uVar7 + 0xc);
          }
          sVar9 = *(short *)(param_2 + 0x42) - sVar9;
          if ((iVar14 == 0) || (bVar5 = sVar9 < (short)local_8, bVar5)) {
            iVar14 = param_2;
            local_24 = param_2;
            local_8 = CONCAT22((short)(uVar7 >> 0x10),sVar9);
            local_4 = piVar8;
          }
        }
        else {
          if ((uVar7 & 1) != 0) {
            local_c = 1;
            *(uint *)(param_2 + 0x4c) = uVar7 & 0xfffffffe;
          }
          *piVar8 = *piVar1;
          iVar6 = local_20;
        }
        param_2 = *piVar1;
        piVar8 = piVar1;
      } while (param_2 != 0);
      param_2 = iVar6;
      if (iVar14 != 0) {
        puVar4 = *(undefined4 **)(iVar14 + 0x14);
        sVar9 = *(short *)(puVar4 + 3);
        sVar3 = *(short *)((int)puVar4 + 10);
        local_10 = *(short *)(puVar4 + 2);
        if ((*(byte *)(iVar14 + 0x4c) & 4) == 0) {
          piVar8 = FUN_00401410(iVar14);
        }
        else {
          piVar8 = (int *)(iVar14 + 0x20);
        }
        iVar6 = (int)sVar3;
        sVar11 = (short)*piVar8 - (short)local_14;
        sVar15 = (short)piVar8[1] - sVar2;
        if (piVar8[2] - *piVar8 != iVar6) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b3);
        }
        iVar14 = (int)sVar9;
        if (piVar8[3] - piVar8[1] != iVar14) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b4);
        }
        if (sVar11 < 0) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b5);
        }
        if (sVar15 < 0) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b6);
        }
        if ((short)uVar10 < sVar3) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b7);
        }
        sVar13 = (short)local_18;
        if (sVar13 < sVar9) {
          FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4b8);
        }
        if (local_1c == 0) {
          local_1c = 1;
          if ((((0 < sVar11) || (0 < sVar15)) || (sVar3 < (short)uVar10)) || (sVar9 < sVar13)) {
            PatBlt(DAT_0040c5ec,0,0,(int)(short)uVar10,(int)sVar13,0xff0062);
          }
          y1 = (int)local_10;
          hdcSrc = (HDC)*puVar4;
          rop = 0xcc0020;
        }
        else {
          y1 = (int)local_10;
          BitBlt(DAT_0040c5ec,(int)sVar11,(int)sVar15,iVar6,iVar14,(HDC)puVar4[1],0,y1,0xee0086);
          hdcSrc = (HDC)*puVar4;
          rop = 0x8800c6;
        }
        BitBlt(DAT_0040c5ec,(int)sVar11,(int)sVar15,iVar6,iVar14,hdcSrc,0,y1,rop);
        *(uint *)(local_24 + 0x4c) = *(uint *)(local_24 + 0x4c) | 1;
        *local_4 = *(int *)(local_24 + 8);
        param_2 = local_20;
      }
    } while (param_2 != 0);
    if (local_1c != 0) {
      BitBlt(param_1,(int)(short)local_14,(int)sVar2,(int)(short)uVar10,(int)(short)local_18,
             DAT_0040c5ec,0,0,0xcc0020);
      return;
    }
    if (local_c != 0) {
      PatBlt(param_1,(int)(short)local_14,(int)sVar2,(int)(short)uVar10,(int)(short)local_18,
             0xff0062);
    }
  }
  return;
}

