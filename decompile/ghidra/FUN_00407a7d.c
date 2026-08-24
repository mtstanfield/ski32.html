/* FUN_00407a7d @ 0x00407a7d size=339 */

void __cdecl FUN_00407a7d(DWORD param_1)

{
  undefined4 *puVar1;
  DWORD *pDVar2;
  DWORD DVar3;
  size_t sVar4;
  HANDLE hFile;
  int iVar5;
  uint *_Dest;
  undefined1 auStackY_1e3 [7];
  LPCVOID lpBuffer;
  LPOVERLAPPED lpOverlapped;
  uint local_1a8 [65];
  uint local_a4 [40];
  
  iVar5 = 0;
  pDVar2 = &DAT_0040c438;
  do {
    if (param_1 == *pDVar2) break;
    pDVar2 = pDVar2 + 2;
    iVar5 = iVar5 + 1;
  } while ((int)pDVar2 < 0x40c4c8);
  if (param_1 == (&DAT_0040c438)[iVar5 * 2]) {
    if ((DAT_0040c7a0 == 1) || ((DAT_0040c7a0 == 0 && (DAT_0040c174 == 1)))) {
      pDVar2 = &param_1;
      puVar1 = (undefined4 *)(iVar5 * 8 + 0x40c43c);
      lpOverlapped = (LPOVERLAPPED)0x0;
      sVar4 = _strlen((char *)*puVar1);
      lpBuffer = (LPCVOID)*puVar1;
      hFile = GetStdHandle(0xfffffff4);
      WriteFile(hFile,lpBuffer,sVar4,pDVar2,lpOverlapped);
    }
    else if (param_1 != 0xfc) {
      DVar3 = GetModuleFileNameA((HMODULE)0x0,(LPSTR)local_1a8,0x104);
      if (DVar3 == 0) {
        FUN_00408170(local_1a8,(uint *)"<program name unknown>");
      }
      _Dest = local_1a8;
      sVar4 = _strlen((char *)local_1a8);
      if (0x3c < sVar4 + 1) {
        sVar4 = _strlen((char *)local_1a8);
        _Dest = (uint *)(auStackY_1e3 + sVar4);
        _strncpy((char *)_Dest,"...",3);
      }
      FUN_00408170(local_a4,(uint *)"Runtime Error!\n\nProgram: ");
      FUN_00408180(local_a4,_Dest);
      FUN_00408180(local_a4,(uint *)&DAT_0040c0dc);
      FUN_00408180(local_a4,*(uint **)(iVar5 * 8 + 0x40c43c));
      auStackY_1e3._3_4_ = 0x407ba1;
      FUN_00408ede(local_a4,"Microsoft Visual C++ Runtime Library",0x12010);
    }
  }
  return;
}

