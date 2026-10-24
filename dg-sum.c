// dg-sum.c -- what every platform of this port shares: each frame Doom
// finishes (i_video.c's I_FinishUpdate) comes here as its 320x200 8-bit
// pixels and a palette of 256 0x00RRGGBB words, and goes to the
// platform's DG_DrawFrame8. With -timedemo, every frame is also folded
// into a checksum, printed when the game exits:
//
//   timedemo: demo1: 2134 frames, checksum 1a2b3c4d
//
// A timedemo plays one tic a frame whatever the clock says, but the screen
// wipe between levels runs by the clock: so while timing, the clock is
// virtual, as Wolf3D's is (c4sdl.c) -- each reading a millisecond later,
// and a sleep adding its wait. The frames are then the same on any
// machine, and the gcc and c4lc builds must print the same line (make
// test-doom). The platforms give the real clock, as plat_ms and
// plat_sleep.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "doomgeneric.h"
#include "m_argv.h"

int dg_pal[256];                        // 0x00RRGGBB, I_SetPalette's
static int dg_timing;                   // -timedemo: checksum the frames
static uint32_t dg_sum;
static int dg_frames;
static char *dg_demo;
static uint32_t dg_now;                 // the virtual clock, in ms

// How fast it went, by the real clock from the first frame, as a line
// before the checksum's: frames, seconds and frames a second. The clock is
// the machine's (TIME), which in the page is the wall's; under cli.js,
// unpaced, it is cycles over MHz.
static uint32_t t0;              // the real clock at the first frame
static void speed(void) {
    uint32_t ms;
    if (!dg_frames) return;
    ms = plat_ms() - t0;
    if (!ms) ms = 1;
    printf("speed: %s: %d frames in %d.%03d s, %d.%d fps\n", dg_demo, dg_frames,
           (int) (ms / 1000), (int) (ms % 1000), (int) (dg_frames * 10000 / ms / 10), (int) (dg_frames * 10000 / ms % 10));
}

static void dg_report(void) {
    speed();
    printf("timedemo: %s: %d frames, checksum %08x\n", dg_demo, dg_frames, (unsigned)dg_sum);
}

void dg_start(void) {
    int p;
    p = M_CheckParmWithArgs("-timedemo", 1);
    if (!p) return;
    dg_timing = 1;
    dg_demo = myargv[p + 1];
    atexit(dg_report);
}

void dg_frame(unsigned char *pixels) {
    int i;
    uint32_t s;
    if (dg_timing) {
        if (!dg_frames) t0 = plat_ms();
        s = dg_sum;
        for (i = 0; i < 256; i++) s = s * 31 + (uint32_t)dg_pal[i];
        // every pixel, four at a time: a word each, the way they lie
        for (i = 0; i < 320 * 200; i += 4)
            s = s * 31 + (uint32_t)(pixels[i] | (pixels[i + 1] << 8) | (pixels[i + 2] << 16) | ((uint32_t)pixels[i + 3] << 24));
        dg_sum = s;
        dg_frames++;
    }
    DG_DrawFrame8(pixels, dg_pal);
}

uint32_t DG_GetTicksMs() {
    if (dg_timing) return ++dg_now;
    return plat_ms();
}

void DG_SleepMs(uint32_t ms) {
    if (dg_timing) { dg_now += ms; return; }
    plat_sleep(ms);
}
