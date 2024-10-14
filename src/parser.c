#include "parser.h"

LabelTable* labelTableCreate() {
    LabelTable* table = malloc(sizeof(LabelTable));
    table->labels = malloc(1);
    table->count = 0;
    table->capacity = 1;
    return table;
}

void labelTableAdd(LabelTable* table, Label label) {
    table->labels = realloc(table->labels, sizeof(Label) * (table->count + 1));
    table->labels[table->count] = label;
    table->count++;
}

Label* labelTableGet(LabelTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if(strcmp(table->labels[i].name, name) == 0) {
            return &table->labels[i];
        }
    }
    return NULL;
}

bool labelTableContains(LabelTable* table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if(strcmp(table->labels[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

void labelTableDestroy(LabelTable* table) {
    free(table->labels);
    free(table);
}

Chunk* parseFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(fileSize + 1);
    fread(buffer, 1, fileSize, file);
    fclose(file);

    Chunk* chunk = parseText(buffer);
    return chunk;
}

Chunk* parseText(char* string) {
    int count = 0;
    char** lines = splitString(string, "\n", &count);

    LabelTable* labels = labelTableCreate();

    //First find all the labels
    int location = 0;
    for (int i = 0; i < count; i++) {
        char *line = lines[i];
        int len = strlen(line);
        if (len == 0) {
            continue;
        }
        if (line[len - 1] == ':') {
            Label label;
            label.name = malloc(len);
            strncpy(label.name, line, len - 1);
            label.location = location;
            labelTableAdd(labels, label);
            lines[i] = "";
            continue;
        }
        int lineCount = 0;
        splitString(line, " ", &lineCount);
        location += lineCount;
    }


    Chunk* chunk = chunkCreate();
    for (int i = 0; i < count; i++) {
        char* line = lines[i];
        int lineCount = 0;
        char** tokens = splitString(lines[i], " ", &lineCount);
        for (int j = 0; j < lineCount; j++) {
            if(parseOpcode(chunk, labels, tokens[j]) == 1) {
                break;
            }
        }
    }

    return chunk;
}

int parseOpcode(Chunk* chunk, LabelTable* labels, char* text)
{
    //Convert text to lowercase
    for (int i = 0; text[i]; i++) {
        text[i] = tolower(text[i]);
    }
    if(labelTableContains(labels, text)) {
        short location = labelTableGet(labels, text)->location;
        chunkWriteByte(chunk, IMS);
        chunkWriteByte(chunk, location & 0xFF);
        chunkWriteByte(chunk, location >> 8);
        return 0;
    }
    switch (text[0]) {
        case '/': {
            if(strcmp(text, "/") == 0) {
                return 1;
            }
            break;
        }
        case 'a': {
            if(strcmp(text, "add") == 0) {
                chunkWriteByte(chunk, ADD);
            } else if(strcmp(text, "alc") == 0) {
                chunkWriteByte(chunk, ALC);
            }
            break;
        }
        case 's': {
            if(strcmp(text, "sys") == 0) {
                chunkWriteByte(chunk, SYS);
            } else if(strcmp(text, "sub") == 0) {
                chunkWriteByte(chunk, SUB);
            } else if(strcmp(text, "stb") == 0) {
                chunkWriteByte(chunk, STB);
            }
            break;
        }
        case 'j': {
            if(strcmp(text, "jmp") == 0) {
                chunkWriteByte(chunk, JMP);
            } else if(strcmp(text, "jeq") == 0) {
                chunkWriteByte(chunk, JEQ);
            } else if(strcmp(text, "jne") == 0) {
                chunkWriteByte(chunk, JNE);
            } else if(strcmp(text, "jgt") == 0) {
                chunkWriteByte(chunk, JGT);
            } else if(strcmp(text, "jlt") == 0) {
                chunkWriteByte(chunk, JLT);
            }
            break;
        }
        case 'm': {
            if(strcmp(text, "mov") == 0) {
                chunkWriteByte(chunk, MOV);
            } else if(strcmp(text, "mul") == 0) {
                chunkWriteByte(chunk, MUL);
            }
            break;
        }
        case 'd': {
            if(strcmp(text, "div") == 0) {
                chunkWriteByte(chunk, DIV);
            }
            break;
        }
        case 'c': {
            if(strcmp(text, "cmp") == 0) {
                chunkWriteByte(chunk, CMP);
            }
            break;
        }
        case 'r': {
            if(strcmp(text, "ret") == 0) {
                chunkWriteByte(chunk, RET);
            }
            break;
        }
        case 'b': {
            if(strcmp(text, "brn") == 0) {
                chunkWriteByte(chunk, BRN);
            } else if(strcmp(text, "beq") == 0) {
                chunkWriteByte(chunk, BEQ);
            } else if(strcmp(text, "bne") == 0) {
                chunkWriteByte(chunk, BNE);
            } else if(strcmp(text, "blt") == 0) {
                chunkWriteByte(chunk, BLT);
            } else if(strcmp(text, "bgt") == 0) {
                chunkWriteByte(chunk, BGT);
            }
            break;
        }
        case 'n': {
            if(strcmp(text, "nop") == 0) {
                chunkWriteByte(chunk, NOP);
            }
            break;
        }
        case 'f': {
            if(strcmp(text, "fre") == 0) {
                chunkWriteByte(chunk, FRE);
            }
            break;
        }
        case '#': {
            short value = atoi(text+1);
            chunkWriteByte(chunk, IMS);
            chunkWriteByte(chunk, value & 0xFF);
            chunkWriteByte(chunk, value >> 8);
            break;
        }
        case '$': {
            chunkWriteByte(chunk, IMB);
            chunkWriteByte(chunk, (unsigned char)atoi(text+1));
            break;
        }
        case '@': {
            chunkWriteByte(chunk, REG);
            chunkWriteByte(chunk, (unsigned char)atoi(text+1));
            break;
        }
    }
    return 0;
}

char** splitString(char* string, char* delimiter, int* count) {
    char** result = malloc(1);
    int resultCount = 0;

    char* stringCopy = malloc(strlen(string) + 1);
    strcpy(stringCopy, string);

    char* token = strtok(stringCopy, delimiter);
    while (token != NULL) {
        result = realloc(result, sizeof(char*) * (resultCount + 1));
        result[resultCount] = token;
        resultCount++;
        token = strtok(NULL, delimiter);
    }


    *count = resultCount;
    return result;
}