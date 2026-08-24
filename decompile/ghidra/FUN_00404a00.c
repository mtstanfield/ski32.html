/* FUN_00404a00 @ 0x00404a00 size=100 */

void FUN_00404a00(void)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = 0;
  DAT_0040c618 = 0;
  DAT_0040c744 = DAT_0040c648;
  uVar2 = 1;
  uVar1 = 0;
  do {
    uVar3 = uVar3 + 1;
    *(uint *)(uVar1 * 0x50 + DAT_0040c648) = uVar2 * 0x50 + DAT_0040c648;
    uVar1 = uVar3 & 0xffff;
    uVar2 = uVar1 + 1;
  } while (uVar2 < 100);
  *(undefined4 *)((uVar3 & 0xffff) * 0x50 + DAT_0040c648) = 0;
  return;
}

