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
    char* dirtyLine;                    //Prevents editing buffer (probably not necessary? its here just in case)
    int len = 0;
    int size = 0;
    int locctr = 0;

    //Go through each line
    while(fgets(buffer, LINE_MAX_LEN + 1, input) != NULL){
        //if line is empty get next line
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

//Parses each line and puts the parts into intermediateRep
void parse(char *line, int lineNum, IntermediateRep *intermediateRep){
    //if line starts with '.' it is a comment
    if(line[0] == '.'){
        intermediateRep->comment = strdup(line);
        return;
    }

    //if line is not long enough to have all required parts, end
    if(strlen(line) < OPCODE_COL_LEN + 1 + LABEL_COL_LEN + 2){
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(4);
    }

    //check for required whitespace
    if(
        line[8] != ' ' ||
        line[15] != ' ' ||
        line[16] != ' '
    ){
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(3);
    }

    //put label, opcode, operand into intermediateRep
    getLabel(line, lineNum, intermediateRep);
    getOpcode(line, lineNum, intermediateRep);
    getOperand(line, lineNum, intermediateRep);
}

//Strips leading and trailing whitespace from string
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

//Makes sure label has correct format
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

//Gets the label from the line
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

//Gets opcode from a line
void getOpcode(char *line, int lineNum, IntermediateRep *intermediateRep){
    char opCode[OPCODE_COL_LEN + 1];
    strncpy(opCode, line + 9, OPCODE_COL_LEN);
    opCode[OPCODE_COL_LEN] = '\0';
    char *strippedOpCode = strip(opCode);

    if(opCode[0] == '+'){
        intermediateRep->format = 4;
        intermediateRep->opcode = findOpcode(strippedOpCode + 1);
    }else{
        intermediateRep->format = -1;
        intermediateRep->opcode = findOpcode(strippedOpCode);
    }

    if(intermediateRep->opcode == NULL){
        printf("Could not find opcode %s. Terminating.\n", strippedOpCode);
        exit(6);
    }

    if (intermediateRep->opcode->formats == 0) {
        return;
    }

    if(intermediateRep->format == 4){
        if (!(intermediateRep->opcode->formats & FMT4)) {
            printf("%s does not support format 4 opcode. Terminating.\n", strippedOpCode);
            exit(7);
        }
        return;
    }

    

    switch(intermediateRep->opcode->formats & (FMT1 | FMT2 | FMT3)){
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

//Gets operand from a line
void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep){
    char operand[OPERAND_COL_LEN + 1];
    strncpy(operand, &line[17], OPERAND_COL_LEN);
    operand[OPERAND_COL_LEN] = '\0';
    char *strippedOperand = strip(operand);
    strcpy(intermediateRep->operand, strippedOperand);
}