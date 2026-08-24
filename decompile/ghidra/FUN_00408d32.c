/* FUN_00408d32 @ 0x00408d32 size=177 */

undefined4 * FUN_00408d32(void)

{
  undefined4 *puVar1;
  LPVOID pvVar2;
  
  if (DAT_0040c97c == DAT_0040c96c) {
    pvVar2 = HeapReAlloc(DAT_0040cba8,0,DAT_0040c980,(DAT_0040c96c * 5 + 0x50) * 4);
    if (pvVar2 == (LPVOID)0x0) {
      return (undefined4 *)0x0;
    }
    DAT_0040c96c = DAT_0040c96c + 0x10;
    DAT_0040c980 = pvVar2;
  }
  puVar1 = (undefined4 *)((int)DAT_0040c980 + DAT_0040c97c * 0x14);
  pvVar2 = HeapAlloc(DAT_0040cba8,8,0x41c4);
  puVar1[4] = pvVar2;
  if (pvVar2 != (LPVOID)0x0) {
    pvVar2 = VirtualAlloc((LPVOID)0x0,0x100000,0x2000,4);
    puVar1[3] = pvVar2;
    if (pvVar2 != (LPVOID)0x0) {
      puVar1[2] = 0xffffffff;
      *puVar1 = 0;
      puVar1[1] = 0;
      DAT_0040c97c = DAT_0040c97c + 1;
      *(undefined4 *)puVar1[4] = 0xffffffff;
      return puVar1;
    }
    HeapFree(DAT_0040cba8,0,(LPVOID)puVar1[4]);
  }
  return (undefined4 *)0x0;
}

