#ifndef FAKEOS_ASM_H
#define FAKEOS_ASM_H
#include <stdlib.h>
#include <stdbool.h>
#include "rendering.h"
#include "ops.h"
#include "chunk.h"

struct VM{
    unsigned char* memory;
    bool* memoryMap;
    unsigned short* memorySizes;
    unsigned short registers[16];

    //Rendering
    unsigned char* videoMemory; //For sprites and such
    unsigned short bp;
    Screen* buffers[2]; //For the actual screen

    unsigned short sp;
    unsigned short stack[256];

    unsigned short cp;
    unsigned short callStack[256];

    unsigned short ip;
    unsigned int codeLength;
    unsigned char* code;

    unsigned char cmpFlags;

    int sysCallCount;
    int (*sysCalls[256])(struct VM* vm);

    void (*debugger)(struct VM* vm);

    bool interrupt;
    const char* error;
};

typedef struct VM VM;

VM* vmCreate();
VM* vmLoadProgram(VM* vm, Chunk* chunk);
void vmSysCall(VM* vm, int (*func)(VM* vm));
int vmRun(VM* vm);
short readShort(VM* vm);
short vmAlloc(VM* vm, int size);
void vmFree(VM* vm, short ptr);

#endif //FAKEOS_ASM_H
