/* FUN_00406e6e @ 0x00406e6e size=11 */

/* WARNING: Stack frame is not setup normally: Input value of stackpointer is not used */

void FUN_00406e6e(void)

{
  int iVar1;
  int unaff_EBP;
  
  iVar1 = *(int *)(unaff_EBP + -0x18);
  *(undefined4 *)(iVar1 + -4) = *(undefined4 *)(unaff_EBP + -0x68);
                    /* WARNING: Subroutine does not return */
  *(code **)(iVar1 + -8) = FUN_00406e79;
  __exit(*(int *)(iVar1 + -4));
}

