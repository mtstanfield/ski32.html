/* FUN_00403430 @ 0x00403430 size=259 */

int __fastcall FUN_00403430(int param_1,short *param_2)

{
  short sVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  int iVar5;
  
  sVar4 = *(short *)(param_1 + 0x46);
  sVar1 = *(short *)(param_1 + 0x48);
  if (param_1 == 0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x79f);
  }
  if (param_2 == (short *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7a0);
  }
  if (*(int *)(param_1 + 0x1c) != *(int *)(param_2 + 6)) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0x7a1);
  }
  uVar3 = param_2[4];
  if (uVar3 == 0) {
    if (sVar4 < 0) {
      uVar3 = 0xffff;
    }
    else {
      uVar3 = (ushort)(0 < sVar4);
    }
  }
  sVar4 = uVar3 * sVar4;
  if (sVar1 < 1) {
    iVar5 = 0;
  }
  else {
    iVar5 = (int)sVar1;
  }
  sVar2 = (short)((param_2[3] * iVar5) / 2);
  if (sVar2 < sVar4) {
    iVar5 = sVar4 + -2;
    if (sVar2 <= iVar5) {
LAB_004034f1:
      sVar2 = (short)iVar5;
    }
  }
  else {
    iVar5 = (int)sVar4 + (int)param_2[2];
    if (iVar5 <= sVar2) goto LAB_004034f1;
  }
  sVar4 = param_2[1];
  if (sVar4 < sVar1) {
    iVar5 = sVar1 + -2;
    if (iVar5 < sVar4) goto LAB_0040351c;
  }
  else {
    iVar5 = (int)*param_2 + (int)sVar1;
    if (sVar4 < iVar5) goto LAB_0040351c;
  }
  sVar4 = (short)iVar5;
LAB_0040351c:
  *(short *)(param_1 + 0x48) = sVar4;
  *(ushort *)(param_1 + 0x46) = uVar3 * sVar2;
  return param_1;
}

