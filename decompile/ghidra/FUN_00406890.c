/* FUN_00406890 @ 0x00406890 size=58 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00406890(void)

{
  int nWidth;
  
  nWidth = (int)(short)((short)DAT_0040c66c + 4);
  MoveWindow(DAT_0040c624,DAT_0040c6b8 - nWidth,DAT_0040c6b4,nWidth,(int)(short)(_DAT_0040c66a + 4),
             1);
  return;
}

