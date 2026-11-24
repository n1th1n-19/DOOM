// Final frame: 3D hi-res view + 320x200 UI buffer -> window-sized ARGB.
#ifndef __COMPOSE__
#define __COMPOSE__
#include <stdint.h>

// pal: 256 ARGB entries. dst: winwidth*winheight pixels.
void Compose (uint32_t* dst, const uint32_t* pal);

#endif
