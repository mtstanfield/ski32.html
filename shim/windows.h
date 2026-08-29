/* Umbrella header so the unmodified game code's `#include <windows.h>`
 * resolves to the shim (compiled with -I shim). */
#ifndef SHIM_WINDOWS_H
#define SHIM_WINDOWS_H
#include <string.h>
#include "types.h"
#include "win32.h"
#define ZeroMemory(p, s) memset((p), 0, (size_t)(s))
#endif
