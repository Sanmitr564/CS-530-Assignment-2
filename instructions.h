#define MNEMONIC_MAX_LEN    6
#define NUM_INSTRUCTIONS    59
#define NUM_DIRECTIVES      14

// Bitmaps for format flags
#define FMT1 0b1000
#define FMT2 0b0100
#define FMT3 0b0010
#define FMT4 0b0001

//Instruction type definition
struct Instruction_Struct{
    char mnemonic[MNEMONIC_MAX_LEN + 1];
    unsigned int formats;   //0b[1][2][3][4] T/F flag for formats
    unsigned int opcode;    //opcode in hex
};

//Assembler Directive ("Directive") type definition
struct Directive_Struct{
    char mnemonic[MNEMONIC_MAX_LEN + 1];
};

typedef struct Instruction_Struct Instruction;
typedef struct Directive_Struct Directive;

//Assembler Instruction list
#pragma once
extern const Instruction instructions[];

//Assembler Directive list:
#pragma once
extern const Directive directives[];


//Search methods (recognizes instruction/directive input)
Instruction *findInstruction(char *mnemonic);

Directive *findDirective(char *mnemonic);