/* FUN_00402470 @ 0x00402470 size=126 */

void __fastcall FUN_00402470(short param_1,short param_2)

{
  uint uVar1;
  undefined4 *puVar2;
  short sVar3;
  undefined4 *puVar4;
  int iVar5;
  short sVar6;
  
  sVar3 = (short)DAT_0040c640;
  sVar6 = param_2 - DAT_0040c5f0._2_2_;
  for (puVar2 = DAT_0040c618; puVar2 != (undefined4 *)0x0; puVar2 = (undefined4 *)*puVar2) {
    if (((puVar2 != DAT_0040c64c) && (uVar1 = puVar2[0x13], (uVar1 & 4) != 0)) && ((uVar1 & 2) == 0)
       ) {
      puVar4 = puVar2;
      if ((uVar1 & 1) != 0) {
        puVar4 = FUN_00402220(puVar2);
      }
      iVar5 = (int)(short)(param_1 - sVar3);
      puVar4[8] = puVar4[8] - iVar5;
      puVar4[10] = puVar4[10] - iVar5;
      puVar4[9] = puVar4[9] - (int)sVar6;
      puVar4[0xb] = puVar4[0xb] - (int)sVar6;
    }
  }
  DAT_0040c5f0._2_2_ = param_2;
  uVar1 = (uint)DAT_0040c640 >> 0x10;
  DAT_0040c640 = CONCAT22((short)uVar1,param_1);
  return;
}

