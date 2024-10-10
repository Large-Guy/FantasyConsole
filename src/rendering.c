#include "rendering.h"

Screen* screenCreate(int width, int height) {
    Screen *screen = malloc(sizeof(Screen));
    screen->buffer = malloc(width * height * sizeof(unsigned char));
    screen->width = width;
    screen->height = height;
    return screen;
}

Palette* paletteCreate() {
    Palette *palette = malloc(sizeof(Palette));
    palette->colors = malloc(256 * sizeof(Color));

    //Generate the vga palette

    //Standard VGA 16 color palette
    palette->colors[0x00] = (Color){0x00, 0x00, 0x00};
    palette->colors[0x01] = (Color){0x00, 0x00, 0xAA};
    palette->colors[0x02] = (Color){0x00, 0xAA, 0x00};
    palette->colors[0x03] = (Color){0x00, 0xAA, 0xAA};
    palette->colors[0x04] = (Color){0xAA, 0x00, 0x00};
    palette->colors[0x05] = (Color){0xAA, 0x00, 0xAA};
    palette->colors[0x06] = (Color){0xAA, 0x55, 0x00};
    palette->colors[0x07] = (Color){0xAA, 0xAA, 0xAA};
    palette->colors[0x08] = (Color){0x55, 0x55, 0x55};
    palette->colors[0x09] = (Color){0x55, 0x55, 0xFF};
    palette->colors[0x0A] = (Color){0x55, 0xFF, 0x55};
    palette->colors[0x0B] = (Color){0x55, 0xFF, 0xFF};
    palette->colors[0x0C] = (Color){0xFF, 0x55, 0x55};
    palette->colors[0x0D] = (Color){0xFF, 0x55, 0xFF};
    palette->colors[0x0E] = (Color){0xFF, 0xFF, 0x55};
    palette->colors[0x0F] = (Color){0xFF, 0xFF, 0xFF};

    //Grey scale
    for (int i = 0x10; i < 0x20; i++) {
        palette->colors[i] = (Color){i, i, i};
    }

    //216 color cube
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                int i = 0x20 + r * 36 + g * 6 + b;
                palette->colors[i] = (Color){r * 51, g * 51, b * 51};
            }
        }
    }

    return palette;
}