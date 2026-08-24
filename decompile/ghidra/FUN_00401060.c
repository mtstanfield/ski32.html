/* FUN_00401060 @ 0x00401060 size=476 */

void __fastcall FUN_00401060(HDC param_1,int *param_2)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  
  if (param_1 == (HDC)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4f8);
  }
  puVar8 = DAT_0040c618;
  if (param_2 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x4f9);
    puVar8 = DAT_0040c618;
  }
  for (; puVar6 = DAT_0040c618, puVar7 = DAT_0040c618, puVar8 != (undefined4 *)0x0;
      puVar8 = (undefined4 *)*puVar8) {
    if (((((*(byte *)(puVar8 + 0x13) & 0xb) == 0) && (iVar5 = puVar8[1], iVar5 != 0)) &&
        (uVar4 = *(uint *)(iVar5 + 0x4c), (uVar4 & 1) != 0)) &&
       (((uVar4 & 2) != 0 && (*(short *)(puVar8 + 4) == *(short *)(iVar5 + 0x10))))) {
      if ((uVar4 & 4) == 0) {
        piVar3 = FUN_00401410(iVar5);
      }
      else {
        piVar3 = (int *)(iVar5 + 0x20);
      }
      if ((*(byte *)(puVar8 + 0x13) & 4) == 0) {
        piVar1 = FUN_00401410((int)puVar8);
      }
      else {
        piVar1 = puVar8 + 8;
      }
      iVar2 = FUN_004012f0(piVar1,piVar3);
      if (iVar2 != 0) {
        puVar8[0x13] = puVar8[0x13] | 1;
        *(uint *)(iVar5 + 0x4c) = *(uint *)(iVar5 + 0x4c) & 0xfffffffe;
        FUN_00401350(iVar5);
      }
    }
  }
  for (; DAT_0040c618 = puVar7, puVar6 != (undefined4 *)0x0; puVar6 = (undefined4 *)*puVar6) {
    uVar4 = puVar6[0x13];
    if ((uVar4 & 8) == 0) {
      if ((uVar4 & 4) == 0) {
        piVar3 = FUN_00401410((int)puVar6);
      }
      else {
        piVar3 = puVar6 + 8;
      }
      uVar4 = FUN_00401290(piVar3,param_2);
      puVar6[0x13] = (uVar4 & 1) << 4 | puVar6[0x13] & 0xffffffef;
      if ((uVar4 & 1) != 0) {
        puVar6[0xc] = *piVar3;
        puVar6[0xd] = piVar3[1];
        puVar6[0xe] = piVar3[2];
        iVar5 = piVar3[3];
        puVar6[2] = 0;
        puVar6[0xf] = iVar5;
      }
    }
    else {
      puVar6[0x13] = uVar4 & 0xffffffef;
    }
    puVar7 = DAT_0040c618;
  }
  puVar8 = puVar7;
  if (puVar7 != (undefined4 *)0x0) {
    do {
      if ((*(byte *)(puVar8 + 0x13) & 0x10) != 0) {
        puVar7 = (undefined4 *)puVar8[1];
        puVar6 = DAT_0040c618;
        if (puVar7 == (undefined4 *)0x0) goto LAB_004011cd;
        if ((*(byte *)(puVar7 + 0x13) & 0x10) == 0) goto LAB_004011cd;
        iVar5 = FUN_00401290(puVar8 + 0xc,puVar7 + 0xc);
        puVar6 = DAT_0040c618;
        if (iVar5 == 0) goto LAB_004011cd;
        do {
          FUN_00401a60((int)puVar8,(int)puVar7);
          puVar6 = DAT_0040c618;
LAB_004011cd:
          while( true ) {
            puVar7 = DAT_0040c618;
            if ((puVar6 == (undefined4 *)0x0) || (puVar8 == puVar6)) goto LAB_004011f2;
            if (((*(byte *)(puVar6 + 0x13) & 0x10) != 0) &&
               (iVar5 = FUN_00401290(puVar8 + 0xc,puVar6 + 0xc), puVar7 = puVar6, iVar5 != 0))
            break;
            puVar6 = (undefined4 *)*puVar6;
          }
        } while( true );
      }
LAB_004011f2:
      puVar8 = (undefined4 *)*puVar8;
    } while (puVar8 != (undefined4 *)0x0);
    for (; puVar8 = DAT_0040c618, puVar7 != (undefined4 *)0x0; puVar7 = (undefined4 *)*puVar7) {
      if ((*(byte *)(puVar7 + 0x13) & 0x10) != 0) {
        FUN_00401540(param_1,(int)puVar7);
      }
    }
  }
  for (; puVar8 != (undefined4 *)0x0; puVar8 = (undefined4 *)*puVar8) {
    if ((*(byte *)(puVar8 + 0x13) & 2) != 0) {
      FUN_00401350((int)puVar8);
    }
  }
  FUN_00401390();
  return;
}

