/* FUN_004086d3 @ 0x004086d3 size=43 */

uint __cdecl FUN_004086d3(int param_1)

{
  uint uVar1;
  
  uVar1 = DAT_0040c980;
  while( true ) {
    if (DAT_0040c980 + DAT_0040c97c * 0x14 <= uVar1) {
      return 0;
    }
    if ((uint)(param_1 - *(int *)(uVar1 + 0xc)) < 0x100000) break;
    uVar1 = uVar1 + 0x14;
  }
  return uVar1;
}

