/* FUN_00404130 @ 0x00404130 size=138 */

int __fastcall FUN_00404130(int *param_1)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  int local_10 [4];
  
  if (param_1 == (int *)0x0) {
    FUN_00401240(s_V__hack_ski32_ski2_c_0040c090,0xa2c);
  }
  if (*param_1 == 0) {
    iVar2 = param_1[6];
    sVar1 = *(short *)((int)param_1 + 0x16);
    iVar3 = param_1[5];
    FUN_004014b0(local_10,param_1[1],(short)iVar3,sVar1,(short)iVar2);
    iVar4 = FUN_00401290(local_10,(int *)&DAT_0040c680);
    if (iVar4 != 0) {
      if (*(ushort *)(param_1 + 2) == 0) {
        puVar5 = FUN_004020d0(param_1[3],param_1[4]);
      }
      else {
        puVar5 = FUN_004026a0(param_1[3],*(ushort *)(param_1 + 2));
      }
      if (puVar5 != (undefined4 *)0x0) {
        puVar5 = FUN_00402390(puVar5,(short)iVar3,sVar1,(short)iVar2);
        *param_1 = (int)puVar5;
        puVar5[3] = param_1;
      }
    }
  }
  return *param_1;
}

