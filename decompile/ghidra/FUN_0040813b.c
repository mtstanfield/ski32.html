/* FUN_0040813b @ 0x0040813b size=47 */

void __cdecl FUN_0040813b(LPVOID param_1)

{
  uint *puVar1;
  
  if (param_1 != (LPVOID)0x0) {
    puVar1 = (uint *)FUN_004086d3((int)param_1);
    if (puVar1 != (uint *)0x0) {
      FUN_004086fe(puVar1,(uint)param_1);
      return;
    }
    HeapFree(DAT_0040cba8,0,param_1);
  }
  return;
}

