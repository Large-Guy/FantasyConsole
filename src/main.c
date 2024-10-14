#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "rendering.h"
#include "asm.h"
#include "parser.h"

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
    short address = vm->registers[1]; //Address of the string
    short length = vm->registers[2]; //Length of the string
    while (vm->memory[address] != 0) {
        printf("%c", vm->memory[address]);
        address++;
    }
    return 0;
}

int sysCallDebugRegisters(VM* vm) {
    for (int i = 0; i < 16; i+=4) {
        printf("R%d: %d\tR%d: %d\tR%d: %d\tR%d: %d\n", i, vm->registers[i], i+1, vm->registers[i+1], i+2, vm->registers[i+2], i+3, vm->registers[i+3]);
    }
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

//region Debugger

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

//endregion

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
    Chunk* program = parseFile("programs/test.asm");

    //Debug the program to a file
    FILE* file = fopen("programs/test.bin", "wb");
    fwrite(program->data, 1, program->size, file);
    fclose(file);

    VM* vm = vmCreate();

    //vm->debugger = manageDebugger;

    vm->buffers[0] = buffers[0];
    vm->buffers[1] = buffers[1];

    vmSysCall(vm, sysCallExit);
    vmSysCall(vm, sysCallPrint);
    vmSysCall(vm, sysCallFlushScreen);
    vmSysCall(vm, sysCallDebugRegisters);

    sysCallFlushScreen(vm);

    vmLoadProgram(vm, program);


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
