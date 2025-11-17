#include <stdio.h>

#define LABEL_MAX_LEN       6
#define OPCODE_MAX_LEN      6

typedef struct{
    unsigned int address;
    char label[LABEL_MAX_LEN];
    char opcode[OPCODE_MAX_LEN];
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