/* FUN_00406550 @ 0x00406550 size=136 */

void __fastcall FUN_00406550(short param_1,short param_2)

{
  char cVar1;
  uint uVar2;
  undefined3 extraout_var;
  short sVar3;
  short sVar4;
  
  if (((DAT_0040c760 != 0) &&
      (((param_1 != DAT_0040c700 || (param_2 != DAT_0040c70c)) &&
       (DAT_0040c72c != (undefined4 *)0x0)))) &&
     ((DAT_0040c72c[7] != 0xb && (DAT_0040c72c[7] != 0x11)))) {
    sVar3 = param_1 - (short)DAT_0040c704;
    sVar4 = param_2 - (short)DAT_0040c5fc;
    if (*(short *)(DAT_0040c72c + 0x11) == 0) {
      uVar2 = FUN_004065e0(sVar3,sVar4);
    }
    else {
      cVar1 = FUN_00406670(sVar3,sVar4);
      uVar2 = CONCAT31(extraout_var,cVar1);
    }
    FUN_00402120(DAT_0040c72c,uVar2);
  }
  DAT_0040c700 = param_1;
  DAT_0040c70c = param_2;
  DAT_0040c760 = 1;
  return;
}

