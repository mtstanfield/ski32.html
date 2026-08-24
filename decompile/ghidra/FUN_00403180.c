/* FUN_00403180 @ 0x00403180 size=207 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00403180(int param_1,short param_2,short param_3)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  if (param_1 == DAT_0040c72c) {
    sVar1 = *(short *)(param_1 + 0x40);
    sVar2 = *(short *)(param_1 + 0x42);
    if (*(int *)(param_1 + 0x18) != 0) {
      FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x72f);
    }
    if (DAT_0040c954 == 0) {
      if ((param_3 < 0x281) && (0x280 < sVar2)) {
        iVar3 = FUN_00402e30((int)sVar1,(int)param_2,(int)sVar2,(int)param_3,0x280);
        if ((-0xa1 < (short)iVar3) && ((short)iVar3 < 0xa1)) {
          DAT_0040c954 = 1;
        }
      }
    }
    else {
      if (0x4100 < sVar2) {
        DAT_0040c954 = 0;
        _DAT_0040c968 = 1;
        FUN_00402e80();
        FUN_00402ec0(&DAT_0040c0f4,DAT_0040c6a8,0);
        return;
      }
      if (sVar2 < 0x281) {
        DAT_0040c954 = 0;
        return;
      }
    }
  }
  return;
}

