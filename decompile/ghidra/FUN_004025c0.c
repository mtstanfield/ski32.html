/* FUN_004025c0 @ 0x004025c0 size=213 */

undefined4 * __fastcall FUN_004025c0(int param_1)

{
  char cVar1;
  undefined3 extraout_var;
  undefined3 extraout_var_00;
  int iVar2;
  undefined4 *puVar3;
  uint uVar4;
  short local_8 [2];
  short local_4 [2];
  
  FUN_004024f0(param_1,local_4,local_8);
  if ((((local_4[0] < -0x240) || (-0x140 < local_4[0])) || (local_8[0] < 0x280)) ||
     (0x21c0 < local_8[0])) {
    if (((local_4[0] < 0x140) || (0x200 < local_4[0])) ||
       ((local_8[0] < 0x280 || (0x4100 < local_8[0])))) {
      if (((local_4[0] < -0xa0) || (0xa0 < local_4[0])) ||
         ((local_8[0] < 0x280 || (0x4100 < local_8[0])))) {
        iVar2 = FUN_004026f0();
      }
      else {
        iVar2 = FUN_004027e0();
      }
    }
    else {
      cVar1 = FUN_004027a0();
      iVar2 = CONCAT31(extraout_var_00,cVar1);
    }
  }
  else {
    cVar1 = FUN_00402770();
    iVar2 = CONCAT31(extraout_var,cVar1);
  }
  if (iVar2 != 0x12) {
    if (iVar2 < 0xb) {
      puVar3 = FUN_004020d0(iVar2,*(uint *)(&DAT_0040a22c + iVar2 * 4));
    }
    else {
      uVar4 = FUN_00402850(iVar2);
      puVar3 = FUN_004026a0(iVar2,(ushort)uVar4);
    }
    if (puVar3 != (undefined4 *)0x0) {
      puVar3 = FUN_00402390(puVar3,local_4[0],local_8[0],0);
      return puVar3;
    }
  }
  return (undefined4 *)0x0;
}

