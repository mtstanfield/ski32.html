/* FUN_004065e0 @ 0x004065e0 size=132 */

int __fastcall FUN_004065e0(short param_1,short param_2)

{
  ushort uVar1;
  
  if (0 < param_2) {
    if (param_1 == 0) {
      return 0;
    }
    uVar1 = (ushort)(((int)param_2 << 2) / (int)param_1);
    if ((short)uVar1 < -0xb) {
      return 0;
    }
    if ((short)uVar1 < -5) {
      return 1;
    }
    if ((short)uVar1 < -2) {
      return 2;
    }
    if (0x7fff < uVar1) {
      return 3;
    }
    if (0xb < (short)uVar1) {
      return 0;
    }
    if (5 < (short)uVar1) {
      return 4;
    }
    if (2 < (short)uVar1) {
      return 5;
    }
    if (0 < (short)uVar1) {
      return 6;
    }
  }
  return ((-1 < param_1) - 1 & 0xfffffffd) + 6;
}

