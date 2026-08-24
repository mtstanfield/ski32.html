/* FUN_00406fc3 @ 0x00406fc3 size=153 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_00406fc3(UINT param_1,int param_2,int param_3)

{
  HANDLE hProcess;
  undefined4 *puVar1;
  UINT uExitCode;
  
  if (DAT_0040c7ec == 1) {
    uExitCode = param_1;
    hProcess = GetCurrentProcess();
    TerminateProcess(hProcess,uExitCode);
  }
  _DAT_0040c7e8 = 1;
  DAT_0040c7e4 = (undefined1)param_3;
  if (param_2 == 0) {
    if ((DAT_0040ccd0 != (undefined4 *)0x0) &&
       (puVar1 = (undefined4 *)(DAT_0040cccc - 4), DAT_0040ccd0 <= puVar1)) {
      do {
        if ((code *)*puVar1 != (code *)0x0) {
          (*(code *)*puVar1)();
        }
        puVar1 = puVar1 + -1;
      } while (DAT_0040ccd0 <= puVar1);
    }
    FUN_0040705c((undefined4 *)&DAT_0040c014,(undefined4 *)&DAT_0040c018);
  }
  FUN_0040705c((undefined4 *)&DAT_0040c01c,(undefined4 *)&DAT_0040c020);
  if (param_3 != 0) {
    return;
  }
  DAT_0040c7ec = 1;
                    /* WARNING: Subroutine does not return */
  ExitProcess(param_1);
}

