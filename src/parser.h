#ifndef FAKEOS_PARSER_H
#define FAKEOS_PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "ops.h"
#include "chunk.h"

typedef struct {
    const char* name;
    short location;
} Label;

typedef struct {
    Label* labels;
    int count;
    int capacity;
} LabelTable;

LabelTable* labelTableCreate();
void labelTableAdd(LabelTable* table, Label label);
Label* labelTableGet(LabelTable* table, const char* name);
bool labelTableContains(LabelTable* table, const char* name);
void labelTableDestroy(LabelTable* table);

Chunk* parseFile(const char* filename);
Chunk* parseText(char* string);
int parseOpcode(Chunk* chunk, LabelTable* labels, char* text);
char** splitString(char* string, char* delimiter, int* count);

#endif //FAKEOS_PARSER_H
