#include <stdio.h>
#include <stdbool.h>

#ifndef OPCODES
#define OPCODES
#include "instructions.h"
#endif

#ifndef LINKEDLIST
#define LINKEDLIST
#include "linkedlist.h"
#endif

#define LABEL_MAX_LEN       6
#define OPCODE_MAX_LEN      6
#define LABEL_COL_LEN       8
#define OPCODE_COL_LEN      8
#define OPERAND_COL_LEN     18
#define LINE_MAX_LEN        66

//Representation of the intermediate file between pass 1 and 2
typedef struct{
    unsigned int address;
    char label[LABEL_MAX_LEN + 1];
    Opcode *opcode;
    int format;
    char operand[OPERAND_COL_LEN + 1];
    char *comment;
}IntermediateRep;

//Representation of a symtable entry
typedef struct {
    char csect[LABEL_MAX_LEN + 1];
    char symbol[LABEL_MAX_LEN + 1];
    unsigned int value;
    unsigned int length;
    char *flags;
}SymtabEntry;

//Representation of a Littable entry
typedef struct {
    char *name;
    int operand;
    int address;
    int length;
}LitTabEntry;

void assemble(FILE *input, char* fileName);
void pass1(FILE* input, List* intermediateList, List* symtabList, List* littabList);
void pass2(List* intermediateList, List* symtabList, List* littabList);

void parse(char *line, int lineNum, IntermediateRep *intermediateRep);

char* strip(char *line);

bool labelValidate(char* label);

void getLabel(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOpcode(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOperandSpecial(char* line, int lineNum, IntermediateRep* intermediateRep, int startIndex);
int format2ObjectCode(char* operand, int expected);
int getRegisterCode(char c);
int format3ObjectCode(char* operand, int pc, bool canBase, int baseAddress, List* symtabList, List* littabList);
int format4ObjectCode(char* operand, List* symtabList, List* littabList);
bool isIndexed(char* str);
int expectedNumRegisters(char* operand);
void freeLists(List* intermediateList, List* symtabList, List* littabList);