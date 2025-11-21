/*
Aaron Alegre,       cssc2101, Red ID: 828222103
Patrick Stewart,    cssc2138, Red ID: 130969511
Edwin Vega,         cssc2141, Red ID: 827746186
*/

#define MNEMONIC_MAX_LEN    6
#define NUM_INSTRUCTIONS    59
#define NUM_DIRECTIVES      14

// Bitmaps for format flags
#define FMT1 0b1000
#define FMT2 0b0100
#define FMT3 0b0010
#define FMT4 0b0001

//Opcode type definition
struct Opcode_Struct{
    char mnemonic[MNEMONIC_MAX_LEN + 1];
    unsigned int formats;   //0b[1][2][3][4] T/F flag for formats
    unsigned int opcode;    //opcode in hex
};

typedef struct Opcode_Struct Opcode;

//Assembler Opcode list
#pragma once
extern const Opcode opcodes[];

//Search methods (recognizes opcode/directive input)
const Opcode *findOpcode(char *mnemonic);
