/*
Aaron Alegre,       cssc2101, Red ID: 828222103
Patrick Stewart,    cssc2138, Red ID: 130969511
Edwin Vega,         cssc2141, Red ID: 827746186
*/

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
//each struct represents one line of the intermediary file
typedef struct{
    unsigned int address;
    char label[LABEL_MAX_LEN + 1];
    const Opcode *opcode;
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

//expected use of assembler.c
//takes input file and filename for naming output files
void assemble(FILE *input, char* fileName);

//pass 1 and 2
void pass1(FILE* input, List* intermediateList, List* symtabList, List* littabList);
void pass2(char* fileName, List* intermediateList, List* symtabList, List* littabList);

//parses each line and puts into an intermediateRep
void parse(char *line, int lineNum, IntermediateRep *intermediateRep);

//used by parse
void getLabel(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOpcode(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep);
void getOperandSpecial(char* line, int lineNum, IntermediateRep* intermediateRep, int startIndex);

//used by getLabel
bool labelValidate(char* label);

//helper methods
char* strip(char *line);
static long getCharValue(char* operand);
static long getHexValue(char* operand);

//format 2 helper methods
int format2ObjectCode(char* operand, int expected);
int expectedNumRegisters(const char* operand);
int getRegisterCode(char c);

//format 3/4 helper methods
int format3ObjectCode(char* operand, int pc, bool canBase, int baseAddress, List* symtabList, List* littabList);
int format4ObjectCode(char* operand, List* symtabList, List* littabList);
bool isIndexed(char* str);

//print symtab and littab at end of pass 2
void printSymtab(FILE* symtabFile, List* symtabList, List* littabList);

//free lists at end of assemble
void freeLists(List* intermediateList, List* symtabList, List* littabList);