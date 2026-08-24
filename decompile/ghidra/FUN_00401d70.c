/* FUN_00401d70 @ 0x00401d70 size=174 */

void __fastcall FUN_00401d70(int param_1,LPSTR param_2)

{
  int iVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  
  uVar6 = (param_1 % 1000 & 0xffffU) / 10;
  uVar5 = (param_1 / 1000) % 0x3c & 0xffff;
  iVar1 = (param_1 / 1000) / 0x3c;
  uVar3 = iVar1 % 0x3c & 0xffff;
  uVar4 = iVar1 / 0x3c & 0xffff;
  pcVar2 = FUN_00401cf0(0xb);
  wsprintfA(param_2,pcVar2,uVar4,uVar3,uVar5,uVar6);
  return;
}

