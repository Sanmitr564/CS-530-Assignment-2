#define MNEMONIC_MAX_LEN    6
#define NUM_INSTRUCTIONS    59
#define NUM_DIRECTIVES      14

// Bitmaps for format flags
#define FMT1 0b1000
#define FMT2 0b0100
#define FMT3 0b0010
#define FMT4 0b0001

//Instruction type definition
typedef struct{
struct Instruction_Struct{
    char mnemonic[MNEMONIC_MAX_LEN + 1];
    unsigned int formats;   //0b[1][2][3][4] T/F flag for formats
    unsigned int opcode;    //opcode in hex
};

struct Directive_Struct{
    char mnemonic[MNEMONIC_MAX_LEN + 1];

};

//Assembler Directive list:
Directive directives[] = {
    {"START" },
    {"END"   },
    {"RESB"  },
    {"RESW"  },
    {"BYTE"  },
    {"WORD"  },
    {"BASE"  },
    {"NOBASE"},
    {"*"     },
    {"LTORG" },
    {"USE"   },    
    {"CSECT" },
    {"EXTDEF"},
    {"EXTREF"},
};


//Search methods (recognizes instruction/directive input)
Instruction findInstruction(char *mnemonic);

Directive findDirective(char *mnemonic);
typedef struct Instruction_Struct Instruction;
typedef struct Directive_Struct Directive;

#pragma once
extern const Instruction instructions[];

Instruction *findInstruction(char *mnemonic);