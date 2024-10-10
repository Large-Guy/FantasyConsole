#ifndef FAKEOS_RENDERING_H
#define FAKEOS_RENDERING_H
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

typedef struct {
    unsigned char *buffer;
    int width, height;
} Screen;

Screen* screenCreate(int width, int height);

typedef struct {
    unsigned char r, g, b;
} Color;

typedef struct {
    Color* colors;
} Palette;

Palette* paletteCreate();


#endif //FAKEOS_RENDERING_H
