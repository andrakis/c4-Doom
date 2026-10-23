// dg-bare.c -- on the bare machine and under C4DOS the display is the
// game's to take: there is no one to ask (dg-gui.c), and no window to
// play in instead

int dg_take(void) { return 1; }
void dg_give(void) {}
int dg_win_open(void) { return 0; }
void dg_win_draw(unsigned char *pixels, int *pal) { (void)pixels; (void)pal; }
int dg_win_key(int *code, int *down) { (void)code; (void)down; return 0 - 1; }
void dg_win_close(void) {}
