/* FUN_00404b50 @ 0x00404b50 size=1448 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00404b50(void)

{
  bool bVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined4 uVar4;
  short sVar5;
  undefined4 local_24 [2];
  short local_1c;
  undefined4 local_18;
  undefined4 local_14;
  short local_10;
  short local_e;
  undefined2 local_c;
  undefined2 local_a;
  undefined2 local_8;
  undefined2 local_6;
  
  local_6 = 0;
  local_8 = 0;
  local_a = 0;
  local_c = 0;
  FUN_00405100((undefined4 *)&DAT_0040c630);
  local_18 = 0x11;
  local_10 = ((short)DAT_0040c6b0 - (short)DAT_0040c704) + 0x3c + (short)DAT_0040c640;
  local_1c = 0x3d;
  if (local_10 < -0x140) {
    local_10 = -0x140;
  }
  local_e = ((short)DAT_0040c6bc - (short)DAT_0040c5fc) + DAT_0040c5f0._2_2_ + -0x3c;
  if (0x280 < local_e) {
    local_e = 0x208;
  }
  FUN_00405120((int *)&DAT_0040c630,local_24);
  local_1c = 0x39;
  local_10 = 0xfdc0;
  local_e = 0x280;
  FUN_00405120((int *)&DAT_0040c630,local_24);
  local_1c = 0x3a;
  local_10 = 0xfec0;
  FUN_00405120((int *)&DAT_0040c630,local_24);
  local_18 = 0xc;
  bVar1 = true;
  DAT_0040c94c = (undefined4 *)0x0;
  sVar5 = 0x3c0;
  do {
    local_1c = 0x18 - (ushort)bVar1;
    local_10 = (-(ushort)bVar1 & 0xffa0) - 400;
    bVar1 = !bVar1;
    local_e = sVar5;
    puVar2 = FUN_00405120((int *)&DAT_0040c630,local_24);
    if (DAT_0040c94c == (undefined4 *)0x0) {
      DAT_0040c94c = puVar2;
    }
    sVar5 = sVar5 + 0x140;
  } while (sVar5 < 0x21c0);
  local_18 = 0x11;
  local_1c = 0x3b;
  local_10 = 0xfdc0;
  local_e = 0x21c0;
  FUN_00405120((int *)&DAT_0040c630,local_24);
  local_1c = 0x3c;
  local_10 = 0xfec0;
  FUN_00405120((int *)&DAT_0040c630,local_24);
  FUN_00405100((undefined4 *)&DAT_0040c5e0);
  local_18 = 0x11;
  local_1c = 0x3e;
  local_10 = ((short)DAT_0040c6b8 - (short)DAT_0040c704) + -0x3c + (short)DAT_0040c640;
  if (0x140 < local_10) {
    local_10 = 0x140;
  }
  local_e = ((short)DAT_0040c6bc - (short)DAT_0040c5fc) + DAT_0040c5f0._2_2_ + -0x3c;
  if (0x280 < local_e) {
    local_e = 0x208;
  }
  FUN_00405120((int *)&DAT_0040c5e0,local_24);
  local_1c = 0x39;
  local_10 = 0x140;
  local_e = 0x280;
  FUN_00405120((int *)&DAT_0040c5e0,local_24);
  local_1c = 0x3a;
  local_10 = 0x200;
  FUN_00405120((int *)&DAT_0040c5e0,local_24);
  bVar1 = true;
  DAT_0040c950 = (undefined4 *)0x0;
  sVar5 = 0x410;
  do {
    local_18 = 0xc;
    local_1c = 0x18 - (ushort)bVar1;
    local_10 = (-(ushort)bVar1 & 0xffe0) + 0x1b0;
    bVar1 = !bVar1;
    local_e = sVar5;
    puVar2 = FUN_00405120((int *)&DAT_0040c5e0,local_24);
    if (DAT_0040c950 == (undefined4 *)0x0) {
      DAT_0040c950 = puVar2;
    }
    local_18 = 0xd;
    uVar3 = FUN_00402850(0xd);
    local_1c = (short)uVar3;
    uVar4 = FUN_004020b0(0x20);
    local_10 = (short)uVar4 + 400;
    FUN_004020b0(400);
    sVar5 = sVar5 + 400;
  } while (sVar5 < 0x4100);
  local_18 = 0x11;
  local_1c = 0x3b;
  local_10 = 0x140;
  local_e = 0x4100;
  FUN_00405120((int *)&DAT_0040c5e0,local_24);
  local_1c = 0x3c;
  local_10 = 0x200;
  FUN_00405120((int *)&DAT_0040c5e0,local_24);
  FUN_00405100((undefined4 *)&DAT_0040c658);
  local_18 = 0x11;
  local_1c = 0x3f;
  local_10 = 0;
  local_e = ((short)DAT_0040c6bc - (short)DAT_0040c5fc) + DAT_0040c5f0._2_2_ + -0x3c;
  if (0x280 < local_e) {
    local_e = 0x208;
  }
  FUN_00405120((int *)&DAT_0040c658,local_24);
  local_1c = 0x39;
  local_10 = 0xff60;
  local_e = 0x280;
  FUN_00405120((int *)&DAT_0040c658,local_24);
  local_1c = 0x3a;
  local_10 = 0xa0;
  FUN_00405120((int *)&DAT_0040c658,local_24);
  local_1c = 0x3b;
  local_10 = 0xff60;
  local_e = 0x4100;
  FUN_00405120((int *)&DAT_0040c658,local_24);
  local_1c = 0x3c;
  local_10 = 0xa0;
  FUN_00405120((int *)&DAT_0040c658,local_24);
  _DAT_0040c968 = 0;
  DAT_0040c954 = 0;
  FUN_00405100((undefined4 *)&DAT_0040c738);
  sVar5 = -0x400;
  do {
    local_18 = 0xd;
    local_1c = 0x40;
    local_10 = 0xff80;
    local_c = 0;
    local_6 = 0;
    local_8 = 0;
    local_a = 0;
    local_e = sVar5;
    FUN_00405120((int *)&DAT_0040c738,local_24);
    sVar5 = sVar5 + 0x800;
  } while (sVar5 < 0x5c01);
  FUN_00405100((undefined4 *)&DAT_0040c720);
  sVar5 = -0x400;
  do {
    local_18 = 4;
    local_1c = 0;
    local_6 = 0;
    local_a = 0;
    local_c = 0x20;
    local_e = sVar5;
    if (-0x400 < sVar5) {
      local_14 = 0x27;
      local_10 = 0xff90;
      local_8 = 0xfffe;
      FUN_00405120((int *)&DAT_0040c720,local_24);
    }
    if (sVar5 < 0x5c00) {
      local_14 = 0x29;
      local_10 = 0xff70;
      local_8 = 2;
      FUN_00405120((int *)&DAT_0040c720,local_24);
    }
    sVar5 = sVar5 + 0x800;
  } while (sVar5 < 0x5c01);
  local_18 = 7;
  local_14 = 0x2a;
  local_1c = 0;
  local_10 = 0xc144;
  local_c = 0;
  local_e = 0;
  local_6 = 0;
  local_8 = 0;
  local_a = 0;
  FUN_00405120((int *)&DAT_0040c720,local_24);
  local_18 = 8;
  local_10 = 0x3ebc;
  FUN_00405120((int *)&DAT_0040c720,local_24);
  local_18 = 5;
  local_10 = 0;
  local_e = 0xf7f4;
  FUN_00405120((int *)&DAT_0040c720,local_24);
  local_18 = 6;
  local_e = 0x7d3c;
  FUN_00405120((int *)&DAT_0040c720,local_24);
  return;
}

