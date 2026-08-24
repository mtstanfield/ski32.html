/* FUN_00401e50 @ 0x00401e50 size=607 */

void FUN_00401e50(void)

{
  uint uVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  
  DAT_0040c714 = DAT_0040c714 - (short)DAT_0040c640;
  DAT_0040c5d8 = DAT_0040c5d8 - DAT_0040c5f0._2_2_;
  for (puVar8 = DAT_0040c618; puVar8 != (undefined4 *)0x0; puVar8 = (undefined4 *)*puVar8) {
    if ((*(byte *)(puVar8 + 0x13) & 10) == 0) {
      puVar8[0x13] = puVar8[0x13] & 0xffffffdf;
      if ((puVar8[3] == 0) && ((int)puVar8[6] < 0xb)) {
        FUN_004028e0(puVar8);
      }
      if (((puVar8[0x13] & 1) == 0) && (puVar8 != DAT_0040c72c)) {
        if ((puVar8[0x13] & 4) == 0) {
          piVar4 = FUN_00401410((int)puVar8);
        }
        else {
          piVar4 = puVar8 + 8;
        }
        iVar3 = FUN_00401290(piVar4,(int *)&DAT_0040c680);
        if (iVar3 == 0) {
          DAT_0040c6fc = DAT_0040c6fc - *(short *)(puVar8[5] + 0xe);
          FUN_00401350((int)puVar8);
        }
      }
    }
  }
  FUN_004046e0((uint *)&DAT_0040c630);
  FUN_004046e0((uint *)&DAT_0040c5e0);
  FUN_004046e0((uint *)&DAT_0040c658);
  FUN_004046e0((uint *)&DAT_0040c738);
  FUN_004040a0((uint *)&DAT_0040c720);
  FUN_00401390();
  for (puVar8 = DAT_0040c618; puVar8 != (undefined4 *)0x0; puVar8 = (undefined4 *)*puVar8) {
    if ((puVar8[0x13] & 2) == 0) {
      if ((puVar8[0x13] & 4) == 0) {
        piVar4 = FUN_00401410((int)puVar8);
      }
      else {
        piVar4 = puVar8 + 8;
      }
      iVar3 = puVar8[0x13];
      for (puVar2 = DAT_0040c618; (puVar2 != (undefined4 *)0x0 && (puVar8 != puVar2));
          puVar2 = (undefined4 *)*puVar2) {
        uVar1 = puVar2[0x13];
        if (((uVar1 & 2) == 0) && ((iVar3 << 0x1a < 0 || ((uVar1 & 0x20) != 0)))) {
          if ((uVar1 & 4) == 0) {
            piVar5 = FUN_00401410((int)puVar2);
          }
          else {
            piVar5 = puVar2 + 8;
          }
          iVar6 = FUN_00401290(piVar4,piVar5);
          if (((iVar6 != 0) && (FUN_00403a00(puVar8,puVar2), (*(byte *)(puVar8 + 0x13) & 8) == 0))
             && ((*(byte *)(puVar2 + 0x13) & 8) == 0)) {
            FUN_00403a00(puVar2,puVar8);
          }
        }
      }
    }
  }
  DAT_0040c714 = DAT_0040c714 + (short)DAT_0040c640;
  for (DAT_0040c5d8 = DAT_0040c5d8 + DAT_0040c5f0._2_2_; 0x3c < DAT_0040c5d8;
      DAT_0040c5d8 = DAT_0040c5d8 + -0x3c) {
    FUN_004025c0(3);
  }
  for (; DAT_0040c5d8 < -0x3c; DAT_0040c5d8 = DAT_0040c5d8 + 0x3c) {
    FUN_004025c0(2);
  }
  for (; 0x3c < DAT_0040c714; DAT_0040c714 = DAT_0040c714 + -0x3c) {
    FUN_004025c0(1);
  }
  for (; DAT_0040c714 < -0x3c; DAT_0040c714 = DAT_0040c714 + 0x3c) {
    FUN_004025c0(0);
  }
  uVar7 = FUN_004020b0(0x29a);
  if ((short)uVar7 != 0) {
    return;
  }
  puVar8 = FUN_004020d0(3,0x1f);
  FUN_00402350(puVar8,2);
  return;
}

