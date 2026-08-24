/* FUN_00406c50 @ 0x00406c50 size=47 */

void __fastcall FUN_00406c50(HDC param_1,short *param_2,LPCSTR param_3,int param_4)

{
  tagSIZE local_8;
  
  GetTextExtentPoint32A(param_1,param_3,param_4,&local_8);
  if (*param_2 < (short)local_8.cx) {
    *param_2 = (short)local_8.cx;
  }
  return;
}

