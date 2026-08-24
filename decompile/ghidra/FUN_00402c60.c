/* FUN_00402c60 @ 0x00402c60 size=452 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00402c60(int param_1,short param_2,short param_3)

{
  short sVar1;
  short sVar2;
  int iVar3;
  ushort uVar4;
  
  if (param_1 == DAT_0040c72c) {
    sVar1 = *(short *)(param_1 + 0x40);
    sVar2 = *(short *)(param_1 + 0x42);
    if (*(int *)(param_1 + 0x18) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x6fc);
    }
    if (DAT_0040c95c == 0) {
      if ((param_3 < 0x281) && (0x280 < sVar2)) {
        iVar3 = FUN_00402e30((int)sVar1,(int)param_2,(int)sVar2,(int)param_3,0x280);
        if ((-0x241 < (short)iVar3) && ((short)iVar3 < -0x13f)) {
          DAT_0040c95c = 1;
          DAT_0040c948 = FUN_00402e30(DAT_0040c698,DAT_0040c708,(int)sVar2,(int)param_3,0x280);
          DAT_0040c944 = DAT_0040c948 - DAT_0040c698;
          DAT_0040c6f8 = DAT_0040c94c;
        }
      }
    }
    else {
      DAT_0040c944 = DAT_0040c698 - DAT_0040c948;
      if (0x21c0 < sVar2) {
        iVar3 = FUN_00402e30(DAT_0040c698,DAT_0040c708,(int)sVar2,(int)param_3,0x21c0);
        DAT_0040c95c = 0;
        DAT_0040c944 = iVar3 - DAT_0040c948;
        _DAT_0040c964 = 1;
        FUN_00402e80();
        FUN_00402ec0(&DAT_0040c0d8,DAT_0040c944,1);
        return;
      }
      if (sVar2 < 0x281) {
        DAT_0040c95c = 0;
        return;
      }
      if (*(short *)((int)DAT_0040c6f8 + 0x16) < sVar2) {
        uVar4 = 0x19;
        iVar3 = FUN_00402e30((int)sVar1,(int)param_2,(int)sVar2,(int)param_3,
                             (int)*(short *)((int)DAT_0040c6f8 + 0x16));
        if (((*(short *)(DAT_0040c6f8 + 2) == 0x17) && (*(short *)(DAT_0040c6f8 + 5) < (short)iVar3)
            ) || ((*(short *)(DAT_0040c6f8 + 2) == 0x18 &&
                  ((short)iVar3 < *(short *)(DAT_0040c6f8 + 5))))) {
          uVar4 = 0x1a;
          DAT_0040c948 = DAT_0040c948 + -5000;
        }
        FUN_00403130(DAT_0040c6f8,uVar4);
        DAT_0040c6f8 = DAT_0040c6f8 + 9;
        return;
      }
    }
  }
  return;
}

