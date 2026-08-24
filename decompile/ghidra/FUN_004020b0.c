/* FUN_004020b0 @ 0x004020b0 size=23 */

undefined4 __fastcall FUN_004020b0(short param_1)

{
  uint uVar1;
  
  uVar1 = FUN_00406cda();
  return CONCAT22((short)((uint)((int)(short)uVar1 / (int)param_1) >> 0x10),(short)uVar1 % param_1);
}

