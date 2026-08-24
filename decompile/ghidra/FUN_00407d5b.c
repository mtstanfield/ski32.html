/* FUN_00407d5b @ 0x00407d5b size=409 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl FUN_00407d5b(int param_1)

{
  BYTE *pBVar1;
  byte *pbVar2;
  byte bVar3;
  byte bVar4;
  UINT CodePage;
  UINT *pUVar5;
  BOOL BVar6;
  uint uVar7;
  BYTE *pBVar8;
  int iVar9;
  byte *pbVar10;
  int iVar11;
  byte *pbVar12;
  undefined4 *puVar13;
  _cpinfo local_1c;
  uint local_8;
  
  CodePage = FUN_00407ef4(param_1);
  if (CodePage == DAT_0040c984) {
    return 0;
  }
  if (CodePage != 0) {
    iVar11 = 0;
    pUVar5 = &DAT_0040c4d0;
    do {
      if (*pUVar5 == CodePage) {
        puVar13 = &DAT_0040caa0;
        for (iVar9 = 0x40; iVar9 != 0; iVar9 = iVar9 + -1) {
          *puVar13 = 0;
          puVar13 = puVar13 + 1;
        }
        local_8 = 0;
        iVar11 = iVar11 * 0x30;
        *(undefined1 *)puVar13 = 0;
        pbVar12 = (byte *)(iVar11 + 0x40c4e0);
        do {
          bVar3 = *pbVar12;
          pbVar10 = pbVar12;
          while ((bVar3 != 0 && (bVar3 = pbVar10[1], bVar3 != 0))) {
            uVar7 = (uint)*pbVar10;
            if (uVar7 <= bVar3) {
              bVar4 = (&DAT_0040c4c8)[local_8];
              do {
                pbVar2 = (byte *)((int)&DAT_0040caa0 + uVar7 + 1);
                *pbVar2 = *pbVar2 | bVar4;
                uVar7 = uVar7 + 1;
              } while (uVar7 <= bVar3);
            }
            pbVar10 = pbVar10 + 2;
            bVar3 = *pbVar10;
          }
          local_8 = local_8 + 1;
          pbVar12 = pbVar12 + 8;
        } while (local_8 < 4);
        _DAT_0040c99c = 1;
        DAT_0040c984 = CodePage;
        DAT_0040cba4 = FUN_00407f3e(CodePage);
        DAT_0040c990 = *(undefined4 *)(iVar11 + 0x40c4d4);
        DAT_0040c994 = *(undefined4 *)(iVar11 + 0x40c4d8);
        DAT_0040c998 = *(undefined4 *)(iVar11 + 0x40c4dc);
        goto LAB_00407ee3;
      }
      pUVar5 = pUVar5 + 0xc;
      iVar11 = iVar11 + 1;
    } while ((int)pUVar5 < 0x40c5c0);
    BVar6 = GetCPInfo(CodePage,&local_1c);
    if (BVar6 == 1) {
      puVar13 = &DAT_0040caa0;
      DAT_0040c984 = CodePage;
      for (iVar11 = 0x40; iVar11 != 0; iVar11 = iVar11 + -1) {
        *puVar13 = 0;
        puVar13 = puVar13 + 1;
      }
      *(undefined1 *)puVar13 = 0;
      DAT_0040cba4 = 0;
      if (local_1c.MaxCharSize < 2) {
        _DAT_0040c99c = 0;
      }
      else {
        if (local_1c.LeadByte[0] != '\0') {
          pBVar8 = local_1c.LeadByte + 1;
          do {
            bVar3 = *pBVar8;
            if (bVar3 == 0) break;
            for (uVar7 = (uint)pBVar8[-1]; uVar7 <= bVar3; uVar7 = uVar7 + 1) {
              pbVar12 = (byte *)((int)&DAT_0040caa0 + uVar7 + 1);
              *pbVar12 = *pbVar12 | 4;
            }
            pBVar1 = pBVar8 + 1;
            pBVar8 = pBVar8 + 2;
          } while (*pBVar1 != 0);
        }
        uVar7 = 1;
        do {
          pbVar12 = (byte *)((int)&DAT_0040caa0 + uVar7 + 1);
          *pbVar12 = *pbVar12 | 8;
          uVar7 = uVar7 + 1;
        } while (uVar7 < 0xff);
        DAT_0040cba4 = FUN_00407f3e(CodePage);
        _DAT_0040c99c = 1;
      }
      DAT_0040c990 = 0;
      DAT_0040c994 = 0;
      DAT_0040c998 = 0;
      goto LAB_00407ee3;
    }
    if (DAT_0040c904 == 0) {
      return 0xffffffff;
    }
  }
  FUN_00407f71();
LAB_00407ee3:
  FUN_00407f9a();
  return 0;
}

