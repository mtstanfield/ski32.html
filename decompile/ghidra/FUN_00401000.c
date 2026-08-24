/* FUN_00401000 @ 0x00401000 size=96 */

void FUN_00401000(void)

{
  DWORD DVar1;
  
  DVar1 = GetTickCount();
  DAT_0040c5f4 = DVar1 - DAT_0040c698;
  DAT_0040c708 = DAT_0040c698;
  DAT_0040c698 = DVar1;
  FUN_00401e50();
  FUN_00401060(DAT_0040c63c,&DAT_0040c6b0);
  DAT_0040c610 = 1;
  if (0x147 < (int)(DAT_0040c698 - DAT_0040c5dc)) {
    FUN_00401b80(DAT_0040c6cc);
    return;
  }
  return;
}

