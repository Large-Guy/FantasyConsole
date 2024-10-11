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
    short address = vm->registers[1]; //Address of the string
    short length = vm->registers[2]; //Length of the string
    while (vm->memory[address] != 0) {
        printf("%c", vm->memory[address]);
        address++;
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

#define SHORT(x) (x & 0xFF), ((x >> 8) & 0xFF)

unsigned char program[] = {
        //Initalization stuff
        ALC, REG, 0, IMS, SHORT(14), //Allocate 14 bytes of memory "Hello, World!\n"
        STB, REG, 0, IMS, SHORT(0), IMB, 'H', //Store 'H' at 0
        STB, REG, 0, IMS, SHORT(1), IMB, 'e', //Store 'e' at 1
        STB, REG, 0, IMS, SHORT(2), IMB, 'l', //Store 'l' at 2
        STB, REG, 0, IMS, SHORT(3), IMB, 'l', //Store 'l' at 3
        STB, REG, 0, IMS, SHORT(4), IMB, 'o', //Store 'o' at 4
        STB, REG, 0, IMS, SHORT(5), IMB, ',', //Store ',' at 5
        STB, REG, 0, IMS, SHORT(6), IMB, ' ', //Store ' ' at 6
        STB, REG, 0, IMS, SHORT(7), IMB, 'W', //Store 'W' at 7
        STB, REG, 0, IMS, SHORT(8), IMB, 'o', //Store 'o' at 8
        STB, REG, 0, IMS, SHORT(9), IMB, 'r', //Store 'r' at 9
        STB, REG, 0, IMS, SHORT(10), IMB, 'l', //Store 'l' at 10
        STB, REG, 0, IMS, SHORT(11), IMB, 'd', //Store 'd' at 11
        STB, REG, 0, IMS, SHORT(12), IMB, '!', //Store '!' at 12
        STB, REG, 0, IMS, SHORT(13), IMB, '\n', //Store '\n' at 13

        //Print "Hello, World!\n"
        //"Hello, World!\n" is stored at register 0 already
        MOV, REG, 1, REG, 0, //Move the address of "Hello, World!\n" to register 1
        MOV, REG, 2, IMS, SHORT(14), //Move the address of "Hello, World!\n" to register 1

        SYS, IMS, SHORT(1), //Call the print syscall

        FRE, REG, 0, //Free the memory allocated for "Hello, World!\n"

        MOV, REG, 0, IMS, SHORT(0), //Move 0 to register 0
        SYS, IMS, SHORT(0), //Call the exit syscall
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
