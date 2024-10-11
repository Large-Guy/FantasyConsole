#ifndef FAKEOS_ASM_H
#define FAKEOS_ASM_H
#include <stdlib.h>
#include <stdbool.h>
#include "rendering.h"

typedef enum {
    NOP,
    SYS,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    JMP,
    JEQ,
    JNE,
    JLT,
    JGT,
    BRN,
    BEQ,
    BNE,
    BLT,
    BGT,
    CMP,
    RET,
    REG,
    IMM,
    ALC,
    FRE,

    //Extended opcodes
    //Video memory
    SPX, //Write pixel to the screen buffer
    CLS, //Clear the screen buffer
} OpCode;

typedef enum {
    CMP_EQUAL = 1,
    CMP_LESS = 2,
    CMP_GREATER = 4,
} CMPFlags;

struct VM{
    unsigned char* memory;
    bool* memoryMap;
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
VM* vmLoadProgram(VM* vm, unsigned char* program, int length);
void vmSysCall(VM* vm, int (*func)(VM* vm));
int vmRun(VM* vm);
short readShort(VM* vm);
short vmAlloc(VM* vm, int size);
void vmFree(VM* vm, short ptr, int size);

#endif //FAKEOS_ASM_H
