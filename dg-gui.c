// dg-gui.c -- the port's platform on libjs's display (libjs/guest/gui.h):
// C4IX, where the desktop gives the display way until the game ends
// (dg-c4ix.c), and the bare machine and C4DOS (dg-bare.c).
//
// Doom's 320x200 frames go out 8-bit, as GUI_BLIT8 with their palette
// (dg-sum.c), and the host scales them to 640x480: 2x across and 2.4x
// down, the 4:3 shape a VGA monitor gave them. On C4IX, with -window or
// when a program that does not give way has the display, they go to the
// Terminal window the game was started from instead (dg-c4ix.c). Keys come from the
// display's event ring, as Doom's own key codes (doomkeys.h): Ctrl fires,
// Space opens, Alt strafes and Shift runs, as the original's defaults
// have it. Time is the machine's (TIME); a sleep is a USLP, which on C4IX
// is a sleep of this task alone. A -timedemo runs without the display:
// its frames are checksummed, not shown -- unless -show is given too,
// so that the time it takes counts the drawing.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "doomgeneric.h"
#include "doomkeys.h"
#include "m_argv.h"
#include "gui.h"

int  dg_take(void);                     // the display is ours to use
void dg_give(void);                     // and given back
int  dg_win_open(void);                 // or a window (C4IX's; 0 elsewhere)
void dg_win_draw(unsigned char *pixels, int *pal);
int  dg_win_key(int *code, int *down);
void dg_win_close(void);
void dg_sound_init(void);               // dg-snd.c

enum { W = 640, H = 480, RING = 65536, KQ = 64 };

static int on;                          // the display is ours
static int win;                         // or the game plays in a window
static int *ring;
static int kq[KQ];                      // keys waiting: (pressed << 8) | Doom's code
static int kh, kt;

static void dg_close(void) {
    if (!on) return;
    gui_cursor(GUI_PTR_ARROW);
    gui_detach();
    dg_give();
    on = 0;
}

static void dg_unwin(void) { dg_win_close(); }

static int dg_window(void) {
    if (!(win = dg_win_open())) return 0;
    atexit(dg_unwin);
    return 1;
}

void DG_Init() {
    if (M_CheckParm("-timedemo") && !M_CheckParm("-show") && !M_CheckParm("--show")) return;
    if ((M_CheckParm("-window") || M_CheckParm("--window")) && dg_window()) return;
    if (!gui_present()) { printf("doom: this machine has no display\n"); return; }
    if (!dg_take()) {
        if (dg_window()) { printf("doom: a program has the display; playing in the window\n"); return; }
        printf("doom: a program has the display\n");
        return;
    }
    if (!(ring = (int *)malloc(RING)) || !gui_attach(ring, RING, W, H)) { dg_give(); return; }
    gui_events(GUI_M_KEYS | GUI_M_WINDOW);
    gui_cursor(GUI_PTR_NONE);
    gui_clear(0x000000);
    gui_show();
    on = 1;
    atexit(dg_close);
}

void DG_DrawFrame() {}

void DG_DrawFrame8(unsigned char *pixels, int *pal) {
    if (win) { dg_win_draw(pixels, pal); return; }
    if (!on) return;
    gui_blit8(0, 0, 320, 200, (char *)pixels, 320, pal, W, H);
    gui_show();
}

// a browser keyCode as Doom names the key, 0 for one it has no use for
static int dkey(int code) {
    if (code >= 65 && code <= 90) return code + 32;     // letters, lower case
    if (code >= 48 && code <= 57) return code;          // digits
    if (code >= 112 && code <= 121) return 0x80 + 0x3b + (code - 112);   // F1-F10
    switch (code) {
    case 122: return KEY_F11;
    case 123: return KEY_F12;
    case 37: return KEY_LEFTARROW;
    case 38: return KEY_UPARROW;
    case 39: return KEY_RIGHTARROW;
    case 40: return KEY_DOWNARROW;
    case 13: return KEY_ENTER;
    case 27: return KEY_ESCAPE;
    case 9: return KEY_TAB;
    case 8: return KEY_BACKSPACE;
    case 17: return KEY_FIRE;
    case 32: return KEY_USE;
    case 16: return KEY_RSHIFT;
    case 18: return KEY_RALT;
    case 19: return KEY_PAUSE;
    case 61: case 187: return KEY_EQUALS;
    case 173: case 189: return KEY_MINUS;
    case 188: return ',';
    case 190: return '.';
    case 191: return '/';
    case 186: return ';';
    case 222: return '\'';
    case 219: return '[';
    case 221: return ']';
    case 220: return '\\';
    case 192: return '`';
    case 36: return KEY_HOME;
    case 35: return KEY_END;
    case 33: return KEY_PGUP;
    case 34: return KEY_PGDN;
    case 45: return KEY_INS;
    case 46: return KEY_DEL;
    }
    return 0;
}

static void dg_queue(int code, int down) {
    int k;
    if (!(k = dkey(code)) || (kt + 1) % KQ == kh) return;
    kq[kt] = (down ? 256 : 0) | k;
    kt = (kt + 1) % KQ;
}

int DG_GetKey(int *pressed, unsigned char *key) {
    int ev[4], k, code, down;
    // the window closed: the game ends, as it does when it is quit
    while (win && (k = dg_win_key(&code, &down)) != 0) {
        if (k < 0) exit(0);
        dg_queue(code, down);
    }
    while (on && gui_poll(ev)) {
        if (ev[0] == GUI_EV_KEYDOWN || ev[0] == GUI_EV_KEYUP) dg_queue(ev[1], ev[0] == GUI_EV_KEYDOWN);
    }
    if (kh == kt) return 0;
    *pressed = kq[kh] >> 8;
    *key = kq[kh] & 255;
    kh = (kh + 1) % KQ;
    return 1;
}

void DG_SetWindowTitle(const char *title) { (void)title; }

uint32_t plat_ms(void) { return __time(); }
void plat_sleep(uint32_t ms) { if (ms) __c4_usleep(ms * 1000); }

// On C4IX the game lives in /usr/games/doom, where its config and saved
// games go too, as they did beside DOOM.EXE, and it starts there: the WAD
// there is a RAM file, which seeks, where the same name looked up from
// elsewhere can be the host's file of it, which does not. On the bare
// machine and under C4DOS there is only the one directory, and the chdir
// fails.
#define DOOM_HOME "/usr/games/doom"

int main(int argc, char **argv) {
    chdir(DOOM_HOME);
    doomgeneric_Create(argc, argv);
    for (;;) doomgeneric_Tick();
    return 0;
}
