#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "rendering.h"
#include "asm.h"

const int WIDTH = 400;
const int HEIGHT = 300;
const int SCALE = 2;

//SDL
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

//2 Screen buffers for double buffering
Screen* buffers[2];
Screen* screen = NULL;
Palette* palette = NULL;

double lastTime = 0;
double currentTime = 0;

double debuggerLastTime = 0;
double debuggerCurrentTime = 0;
double debuggerUpdateTime = 0;

int sysCallExit(VM* vm) {
    return 1;
}

int sysCallPrint(VM* vm) {
    for (int i = 0; i < 16; i+=4) {
        printf("R%d: %d\tR%d: %d\tR%d: %d\tR%d: %d\n", i, vm->registers[i], i+1, vm->registers[i+1], i+2, vm->registers[i+2], i+3, vm->registers[i+3]);
    }
    printf("\n");
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

    printf("FPS: %f\n", 1000.0 / delta);

    vm->bp = screen == buffers[0] ? 0 : 1;
    return 0;
}

#define SHORT(x) (x & 0xFF), ((x >> 8) & 0xFF)

unsigned char program[] = {
        //Initalization stuff
        MOV, REG, 4, IMM, SHORT(0), //Set register 4 to 0 frame counter

        MOV, REG, 1, IMM, SHORT(0), //Set register 0 to 0 Y
        MOV, REG, 2, IMM, SHORT(0), //Set register 1 to 0 X
        MOV, REG, 3, REG, 4, //Set register 2 to 0 Color

        CMP , REG, 2, IMM, SHORT(WIDTH), //Compare X to WIDTH
        JGT, IMM, SHORT(104), //If X > WIDTH jump
        JEQ, IMM, SHORT(104), //If X == WIDTH jump

        MOV, REG, 1, IMM, SHORT(0), //Set Y to 0

        CMP, REG, 1, IMM, SHORT(HEIGHT), //Compare Y to HEIGHT
        JGT, IMM, SHORT(84), //If Y > HEIGHT jump
        JEQ, IMM, SHORT(84), //If Y == HEIGHT jump

        SPX, REG, 2, REG, 1, REG, 3, //Set pixel at X, Y to color

        ADD, REG, 3, REG, 3, IMM, SHORT(1), //Increment color

        ADD, REG, 1, REG, 1,IMM, SHORT(1), //Increment Y
        JMP, IMM, SHORT(43), //Jump to the y loop

        ADD, REG, 3, REG, 3, IMM, SHORT(1), //Increment color

        ADD, REG, 2, REG, 2, IMM, SHORT(1), //Increment X
        JMP, IMM, SHORT(23), //Jump to the x loop

        ADD, REG, 4, REG, 4, IMM, SHORT(1), //Increment frame counter

        SYS, IMM, SHORT(2), //Flush the screen
        JMP, IMM, SHORT(6), //Jump to the beginning
};

const char* names[] = {
        "SYS",
        "MOV",
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "JMP",
        "JEQ",
        "JNE",
        "JLT",
        "JGT",
        "BRN",
        "BEQ",
        "BNE",
        "BLT",
        "BGT",
        "CMP",
        "RET",
        "REG",
        "IMM",

        //Extended opcodes
        //Video memory
        "SPX", //Write pixel to the screen buffer
        "CLS", //Clear the screen buffer

        //Debugging
        "BREAK_POINT",
};

void manageDebugger(VM* vm) {
    //Set cursor to 0,0
    printf("\033[H");
    //Print the registers as a 4x4 grid
    printf("IP: %d\n", vm->ip);
    //Print the CMP flags as binary
    printf("CMP: %d%d%d\n", (vm->cmpFlags & CMP_EQUAL) > 0, (vm->cmpFlags & CMP_LESS) > 0, (vm->cmpFlags & CMP_GREATER) > 0);
    //Print the current opcode
    printf("Opcode: %s\n", names[vm->code[vm->ip]]);

    for (int i = 0; i < 16; i+=4) {
        printf("R%d: %d\tR%d: %d\tR%d: %d\tR%d: %d\n", i, vm->registers[i], i+1, vm->registers[i+1], i+2, vm->registers[i+2], i+3, vm->registers[i+3]);
    }

}

int main(void) {
    //region SDL setup
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Fake OS", 100, 100, WIDTH*SCALE, HEIGHT*SCALE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
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

    for (int i = 0; i < 16; ++i) {
        vm->registers[i] = 0;
    }

    //vm->debugger = manageDebugger;

    vm->buffers[0] = buffers[0];
    vm->buffers[1] = buffers[1];

    vmSysCall(vm, sysCallExit);
    vmSysCall(vm, sysCallPrint);
    vmSysCall(vm, sysCallFlushScreen);

    sysCallFlushScreen(vm);

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
