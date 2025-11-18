#include <stdio.h>
#include <stdbool.h>

#ifndef OPCODES
#define OPCODES
#include "instructions.h"
#endif

#define LABEL_MAX_LEN       6
#define OPCODE_MAX_LEN      6
#define LABEL_COL_LEN       8
#define OPCODE_COL_LEN      8
#define OPERAND_COL_LEN     18
#define LINE_MAX_LEN        66

typedef struct{
    unsigned int address;
    char label[LABEL_MAX_LEN + 1];
    Instruction *instruction;
    int format;
    char operand[OPERAND_COL_LEN + 1];
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

void parse(char *line, int lineNum, IntermediateRep *intermediateRep);

char* strip(char *line);

bool labelValidate(char* label);

void getLabel(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOpcode(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep);