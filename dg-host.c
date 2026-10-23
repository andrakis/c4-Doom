// dg-host.c -- the port's platform on a host, built with gcc (make
// doom-host): no window and no sound. The reference the c4lc build is
// held to: a -timedemo's frames are checksummed (dg-sum.c).

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "doomgeneric.h"

void DG_Init() {}
void DG_DrawFrame() {}
void DG_DrawFrame8(unsigned char *pixels, int *pal) { (void)pixels; (void)pal; }
void plat_sleep(uint32_t ms) { usleep(ms * 1000); }
uint32_t plat_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
int DG_GetKey(int *pressed, unsigned char *key) { (void)pressed; (void)key; return 0; }
void DG_SetWindowTitle(const char *title) { (void)title; }

int main(int argc, char **argv) {
    doomgeneric_Create(argc, argv);
    for (;;) doomgeneric_Tick();
    return 0;
}
