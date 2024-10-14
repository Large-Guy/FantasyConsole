#ifndef FAKEOS_OPS_H
#define FAKEOS_OPS_H

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
    IMS,
    IMB,
    ALC,
    FRE,
    STB,
    LDB,

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

#endif //FAKEOS_OPS_H
