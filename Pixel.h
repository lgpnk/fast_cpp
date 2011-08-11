#ifndef PIXEL_H
#define PIXEL_H
#include <stdint.h>

typedef struct 
{ 
    uint16_t x, y;
    bool bright;
} Pixel;
#endif // PIXEL_H
