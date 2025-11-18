#include <stdlib.h>
#include <string.h>

#include "instructions.h"

Instruction* findInstruction(char *mnemonic){
    for(int i = 0; i < NUM_INSTRUCTIONS; i++){
        if(strcmp(mnemonic, instructions->mnemonic) == 0){
            return &instructions[i];
        }
    }
    return NULL;
}