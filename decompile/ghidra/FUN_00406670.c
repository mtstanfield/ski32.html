/* FUN_00406670 @ 0x00406670 size=85 */

char __fastcall FUN_00406670(short param_1,short param_2)

{
  if (-1 < param_1) {
    if (param_2 < 0) {
      return (-(int)param_1 != (int)param_2 && (int)param_1 <= -(int)param_2) + '\x0f';
    }
    return (param_2 <= param_1) + '\r';
  }
  if (param_2 < 0) {
    return ((param_1 <= param_2) - 1U & 2) + 0xe;
  }
  return ((int)param_1 <= -(int)param_2) + '\r';
}

