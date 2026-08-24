/* FUN_00401e20 @ 0x00401e20 size=42 */

void __fastcall FUN_00401e20(HDC param_1,LPCSTR param_2,short param_3,short *param_4,int param_5)

{
  TextOutA(param_1,(int)param_3,(int)*param_4,param_2,param_5);
  *param_4 = *param_4 + DAT_0040c668;
  return;
}

