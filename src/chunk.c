#include "chunk.h"

Chunk* chunkCreate() {
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->data = malloc(1);
    chunk->size = 0;
    chunk->capacity = 1;
    return chunk;
}

void chunkWriteByte(Chunk* chunk, unsigned char data) {
    chunk->data[chunk->size] = data;
    chunk->size++;
    if (chunk->size >= chunk->capacity) {
        chunk->capacity *= 2;
        chunk->data = realloc(chunk->data, chunk->capacity);
    }
}

void chunkDestroy(Chunk* chunk) {
    free(chunk->data);
    free(chunk);
}