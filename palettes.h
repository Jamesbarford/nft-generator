#ifndef __PALETTES_H__
#define __PALETTES_H__

#include "hmap.h"

typedef struct colorPalette {
    int size;
    int **colors;
} colorPalette;

hmap *colorPaletteMapCreate();

#endif
