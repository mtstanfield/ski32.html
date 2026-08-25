/* Reconstructed SkiFree 1.04 (32-bit) game core.
 * Reconstructed from decompile/ghidra — each src file cites its source
 * Ghidra address range in a header comment.
 */
#ifndef SKI_GAME_H
#define SKI_GAME_H
#include <windows.h>
#include "skidef.h"

extern int g_ski_tick;
void ski_init(void);            /* globals, resources, windows        */
void ski_run(void);             /* message loop until WM_QUIT         */
void ski_tick(void);            /* one WM_TIMER game step             */
void ski_render_main(void);     /* draw main window (GDI)             */
void ski_render_status(void);   /* draw status panel in main client area (GDI) — M0 amendment: single window */
#endif
