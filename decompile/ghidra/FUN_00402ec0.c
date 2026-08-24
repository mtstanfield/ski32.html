/* FUN_00402ec0 @ 0x00402ec0 size=619 */

void __fastcall FUN_00402ec0(LPCSTR param_1,int param_2,int param_3)

{
  byte bVar1;
  ushort uVar2;
  int iVar3;
  int *piVar4;
  char *pcVar5;
  void *extraout_ECX;
  void *this;
  uint uVar6;
  ushort uVar7;
  byte *pbVar8;
  ushort uVar9;
  byte *pbVar10;
  UINT uType;
  int local_128 [10];
  byte local_100;
  byte local_ff [255];
  
  uVar2 = 0;
  pbVar10 = &local_100;
  if (param_3 != 0) {
    param_2 = -param_2;
  }
  GetPrivateProfileStringA
            (&DAT_0040c080,param_1,&DAT_0040c788,(LPSTR)&local_100,0x100,s_entpack_ini_0040c084);
  this = extraout_ECX;
  while ((local_100 != 0 && (uVar2 < 10))) {
    bVar1 = *pbVar10;
    while (bVar1 == 0x20) {
      pbVar8 = pbVar10 + 1;
      pbVar10 = pbVar10 + 1;
      bVar1 = *pbVar8;
    }
    bVar1 = *pbVar10;
    pbVar8 = pbVar10;
    if (bVar1 != 0x20) {
      do {
        if (bVar1 == 0) break;
        bVar1 = pbVar8[1];
        pbVar8 = pbVar8 + 1;
      } while (bVar1 != 0x20);
      if (pbVar8 != pbVar10) {
        if (*pbVar8 != 0) {
          *pbVar8 = 0;
          pbVar8 = pbVar8 + 1;
        }
        iVar3 = FUN_00406cf8(this,pbVar10);
        this = (void *)(uint)uVar2;
        uVar2 = uVar2 + 1;
        local_128[(int)this] = iVar3;
        pbVar10 = pbVar8;
      }
    }
    local_100 = *pbVar10;
  }
  uVar7 = 0;
  uVar9 = 0;
  if (uVar2 != 0) {
    do {
      uVar7 = uVar9;
      if (local_128[uVar7] < param_2) break;
      uVar7 = uVar7 + 1;
      uVar9 = uVar7;
    } while (uVar7 < uVar2);
    if (9 < uVar7) goto LAB_00402fe2;
  }
  if (uVar2 == 10) {
    uVar2 = 9;
  }
  uVar9 = uVar2;
  if (uVar7 < uVar2) {
    piVar4 = local_128 + uVar2;
    iVar3 = (uint)uVar2 - (uint)uVar7;
    do {
      uVar9 = uVar9 - 1;
      *piVar4 = piVar4[-1];
      piVar4 = piVar4 + -1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
  }
  uVar2 = uVar2 + 1;
  local_128[uVar9] = param_2;
LAB_00402fe2:
  pbVar10 = &local_100;
  if (uVar2 != 0) {
    piVar4 = local_128;
    uVar6 = (uint)uVar2;
    do {
      iVar3 = wsprintfA((LPSTR)pbVar10,&DAT_0040c0ec,*piVar4);
      pbVar10 = pbVar10 + iVar3;
      piVar4 = piVar4 + 1;
      uVar6 = uVar6 - 1;
    } while (uVar6 != 0);
  }
  WritePrivateProfileStringA(&DAT_0040c080,param_1,(LPCSTR)&local_100,s_entpack_ini_0040c084);
  uVar9 = 0;
  pbVar10 = &local_100;
  if (uVar2 != 0) {
    piVar4 = local_128;
    do {
      if (uVar9 != 0) {
        *pbVar10 = 10;
        pbVar10 = pbVar10 + 1;
      }
      if (param_3 == 0) {
        uVar6 = wsprintfA((LPSTR)pbVar10,&DAT_0040c0e4,*piVar4);
      }
      else {
        uVar6 = FUN_00401d70(-*piVar4,(LPSTR)pbVar10);
        uVar6 = uVar6 & 0xffff;
      }
      pbVar10 = pbVar10 + uVar6;
      if (uVar9 == uVar7) {
        pcVar5 = FUN_00401cf0(0x10);
        iVar3 = wsprintfA((LPSTR)pbVar10,&DAT_0040c0e0,pcVar5);
        pbVar10 = pbVar10 + iVar3;
      }
      uVar9 = uVar9 + 1;
      piVar4 = piVar4 + 1;
    } while (uVar9 < uVar2);
  }
  if (uVar7 == 10) {
    iVar3 = wsprintfA((LPSTR)pbVar10,&DAT_0040c0dc);
    pbVar10 = pbVar10 + iVar3;
    if (param_3 == 0) {
      uVar6 = wsprintfA((LPSTR)pbVar10,&DAT_0040c0e4,param_2);
    }
    else {
      uVar2 = FUN_00401d70(-param_2,(LPSTR)pbVar10);
      uVar6 = (uint)uVar2;
    }
    pcVar5 = FUN_00401cf0(0x11);
    wsprintfA((LPSTR)(pbVar10 + uVar6),&DAT_0040c0e0,pcVar5);
  }
  uType = 0;
  pcVar5 = FUN_00401cf0(0xf);
  MessageBoxA(DAT_0040c6c8,(LPCSTR)&local_100,pcVar5,uType);
  return;
}

