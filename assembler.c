#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "linkedlist.h"
#include "assembler.h"
#include "instructions.h"

void assemble(FILE *input, FILE *output){
    int lineNum = 0;
    char *line = NULL;
    char *dirtyLine = NULL;
    int len = 0;
    int size = 0;
    int locctr = 0;

    while((size = getline(&line, &len, input) != -1)){
        dirtyLine = line;
        lineNum++;
        IntermediateRep *intermediateRep = (IntermediateRep*)malloc(sizeof(IntermediateRep));
        SymtabEntry *symtabEntry = (SymtabEntry*)malloc(sizeof(SymtabEntry));
        LitTabEntry *litTabEntry = (LitTabEntry*)malloc(sizeof(LitTabEntry));

        parse(dirtyLine, lineNum, intermediateRep);

    }

    free(line);
}

void parse(char *line, int lineNum, IntermediateRep *intermediateRep){
    bool hasLabel = isspace(line[0]);
    const char *delimiter = " ";
    char *token;

    if(!hasLabel){
        token = strtok(line, delimiter);
        if(strlen(token) > 6){
            printf("Label on line %d longer than 6 characters. Terminating.\n", lineNum);
            exit(2);
        }
        if(!labelValidate(token)){
            printf("Label on line %d is invalid. Terminating.\n", lineNum);
            exit(3);
        }
        strcpy(intermediateRep->label, token);

    }else if(line[0] == '.'){
        intermediateRep->comment = strdup(line);
        return;
    }else{
        lstrip(line);
    }

    token = strtok(line, delimiter);
    bool extension = false;
    if(token[0] == '+'){
        extension = true;
    }else if(!isalpha(token[0])){
        printf("Opcode on line %d is invalid. Terminating.\n", lineNum);
        exit(1);
    }

}

void lstrip(char *str){
    int i = 0;
    int j = 0;
    while(isspace(str[i])){
        i++;
    }

    if(i != 0){
        while(str[i] != '\0'){
            str[i] = str[j];
            i++;
            j++;
        }
        str[j] = '\0';
    }
}

bool labelValidate(char* label){
    bool valid = true;
    if(!isalpha(label[0])){
        return false;
    }
    for(int i = 1; i < strlen(label); i++){
        valid = valid && isalnum(label[i]);
    }
    return valid;
}