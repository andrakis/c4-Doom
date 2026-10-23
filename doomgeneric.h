#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

#ifndef DOOMGENERIC_RESX
#define DOOMGENERIC_RESX 640
#endif  // DOOMGENERIC_RESX

#ifndef DOOMGENERIC_RESY
#define DOOMGENERIC_RESY 400
#endif  // DOOMGENERIC_RESY


#ifdef CMAP256

typedef uint8_t pixel_t;

#else  // CMAP256

typedef uint32_t pixel_t;

#endif  // CMAP256


extern pixel_t* DG_ScreenBuffer;

#ifdef __cplusplus
extern "C" {
#endif

void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();


//Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

// c4doom: an 8-bit frame and its palette (0x00RRGGBB), from dg-sum.c,
// which also checksums a -timedemo's frames
void DG_DrawFrame8(unsigned char *pixels, int *pal);
void dg_frame(unsigned char *pixels);
void dg_start(void);
extern int dg_pal[256];
// the platform's real clock; DG_GetTicksMs and DG_SleepMs are dg-sum.c's
uint32_t plat_ms(void);
void plat_sleep(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif //DOOM_GENERIC
