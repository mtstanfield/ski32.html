/* FUN_004051e0 @ 0x004051e0 size=226 */

void FUN_004051e0(void)

{
  undefined4 *puVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  
  sVar4 = DAT_0040c5f0._2_2_;
  sVar2 = *(short *)(DAT_0040c5f8 + 0x35a);
  sVar5 = 0;
  sVar3 = DAT_0040c5f0._2_2_;
  puVar1 = FUN_004026a0(0x11,0x35);
  sVar2 = (short)(-0x28 - (int)sVar2 / 2);
  FUN_00402390(puVar1,sVar2,sVar3,sVar5);
  sVar3 = 0;
  sVar4 = sVar4 + *(short *)(DAT_0040c5f8 + 0x36c) + 4;
  puVar1 = FUN_004026a0(0x11,0x36);
  FUN_00402390(puVar1,sVar2,sVar4,sVar3);
  sVar2 = *(short *)(DAT_0040c5f8 + 0x37a);
  if (*(short *)(DAT_0040c5f8 + 0x37a) <= *(short *)(DAT_0040c5f8 + 0x38a)) {
    sVar2 = *(short *)(DAT_0040c5f8 + 0x38a);
  }
  sVar4 = *(short *)(DAT_0040c5f8 + 0x37c);
  sVar5 = 0;
  sVar3 = sVar4;
  puVar1 = FUN_004026a0(0x11,0x37);
  FUN_00402390(puVar1,sVar2,sVar3,sVar5);
  sVar3 = 0;
  sVar4 = sVar4 + *(short *)(DAT_0040c5f8 + 0x38c) + 4;
  puVar1 = FUN_004026a0(0x11,0x38);
  FUN_00402390(puVar1,sVar2,sVar4,sVar3);
  return;
}

