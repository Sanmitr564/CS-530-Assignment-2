#define _CRT_SECURE_NO_WARNINGS

#ifndef POSIX_COMPAT_H
#define POSIX_COMPAT_H

#include <string.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

#endif

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

#include "linkedlist.h"
#include "assembler.h"

#ifndef OPCODES
#define OPCODES
#include "instructions.h"
#endif

void assemble(FILE *input, FILE *output){
    int lineNum = 0;
    char buffer[LINE_MAX_LEN + 1];
    char* dirtyLine;
    int len = 0;
    int size = 0;
    int locctr = 0;

    while(fgets(buffer, LINE_MAX_LEN + 1, input) != NULL){
        if(strlen(buffer) == 0){
            continue;
        }

        dirtyLine = buffer;
        lineNum++;
        IntermediateRep *intermediateRep = (IntermediateRep*)malloc(sizeof(IntermediateRep));
        SymtabEntry *symtabEntry = (SymtabEntry*)malloc(sizeof(SymtabEntry));
        LitTabEntry *litTabEntry = (LitTabEntry*)malloc(sizeof(LitTabEntry));

        parse(dirtyLine, lineNum, intermediateRep);

    }
}

void parse(char *line, int lineNum, IntermediateRep *intermediateRep){
    if(line[0] == '.'){
        intermediateRep->comment = strdup(line);//TODO: fix
        return;
    }

    if(strlen(line) < OPCODE_COL_LEN + 1 + LABEL_COL_LEN + 2){
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(4);
    }

    if(
        line[8] != ' ' ||
        line[15] != ' ' ||
        line[16] != ' '
    ){
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(3);
    }

    getLabel(line, lineNum, intermediateRep);
    getOpcode(line, lineNum, intermediateRep);
    getOperand(line, lineNum, intermediateRep);


}

char* strip(char *str){
    int i = 0;
    int j = 0;
    int end = strlen(str);
    while(isspace(str[i])){
        i++;
    }

    char *strippedStr = str + i;

    i = end - 1;
    while(isspace(str[i]) && i > 0){
        i--;
    }
    i++;
    str[i] = '\0';
    return strippedStr;
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

void getLabel(char *line, int lineNum, IntermediateRep *intermediateRep){
    char label[LABEL_COL_LEN + 1];
    strncpy(label, line, LABEL_COL_LEN);
    label[LABEL_COL_LEN] = '\0';
    char *strippedLabel = strip(label);
    if(strlen(label) > LABEL_MAX_LEN + 1){
        printf("Label on line %d longer than %d characters. Terminating.\n", lineNum, LABEL_MAX_LEN);
        exit(5);
    }

    strcpy(intermediateRep->label, strippedLabel);

    if(strlen(intermediateRep->label) > 0 && !labelValidate(intermediateRep->label)){
        printf("Label %s on line %d is invalid. Terminating.\n", intermediateRep->label, lineNum);
        exit(8);
    }
}

void getOpcode(char *line, int lineNum, IntermediateRep *intermediateRep){
    char opCode[OPCODE_COL_LEN + 1];
    strncpy(opCode, line + 9, OPCODE_COL_LEN);
    opCode[OPCODE_COL_LEN] = '\0';
    char *strippedOpCode = strip(opCode);

    if(opCode[0] == '+'){
        intermediateRep->format = 4;
        intermediateRep->instruction = findInstruction(strippedOpCode + 1);
    }else{
        intermediateRep->format = -1;
        intermediateRep->instruction = findInstruction(strippedOpCode);
    }

    if(intermediateRep->instruction == NULL){
        printf("Could not find opcode %s. Terminating.\n", strippedOpCode);
        exit(6);
    }

    if(intermediateRep->format == 4){
        if (!(intermediateRep->instruction->formats & FMT4)) {
            printf("%s does not support format 4 instruction. Terminating.\n", strippedOpCode);
            exit(7);
        }
        return;
    }

    

    switch(intermediateRep->instruction->formats & (FMT1 | FMT2 | FMT3)){
        case FMT1:
            intermediateRep->format = 1;
            break;

        case FMT2:
            intermediateRep->format = 2;
            break;

        case FMT3:
            intermediateRep->format = 3;
            break;

        default:
            printf("%s format not found. Terminating.\n", opCode);
            exit(9);
    }
}

void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep){
    char operand[OPERAND_COL_LEN + 1];
    strncpy(operand, &line[17], OPERAND_COL_LEN);
    operand[OPERAND_COL_LEN] = '\0';
    char *strippedOperand = strip(operand);
    strcpy(intermediateRep->operand, strippedOperand);
}