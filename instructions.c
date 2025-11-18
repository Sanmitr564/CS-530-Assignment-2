#include <stdlib.h>
#include <string.h>

#ifndef OPCODES
#define OPCODES
#include "instructions.h"
#endif

//Instruction list and their opcodes:
const Instruction instructions[] = {
    {"ADD",     FMT3 | FMT4,    0x18},
    {"ADDF",    FMT3 | FMT4,    0x58},
    {"ADDR",    FMT2,           0x90},
    {"AND",     FMT3 | FMT4,    0x40},
    {"CLEAR",   FMT2,           0xB4},
    {"COMP",    FMT3 | FMT4,    0x28},
    {"COMPF",   FMT3 | FMT4,    0x88},
    {"COMPR",   FMT2,           0xA0},
    {"DIV",     FMT3 | FMT4,    0x24},
    {"DIVF",    FMT3 | FMT4,    0x64},
    {"DIVR",    FMT2,           0x9C},
    {"FIX",     FMT1,           0xC4},
    {"FLOAT",   FMT1,           0xC0},
    {"HIO",     FMT1,           0xF4},
    {"J",       FMT3 | FMT4,    0x3C},
    {"JEQ",     FMT3 | FMT4,    0x30},
    {"JGT",     FMT3 | FMT4,    0x34},
    {"JLT",     FMT3 | FMT4,    0x38},
    {"JSUB",    FMT3 | FMT4,    0x48},
    {"LDA",     FMT3 | FMT4,    0x00},
    {"LDB",     FMT3 | FMT4,    0x68},
    {"LDCH",    FMT3 | FMT4,    0x50},
    {"LDF",     FMT3 | FMT4,    0x70},
    {"LDL",     FMT3 | FMT4,    0x08},
    {"LDS",     FMT3 | FMT4,    0x6C},
    {"LDT",     FMT3 | FMT4,    0x74},
    {"LDX",     FMT3 | FMT4,    0x04},
    {"LPS",     FMT3 | FMT4,    0xD0},
    {"MUL",     FMT3 | FMT4,    0x20},
    {"MULF",    FMT3 | FMT4,    0x60},
    {"MULR",    FMT2,           0x98},
    {"NORM",    FMT1,           0xC8},
    {"OR",      FMT3 | FMT4,    0x44},
    {"RD",      FMT3 | FMT4,    0xD8},
    {"RMO",     FMT2,           0xAC},
    {"RSUB",    FMT3 | FMT4,    0x4C},
    {"SHIFTL",  FMT2,           0xA4},
    {"SHIFTR",  FMT2,           0xA8},
    {"SIO",     FMT1,           0xF0},
    {"SSK",     FMT3 | FMT4,    0xEC},
    {"STA",     FMT3 | FMT4,    0x0C},
    {"STB",     FMT3 | FMT4,    0x78},
    {"STCH",    FMT3 | FMT4,    0x54},
    {"STF",     FMT3 | FMT4,    0x80},
    {"STI",     FMT3 | FMT4,    0xD4},
    {"STL",     FMT3 | FMT4,    0x14},
    {"STS",     FMT3 | FMT4,    0x7C},
    {"STSW",    FMT3 | FMT4,    0xE8},
    {"STT",     FMT3 | FMT4,    0x84},
    {"STX",     FMT3 | FMT4,    0x10},
    {"SUB",     FMT3 | FMT4,    0x1C},
    {"SUBF",    FMT3 | FMT4,    0x5C},
    {"SUBR",    FMT2,           0x94},
    {"SVC",     FMT2,           0xB0},
    {"TD",      FMT3 | FMT4,    0xE0},
    {"TIO",     FMT1,           0xF8},
    {"TIX",     FMT3 | FMT4,    0x2C},
    {"TIXR",    FMT2,           0xB8},
    {"WD",      FMT3 | FMT4,    0xDC}
};

const Directive directives[] = {
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

Instruction* findInstruction(char *mnemonic){
    for(int i = 0; i < NUM_INSTRUCTIONS; i++){
        if(strcmp(mnemonic, instructions[i].mnemonic) == 0) {
            return &instructions[i];
        }
    }
    return NULL;
}

Directive* findDirective(char *mnemonic){
    for(int i = 0; i < NUM_DIRECTIVES; i++){
        if(strcmp(mnemonic, directives[i].mnemonic) == 0) {
            return &directives[i];
        }
    }
    return NULL;
}
