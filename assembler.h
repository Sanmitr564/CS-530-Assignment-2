#include <stdio.h>

#include "instructions.h"

#define LABEL_MAX_LEN       6
#define OPCODE_MAX_LEN      6

typedef struct{
    unsigned int address;
    char label[LABEL_MAX_LEN + 1];
    Instruction *instruction;
    int format;
    char *operand;
    char *comment;
}IntermediateRep;

typedef struct {
    char *csect;
    char *symbol;
    unsigned int value;
    unsigned int length;
    char *flags;
}SymtabEntry;

typedef struct {
    char *name;
    int operand;
    int address;
    int length;
}LitTabEntry;


void assemble(FILE *input, FILE *output);

void lstrip(char *line);


