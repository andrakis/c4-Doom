// dg-c4ix.c -- on C4IX the display is the desktop's, which gives it way to
// the game until the game ends (include/display.h): dg-gui.c asks first
#include "display.h"

int dg_take(void) { return udisplay_take(0); }
void dg_give(void) { udisplay_give(); }
