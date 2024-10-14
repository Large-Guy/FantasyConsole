#ifndef FAKEOS_CHUNK_H
#define FAKEOS_CHUNK_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned char* data;
    int capacity;
    int size;
} Chunk;

Chunk* chunkCreate();
void chunkWriteByte(Chunk* chunk, unsigned char data);
void chunkDestroy(Chunk* chunk);

#endif //FAKEOS_CHUNK_H
