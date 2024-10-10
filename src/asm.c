#include "asm.h"

VM* vmCreate() {
    VM *vm = malloc(sizeof(VM));
    vm->memory = malloc(4096);
    vm->codeLength = 0;
    vm->code = NULL;
    vm->ip = 0;
    vm->sp = 0;
    vm->cp = 0;
    vm->bp = 0;
    vm->cmpFlags = 0;
    vm->interrupt = false;
    vm->sysCallCount = 0;
    return vm;
}

VM* vmLoadProgram(VM* vm, unsigned char* program, int length) {
    vm->code = malloc(length);
    vm->codeLength = length;
    for (int i = 0; i < length; i++) {
        vm->code[i] = program[i];
    }
    return vm;
}

void vmSysCall(VM* vm, int (*func)(VM* vm)) {
    vm->sysCalls[vm->sysCallCount] = func;
    vm->sysCallCount++;
}

int vmRun(VM* vm) {
#define ERROR(msg) {vm->error = msg; vm->interrupt = true; return -1;}

    vm->interrupt = false;
    while(!vm->interrupt && vm->ip < vm->codeLength) {
        OpCode op = vm->code[vm->ip];
        switch (op) {
            case NOP: {
                vm->ip++;
                break;
            }
            case SYS: {
                vm->ip++;
                short sysCall = readShort(vm);
                if(vm->sysCalls[sysCall](vm) != 0) {
                    vm->interrupt = true;
                }
                vm->ip++;
                break;
            }
            case MOV: {
                vm->ip++;
                if(vm->code[vm->ip] != REG)
                    ERROR("Expected register");
                vm->ip++;
                unsigned char reg = vm->code[vm->ip];
                vm->ip++;
                short value = readShort(vm);
                vm->registers[reg] = value;
                vm->ip++;
                break;
            }
            case ADD: {
                vm->ip++;
                if(vm->code[vm->ip] != REG)
                    ERROR("Expected register");
                vm->ip++;
                unsigned char reg = vm->code[vm->ip];
                vm->ip++;
                short a = readShort(vm);
                vm->ip++;
                short b = readShort(vm);
                vm->registers[reg] = a + b;
                vm->ip++;
                break;
            }
            case SUB: {
                vm->ip++;
                if(vm->code[vm->ip] != REG)
                    ERROR("Expected register");
                vm->ip++;
                unsigned char reg = vm->code[vm->ip];
                vm->ip++;
                short a = readShort(vm);
                vm->ip++;
                short b = readShort(vm);
                vm->registers[reg] = a - b;
                vm->ip++;
                break;
            }
            case MUL: {
                vm->ip++;
                if(vm->code[vm->ip] != REG)
                    ERROR("Expected register");
                vm->ip++;
                unsigned char reg = vm->code[vm->ip];
                vm->ip++;
                short a = readShort(vm);
                vm->ip++;
                short b = readShort(vm);
                vm->registers[reg] = a * b;
                vm->ip++;
                break;
            }
            case DIV: {
                vm->ip++;
                if(vm->code[vm->ip] != REG)
                    ERROR("Expected register");
                vm->ip++;
                unsigned char reg = vm->code[vm->ip];
                vm->ip++;
                short a = readShort(vm);
                vm->ip++;
                short b = readShort(vm);
                if(b == 0)
                    ERROR("Division by zero");
                vm->registers[reg] = a / b;
                vm->ip++;
                break;
            }
            case BRN: {
                vm->ip++;
                short jumpLocation = readShort(vm);

                vm->callStack[vm->cp] = vm->ip + 1;
                vm->cp++;

                vm->ip = jumpLocation;
                break;
            }
            case BEQ: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_EQUAL) {
                    vm->callStack[vm->cp] = vm->ip + 1;
                    vm->cp++;

                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case BNE: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(!(vm->cmpFlags & CMP_EQUAL)) {
                    vm->callStack[vm->cp] = vm->ip + 1;
                    vm->cp++;

                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case BLT: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_LESS) {
                    vm->callStack[vm->cp] = vm->ip + 1;
                    vm->cp++;

                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case BGT: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_GREATER) {
                    //add to call stack
                    vm->callStack[vm->cp] = vm->ip + 1;
                    vm->cp++;

                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case JMP: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                vm->ip = jumpLocation;
                break;
            }
            case JEQ: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_EQUAL) {
                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case JNE: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(!(vm->cmpFlags & CMP_EQUAL)) {
                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case JLT: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_LESS) {
                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case JGT: {
                vm->ip++;
                short jumpLocation = readShort(vm);
                if(vm->cmpFlags & CMP_GREATER) {
                    vm->ip = jumpLocation;
                } else {
                    vm->ip++;
                }
                break;
            }
            case CMP: {
                vm->ip++;
                short a = readShort(vm);
                vm->ip++;
                short b = readShort(vm);
                if(a == b) {
                    vm->cmpFlags |= CMP_EQUAL;
                } else {
                    vm->cmpFlags &= ~CMP_EQUAL;
                }
                if(a < b) {
                    vm->cmpFlags |= CMP_LESS;
                } else {
                    vm->cmpFlags &= ~CMP_LESS;
                }
                if(a > b) {
                    vm->cmpFlags |= CMP_GREATER;
                } else {
                    vm->cmpFlags &= ~CMP_GREATER;
                }
                vm->ip++;
                break;
            }
            case RET: {
                vm->ip = vm->callStack[vm->cp - 1];
                vm->cp--;
                break;
            }
            //Extended opcodes
            case SPX: {
                vm->ip++;
                short x = readShort(vm);
                vm->ip++;
                short y = readShort(vm);
                vm->ip++;
                short color = readShort(vm);
                vm->buffers[vm->bp]->buffer[y * vm->buffers[vm->bp]->width + x] = (unsigned char)color;
                vm->ip++;
                break;
            }
            case CLS: {
                vm->ip++;
                short color = readShort(vm);
                for (int i = 0; i < vm->buffers[vm->bp]->width * vm->buffers[vm->bp]->height; i++) {
                    vm->buffers[vm->bp]->buffer[i] = (unsigned char)color;
                }
                vm->ip++;
                break;
            }
            default: {
                vm->interrupt = true;
                vm->error = "Unknown opcode";
                return -1;
            }
        }
    }
    return vm->registers[0];
}

short readShort(VM* vm) {
    unsigned char low = vm->code[vm->ip];
    switch(low) {
        case IMM:
        {
            vm->ip++;
            unsigned char a = vm->code[vm->ip];
            vm->ip++;
            unsigned char b = vm->code[vm->ip];
            return (b << 8) | a;
        }
        case REG:
        {
            vm->ip++;
            unsigned char reg = vm->code[vm->ip];
            return vm->registers[reg];
        }
        default:
            return low;
    }
}
