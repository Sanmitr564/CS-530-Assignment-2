#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "linkedlist.h"
#include "assembler.h"

void assemble(FILE *input, FILE *output){
    char *line = NULL;
    int len = 0;
    int size = 0;

    while((size = getline(&line, &len, input) != -1)){
        IntermediateRep *intermediateRep = NULL;
        SymtabEntry *symtabEntry = NULL;
        LitTabEntry *litTabEntry = NULL;

        parse(line, intermediateRep);

    }
}

void parse(char *line, IntermediateRep *intermediateRep){
    bool hasLabel = isspace(line[0]);


}