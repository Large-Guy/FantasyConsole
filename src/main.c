#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "rendering.h"
#include "asm.h"

const int WIDTH = 400;
const int HEIGHT = 300;
const int SCALE = 3;

//SDL
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

//2 Screen buffers for double buffering
Screen* buffers[2];
Screen* screen = NULL;
Palette* palette = NULL;

double lastTime = 0;
double currentTime = 0;

int sysCallExit(VM* vm) {
    return 1;
}

int sysCallPrint(VM* vm) {
    printf("Print Syscall: %d\n", vm->registers[0]);
    return 0;
}

int sysCallFlushScreen(VM* vm) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return 1;
        }
    }

    //Swap buffers
    screen = buffers[screen == buffers[0]];

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    for (int i = 0; i < screen->width * screen->height; i++) {
        Color color = palette->colors[screen->buffer[i]];
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawPoint(renderer, i % screen->width, i / screen->width);
    }

    SDL_RenderPresent(renderer);

    currentTime = SDL_GetTicks();
    double delta = currentTime - lastTime;
    lastTime = currentTime;

    vm->bp = screen == buffers[0] ? 0 : 1;
    return 0;
}

#define SHORT(x) (x & 0xFF), ((x >> 8) & 0xFF)

unsigned char program[] = {
        MOV, REG, 0, IMM, SHORT(0), //Set register 0 to 0
        CLS, REG, 0, //Clear the screen with color 0
        ADD, REG, 0, REG, 0, IMM, SHORT(1), //Add 1 to register 0
        SYS, IMM, SHORT(2), //Flush the screen
        JMP, IMM, SHORT(6), //Jump to the beginning
};

int main(void) {
    //region SDL setup
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Hello World!", 100, 100, WIDTH*SCALE, HEIGHT*SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_RenderSetScale(renderer, SCALE, SCALE);

    //endregion
    buffers[0] = screenCreate(WIDTH, HEIGHT);
    buffers[1] = screenCreate(WIDTH, HEIGHT);

    screen = buffers[0];

    palette = paletteCreate();

    lastTime = SDL_GetTicks();

//region VM setup
    VM* vm = vmCreate();

    vm->buffers[0] = buffers[0];
    vm->buffers[1] = buffers[1];

    vmSysCall(vm, sysCallExit);
    vmSysCall(vm, sysCallPrint);
    vmSysCall(vm, sysCallFlushScreen);

    vmLoadProgram(vm, program, sizeof(program));

    int result = vmRun(vm);

    printf("Program exited with code %d\n", result);

    if(result == -1) {
        printf("Error: %s\n    at byte: %d", vm->error, vm->ip);
    }
//endregion

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
