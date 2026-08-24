/* FUN_00405ab0 @ 0x00405ab0 size=996 */

undefined4 __fastcall FUN_00405ab0(HDC param_1)

{
  int iVar1;
  int iVar2;
  HANDLE pvVar3;
  HBITMAP pHVar4;
  HGDIOBJ pvVar5;
  HDC hdc;
  HDC pHVar6;
  short sVar7;
  undefined4 *puVar8;
  short sVar9;
  uint uVar10;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  undefined1 local_18 [4];
  int local_14;
  int local_10;
  
  *DAT_0040c5f8 = 0;
  DAT_0040c5f8[1] = 0;
  local_34 = 0;
  local_20 = 0;
  *(undefined2 *)(DAT_0040c5f8 + 2) = 0;
  local_2c = 0;
  local_30 = 0;
  *(undefined2 *)((int)DAT_0040c5f8 + 10) = 0;
  local_24 = 0;
  local_28 = 0;
  *(undefined2 *)(DAT_0040c5f8 + 3) = 0;
  uVar10 = 1;
  *(undefined2 *)((int)DAT_0040c5f8 + 0xe) = 0;
  do {
    pvVar3 = (HANDLE)FUN_00405ea0(uVar10);
    if (pvVar3 == (HANDLE)0x0) {
      return 0;
    }
    GetObjectA(pvVar3,0x18,local_18);
    if ((short)(ushort)local_34 < local_14) {
      local_34 = local_14;
    }
    if ((short)local_20 < local_10) {
      local_20 = local_10;
    }
    if (local_14 < 0x21) {
      local_30 = local_30 + local_10;
    }
    else {
      local_2c = local_2c + local_10;
    }
    DeleteObject(pvVar3);
    uVar10 = uVar10 + 1;
  } while ((ushort)uVar10 < 0x5a);
  DAT_0040c710 = CreateCompatibleDC(param_1);
  if (DAT_0040c710 == (HDC)0x0) {
    return 0;
  }
  pHVar4 = CreateCompatibleBitmap(param_1,0x20,(int)(short)local_30);
  if (pHVar4 == (HBITMAP)0x0) {
    return 0;
  }
  DAT_0040c620 = SelectObject(DAT_0040c710,pHVar4);
  if (DAT_0040c620 == (HGDIOBJ)0x0) {
    DeleteObject(pHVar4);
    return 0;
  }
  DAT_0040c6a4 = CreateCompatibleDC(param_1);
  if (DAT_0040c6a4 == (HDC)0x0) {
    return 0;
  }
  pHVar4 = CreateBitmap(0x20,(int)(short)local_30,1,1,(void *)0x0);
  if (pHVar4 == (HBITMAP)0x0) {
    return 0;
  }
  DAT_0040c6d4 = SelectObject(DAT_0040c6a4,pHVar4);
  if (DAT_0040c6d4 == (HGDIOBJ)0x0) {
    DeleteObject(pHVar4);
    return 0;
  }
  DAT_0040c730 = CreateCompatibleDC(param_1);
  if (DAT_0040c730 != (HDC)0x0) {
    pHVar4 = CreateCompatibleBitmap(param_1,(int)(short)(ushort)local_34,(int)(short)local_2c);
    if (pHVar4 == (HBITMAP)0x0) {
      return 0;
    }
    DAT_0040c644 = SelectObject(DAT_0040c730,pHVar4);
    if (DAT_0040c644 == (HGDIOBJ)0x0) {
      DeleteObject(pHVar4);
      return 0;
    }
    DAT_0040c6ec = CreateCompatibleDC(param_1);
    if (DAT_0040c6ec == (HDC)0x0) {
      return 0;
    }
    pHVar4 = CreateBitmap((int)(short)(ushort)local_34,(int)(short)local_2c,1,1,(void *)0x0);
    if (pHVar4 != (HBITMAP)0x0) {
      DAT_0040c75c = SelectObject(DAT_0040c6ec,pHVar4);
      if (DAT_0040c75c == (HGDIOBJ)0x0) {
        DeleteObject(pHVar4);
        return 0;
      }
      DAT_0040c5ec = CreateCompatibleDC(param_1);
      uVar10 = 1;
      do {
        puVar8 = DAT_0040c5f8 + (uVar10 & 0xffff) * 4;
        pvVar3 = (HANDLE)FUN_00405ea0(uVar10);
        if (pvVar3 == (HANDLE)0x0) {
          return 0;
        }
        GetObjectA(pvVar3,0x18,local_18);
        sVar9 = (short)local_14;
        *(short *)((int)puVar8 + 10) = sVar9;
        *(short *)(puVar8 + 3) = (short)local_10;
        *(short *)((int)puVar8 + 0xe) = (short)local_10 * sVar9;
        if (sVar9 < 0x21) {
          iVar1 = local_28;
          iVar2 = local_24 + local_10;
          sVar7 = (short)local_24;
        }
        else {
          iVar1 = local_28 + local_10;
          iVar2 = local_24;
          sVar7 = (short)local_28;
        }
        local_24 = iVar2;
        local_28 = iVar1;
        *(short *)(puVar8 + 2) = sVar7;
        pvVar5 = SelectObject(DAT_0040c5ec,pvVar3);
        hdc = DAT_0040c730;
        if (sVar9 < 0x21) {
          hdc = DAT_0040c710;
        }
        *puVar8 = hdc;
        pHVar6 = DAT_0040c6ec;
        if (sVar9 < 0x21) {
          pHVar6 = DAT_0040c6a4;
        }
        puVar8[1] = pHVar6;
        BitBlt(hdc,0,(int)sVar7,local_14,local_10,DAT_0040c5ec,0,0,0xcc0020);
        BitBlt((HDC)puVar8[1],0,(int)sVar7,local_14,local_10,DAT_0040c5ec,0,0,0x330008);
        pvVar5 = SelectObject(DAT_0040c5ec,pvVar5);
        DeleteObject(pvVar5);
        uVar10 = uVar10 + 1;
      } while ((ushort)uVar10 < 0x5a);
      DAT_0040c690 = ((ushort)local_34 & 0xffc0) + 0x40;
      DAT_0040c6e8 = ((ushort)local_20 & 0xffc0) + 0x40;
      pHVar4 = CreateCompatibleBitmap(param_1,(int)DAT_0040c690,(int)DAT_0040c6e8);
      if (pHVar4 != (HBITMAP)0x0) {
        DAT_0040c614 = SelectObject(DAT_0040c5ec,pHVar4);
        if (DAT_0040c614 == (HGDIOBJ)0x0) {
          DeleteObject(pHVar4);
          return 0;
        }
        return 1;
      }
      return 0;
    }
    return 0;
  }
  return 0;
}

