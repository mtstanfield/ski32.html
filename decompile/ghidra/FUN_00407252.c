/* FUN_00407252 @ 0x00407252 size=185 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00407252(void)

{
  char cVar1;
  size_t sVar2;
  undefined4 *puVar3;
  void *pvVar4;
  int iVar5;
  uint *puVar6;
  
  if (DAT_0040ccc8 == 0) {
    FUN_0040811f();
  }
  iVar5 = 0;
  for (puVar6 = DAT_0040c798; (char)*puVar6 != '\0'; puVar6 = (uint *)((int)puVar6 + sVar2 + 1)) {
    if ((char)*puVar6 != '=') {
      iVar5 = iVar5 + 1;
    }
    sVar2 = _strlen((char *)puVar6);
  }
  puVar3 = _malloc(iVar5 * 4 + 4);
  _DAT_0040c7cc = puVar3;
  if (puVar3 == (undefined4 *)0x0) {
    FUN_00406e79(9);
  }
  cVar1 = (char)*DAT_0040c798;
  puVar6 = DAT_0040c798;
  while (cVar1 != '\0') {
    sVar2 = _strlen((char *)puVar6);
    if ((char)*puVar6 != '=') {
      pvVar4 = _malloc(sVar2 + 1);
      *puVar3 = pvVar4;
      if (pvVar4 == (void *)0x0) {
        FUN_00406e79(9);
      }
      FUN_00408170((uint *)*puVar3,puVar6);
      puVar3 = puVar3 + 1;
    }
    puVar6 = (uint *)((int)puVar6 + sVar2 + 1);
    cVar1 = (char)*puVar6;
  }
  FUN_0040813b(DAT_0040c798);
  DAT_0040c798 = (uint *)0x0;
  *puVar3 = 0;
  _DAT_0040ccc4 = 1;
  return;
}

