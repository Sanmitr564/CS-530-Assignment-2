/*
Aaron Alegre,       cssc2101, Red ID: 828222103
Patrick Stewart,    cssc2138, Red ID: 130969511
Edwin Vega,         cssc2141, Red ID: 827746186
*/

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

#ifndef LINKEDLIST
#define LINKEDLIST
#include "linkedlist.h"
#endif

#include "assembler.h"

#ifndef OPCODES
#define OPCODES
#include "instructions.h"
#endif

//return 1 if directive, 0 otherwise
static int isDirective(const Opcode *op) {
    return op != NULL && op->formats == 0;
}

//parse int
static unsigned int parseIntAuto(const char *text) {
    if (text == NULL) return 0;
    //allow 0xHEX or decimal. plain HEX if prefixed with 0x 
    //strtoul with base 0 detects 0x/0 prefix
    return (unsigned int)strtoul(text, NULL, 0);
}

static int isHexDigit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

// operand for BYTE or literal body without '='
// C'..' so: length = number of chars
// X'..' so: length = hexDigits/2. should be even
static int byteLikeLength(const char *operand) {
    if (operand == NULL || operand[0] == '\0') return 0;

    //expect it to be: C'..' or X'..'
    if ((operand[0] != 'C' && operand[0] != 'X') || operand[1] != '\'') {
        printf("Improper constant %s. Terminating.\n", operand);
        exit(39);
    }

    char type = operand[0];
    const char *content = operand + 2; //skip prefix and opening '
    const char *end = strrchr(content, '\'');
    if (end == NULL) return 0;

    int len = (int)(end - content);
    if (len < 0) return 0;

    if (type == 'C') {
        return len;
    }

    // type == 'X'
    //check all hex digits and even count
    for (int i = 0; i < len; i++) {
        if (!isHexDigit(content[i])) return 0;
    }
    if (len % 2 != 0) {
        printf("Invalid hexadecimal %s. Terminating.", operand);
        exit(36);
    }
    return len / 2;
}

//given a constant X'..'/C'..' return its value
static long getConstValue(char* operand) {
    //check formatting
    if (strlen(operand) < 4 || (operand[0] != 'X' && operand[0] != 'C') || operand[1] != '\'' || operand[strlen(operand) - 1] != '\'') {
        printf("%s is not a constant. Terminating.\n", operand);
        exit(41);
    }
    
    if (operand[0] == 'X') {
        return getHexValue(&operand[2]);
    }
    return getCharValue(operand + 2);

}

//expects string of the form "{hex chars}'"
//returns value of hex string
static long getHexValue(char* operand) {
    const char* end = strrchr(operand, '\'');
    if (end == NULL) {
        printf("Invalid hexadecimal %s. Terminating.", operand);
        exit(37);
    }

    int len = (int)(end - operand);
    if (len < 0) {
        printf("Invalid hexadecimal %s. Terminating.", operand);
        exit(38);
    }

    //check all hex digits and even count
    for (int i = 0; i < len; i++) {
        if (!isHexDigit(operand[i])) {
            printf("Invalid hexadecimal character found in %s. Terminating.", operand);
            exit(38);
        }
    }
    if (len % 2 != 0) {
        printf("Invalid hexadecimal %s. Terminating.", operand);
        exit(36);
    }
    //return value
    return strtol(operand, NULL, 16);
}

//expects string of the form "{chars}'"
//returns long of the string's byte values
static long getCharValue(char* operand) {
    long value = 0;
    unsigned char* bytes = (unsigned char*)operand;
    for (int i = 0; i < strlen(operand) - 1; i++) {
        value += bytes[i];
        value = value << 8;
    }
    value = value >> 8;
    return value;
}

//literal key is the full literal text, like: "=C'EOF'"
static LitTabEntry *findLiteral(List *list, const char *literalKey) {
    if (list == NULL || list->head == NULL || literalKey == NULL) return NULL;
    Node *cur = list->head;
    while (cur != NULL) {
        LitTabEntry *entry = (LitTabEntry*)cur->data;
        if (entry != NULL && entry->name != NULL && strcmp(entry->name, literalKey) == 0) {
            return entry;
        }
        cur = cur->nextNode;
    }
    return NULL;
}

//find and return symbol from symtable
static SymtabEntry *findSymbol(List *list, const char *label) {
    if (list == NULL || list->head == NULL || label == NULL || label[0] == '\0') return NULL;
    Node *cur = list->head;
    while (cur != NULL) {
        SymtabEntry *entry = (SymtabEntry*)cur->data;
        if (entry != NULL && entry->symbol != NULL && strcmp(entry->symbol, label) == 0) {
            return entry;
        }
        cur = cur->nextNode;
    }
    return NULL;
}

//Performs passes 1 and 2
void assemble(FILE *input, char* fileName){
    List intermediateList;
    List symtabList;
    List littabList;

    //initialize lists so addNode works
    intermediateList.head = intermediateList.tail = NULL;
    symtabList.head = symtabList.tail = NULL;
    littabList.head = littabList.tail = NULL;

    pass1(input, &intermediateList, &symtabList, &littabList);
    pass2(fileName, &intermediateList, &symtabList, &littabList);

    //free memory used by list data entries (should be the only dynamically allocated memory) to prevent leaks and prepare for next input file
    freeLists(&intermediateList, &symtabList, &littabList);
}

//reads from input and populates lists with relevant information for second pass
void pass1(FILE* input, List* intermediateList, List* symtabList, List* littabList) {
    int lineNum = 0;
    char buffer[LINE_MAX_LEN + 1];
    char* dirtyLine;                    //Prevents editing buffer (probably not necessary? its here just in case)
    int len = 0;
    int size = 0;
    int locctr = 0;
    unsigned int startAddress = 0;
    unsigned int programLength = 0;

    //Go through each line
    while (fgets(buffer, LINE_MAX_LEN + 1, input) != NULL) {
        //if line is empty get next line
        if (strlen(buffer) == 0) {
            continue;
        }

        dirtyLine = buffer;
        lineNum++;
        IntermediateRep* intermediateRep = (IntermediateRep*)calloc(1, sizeof(IntermediateRep));

        //separates [label][opcode][operand] into a more workable state
        parse(dirtyLine, lineNum, intermediateRep);

        // store and continue no change to LOCCTR 
        if (intermediateRep->comment != NULL) {
            createAndAppendNode(intermediateList, intermediateRep);
            continue;
        }

        const char* mnemonic = intermediateRep->opcode->mnemonic;

        //handles START
        if (strcmp(mnemonic, "START") == 0) {
            startAddress = parseIntAuto(intermediateRep->operand);
            locctr = startAddress;
            intermediateRep->address = locctr;
            createAndAppendNode(intermediateList, intermediateRep);

            SymtabEntry* symtabEntry = (SymtabEntry*)calloc(1, sizeof(SymtabEntry));
            strcpy(symtabEntry->csect, intermediateRep->label);
            symtabEntry->symbol[0] = '\0';
            symtabEntry->value = startAddress;
            createAndAppendNode(symtabList, symtabEntry);
            continue;
        }

        //inserts label into SYMTAB if present
        if (strlen(intermediateRep->label) > 0) {
            if (findSymbol(symtabList, intermediateRep->label) != NULL) {
                printf("Duplicate label %s on line %d. Terminating.\n", intermediateRep->label, lineNum);
                exit(11);
            }
            SymtabEntry* symtabEntry = (SymtabEntry*)calloc(1, sizeof(SymtabEntry));
            strcpy(symtabEntry->symbol, intermediateRep->label);
            symtabEntry->value = (unsigned int)locctr;
            symtabEntry->flags = strdup("R");
            symtabEntry->length = 0;
            symtabEntry->csect[0] = '\0';
            createAndAppendNode(symtabList, symtabEntry);
        }

        //record current address
        intermediateRep->address = locctr;

        //register literal if operand begins with =. Do not place yet
        if (intermediateRep->operand[0] == '=') {
            //keep the full literal text as key
            if (findLiteral(littabList, intermediateRep->operand) == NULL) {
                LitTabEntry* lit = (LitTabEntry*)calloc(1, sizeof(LitTabEntry));
                lit->name = strdup(intermediateRep->operand);
                //calculate length after =
                int litLen = byteLikeLength(intermediateRep->operand + 1);
                lit->length = litLen;
                lit->address = -1;
                lit->operand = 0; // this could hold a number value but is unused here
                createAndAppendNode(littabList, lit);
            }
        }

        int locctrIncrement = 0; //amount to add to LOCCTR

        //classify mnemonic and set how far to increment LOCCTR
        if (isDirective(intermediateRep->opcode)) {
            if (strcmp(mnemonic, "BYTE") == 0) {
                locctrIncrement = byteLikeLength(intermediateRep->operand);
            }
            else if (strcmp(mnemonic, "WORD") == 0) {
                locctrIncrement = 3;
            }
            else if (strcmp(mnemonic, "RESB") == 0) {
                locctrIncrement = (int)parseIntAuto(intermediateRep->operand);
            }
            else if (strcmp(mnemonic, "RESW") == 0) {
                locctrIncrement = 3 * (int)parseIntAuto(intermediateRep->operand);
            }
            else if (strcmp(mnemonic, "BASE") == 0 || strcmp(mnemonic, "NOBASE") == 0) {
                locctrIncrement = 0; // pass 1 bookkeeping only
            }
            else if (strcmp(mnemonic, "*") == 0) {
                //operand must be a literal we have seen or new one
                const char* litKey = intermediateRep->operand;
                if (litKey == NULL || litKey[0] != '=') {
                    printf("Invalid literal placement on line %d. Terminating.\n", lineNum);
                    exit(12);
                }
                LitTabEntry* lit = findLiteral(littabList, litKey);
                if (lit == NULL) {
                    lit = (LitTabEntry*)calloc(1, sizeof(LitTabEntry));
                    lit->name = strdup(litKey);
                    lit->length = byteLikeLength(litKey + 1);
                    lit->address = -1;
                    createAndAppendNode(littabList, lit);
                }
                if (lit->length <= 0) {
                    printf("Invalid literal on line %d. Terminating.\n", lineNum);
                    exit(13);
                }
                if (lit->address == -1) {
                    lit->address = locctr;
                    locctrIncrement = lit->length;
                }
                else {
                    locctrIncrement = 0; // already placed
                }
            }
            else if (strcmp(mnemonic, "END") == 0) {
                //Append END record
                createAndAppendNode(intermediateList, intermediateRep);

                //place all unassigned literals at current LOCCTR
                Node* cur = littabList->head;
                while (cur != NULL) {
                    LitTabEntry* lit = (LitTabEntry*)cur->data;
                    if (lit != NULL && lit->address == -1 && lit->length > 0) {
                        lit->address = locctr;
                        locctr += lit->length;
                    }
                    cur = cur->nextNode;
                }

                programLength = locctr - startAddress;
                if (((SymtabEntry*)symtabList->head->data)->csect[0] != '\0') {
                    ((SymtabEntry*)symtabList->head->data)->length = programLength;
                }
                //end pass 1

                break;
            }
            else {
                locctrIncrement = 0; //other recognized directives have no effect on LOCCTR in our pass 1
            }
        }
        else {
            //instruction size is determined by the parsed format 
            locctrIncrement = intermediateRep->format;
        }

        createAndAppendNode(intermediateList, intermediateRep);
        locctr += locctrIncrement;
    }

    //enforce program ends with END
    if (strcmp(((IntermediateRep*)intermediateList->tail->data)->opcode->mnemonic, "END") != 0) {
        printf("Program ended without END directive. Terminating.\n");
        exit(16);
    }
    //debug code for easily reading lists
    /*
    Node* debugNode = symtabList->head;
    while (debugNode != NULL) {
        debugNode = debugNode->nextNode;
    }
    */
}

//determine object code and print to output files
void pass2(char* fileName, List* intermediateList, List* symtabList, List* littabList) {
    Node* intermediateRepNode = intermediateList->head;

    //base information
    bool canBase = false;
    char* baseLabel = NULL;
    int baseAddress = 0;

    //create output files
    char listingFileName[100];
    char symtabFileName[100];
    FILE* listingFile;
    FILE* symtabFile;

    snprintf(listingFileName, sizeof(listingFileName), "%s.l", fileName);
    snprintf(symtabFileName, sizeof(symtabFileName), "%s.st", fileName);

    listingFile = fopen(listingFileName, "w");
    symtabFile = fopen(symtabFileName, "w");

    //go through each intermediateRepNode (simulates one line of the intermediary file)
    while (intermediateRepNode != NULL) {
        IntermediateRep* intermediateRep = (IntermediateRep*)intermediateRepNode->data;

        //special formatting for END
        if (intermediateRep->opcode != NULL && strcmp(intermediateRep->opcode->mnemonic, "END") == 0) {
            fprintf(listingFile, "                 END      %s\n", intermediateRep->operand);
            break;
        }

        //special formatting for comments
        if (intermediateRep->comment != NULL && strlen(intermediateRep->comment) != 0) {
            fprintf(listingFile, "%s", intermediateRep->comment);
            intermediateRepNode = intermediateRepNode->nextNode;
            continue;
        }

        fprintf(listingFile, "%04X    ", intermediateRep->address);

        //special formatting for *
        if (intermediateRep->opcode != NULL && strcmp(intermediateRep->opcode->mnemonic, "*") == 0) {
            long value = getConstValue(&intermediateRep->operand[1]);
            fprintf(listingFile, "*       %s                            %X\n", intermediateRep->operand, (unsigned int)value);
            intermediateRepNode = intermediateRepNode->nextNode;
            continue;
        }

        fprintf(listingFile, "%-8s", intermediateRep->label);

        if (intermediateRep->format == 4) {
            fprintf(listingFile, "+");
        }
        else {
            fprintf(listingFile, " ");
        }

        fprintf(listingFile, "%-8s", intermediateRep->opcode->mnemonic);
        
        //different formatting for special addressing mode indicators
        if (
            intermediateRep->operand[0] == '#' ||
            intermediateRep->operand[0] == '@' ||
            intermediateRep->operand[0] == '='
            ) {
            fprintf(listingFile, "%-26s", intermediateRep->operand);
        }
        else {
            fprintf(listingFile, " %-25s", intermediateRep->operand);
        }

        //if instruction
        if (intermediateRep->format != -1) {
            int instruction = intermediateRep->opcode->opcode;

            if (intermediateRep->format == 1) {
                //format 1 instructions don't need anything special
            }
            else if (strcmp(intermediateRep->opcode->mnemonic, "RSUB") == 0) {
                //special formatting for RSUB
                instruction += 0b11;
                instruction = instruction << ((intermediateRep->format - 1) * 8);
            }
            else if (intermediateRep->format == 2) {
                //complete object code formatting for format 2
                instruction = (instruction << 8) + format2ObjectCode(intermediateRep->operand, expectedNumRegisters(intermediateRep->opcode->mnemonic));
            }
            else {
                //figure out ni bits for format 3/4
                int ni = 0b11;
                if (intermediateRep->operand[0] == '#') {
                    ni = 0b01;
                }
                else if (intermediateRep->operand[0] == '@') {
                    ni = 0b10;
                }
                instruction += ni;
                
                //add xbpe and operand bits to object code
                if (intermediateRep->format == 3) {
                    if (intermediateRep->operand[0] == '#' ||
                        intermediateRep->operand[0] == '@'
                        ) {
                        instruction = (instruction << 16) + format3ObjectCode(&intermediateRep->operand[1], intermediateRep->address + 3, canBase, baseAddress, symtabList, littabList);
                    }
                    else {
                        instruction = (instruction << 16) + format3ObjectCode(intermediateRep->operand, intermediateRep->address + 3, canBase, baseAddress, symtabList, littabList);
                    }
                }
                else if (intermediateRep->format == 4) {
                    if (intermediateRep->operand[0] == '#' ||
                        intermediateRep->operand[0] == '@'
                        ) {
                        instruction = (instruction << 24) + format4ObjectCode(&intermediateRep->operand[1], symtabList, littabList);
                    }
                    else {
                        instruction = (instruction << 24) + format4ObjectCode(intermediateRep->operand, symtabList, littabList);
                    }
                }

            }
            //print completed object code to listing file
           fprintf(listingFile, "%0*X", intermediateRep->format * 2, instruction);
        }
        //if directive
        //most directives have unique effects
        else {
            //if base directive, tell assembler base relative addressing is allowed and what to expect for base
            if (strcmp(intermediateRep->opcode->mnemonic, "BASE") == 0) {
                canBase = true;
                SymtabEntry* symbol = findSymbol(symtabList, intermediateRep->operand);
                if (symbol == NULL) {
                    printf("Could not find symbol %s. Terminating.\n", intermediateRep->operand);
                    exit(31);
                }
                baseAddress = symbol->value;
            }
            //end base relative addressing
            else if (strcmp(intermediateRep->opcode->mnemonic, "NOBASE") == 0) {
                canBase = false;
            }
            //BYTE expects a constant so no fancy checks needed
            else if (strcmp(intermediateRep->opcode->mnemonic, "BYTE") == 0) {
                fprintf(listingFile, "%0*X", (unsigned int)byteLikeLength(intermediateRep->operand) * 2, (unsigned int)getConstValue(intermediateRep->operand));
            }
            //WORD can take a constant or a number
            else if (strcmp(intermediateRep->opcode->mnemonic, "WORD") == 0) {
                long value = 0;
                char* end;
                if (isdigit(intermediateRep->operand[0])) {
                    value = strtol(intermediateRep->operand, &end, 10);
                    if (end != intermediateRep->operand + strlen(intermediateRep->operand)) {
                        printf("%s is not a number. Terminating.\n", intermediateRep->operand);
                        exit(42);
                    }
                }
                else {
                    value = getConstValue(intermediateRep->operand);
                }
                if (value < 0 || value > 0xFFFFFF) {
                    printf("%s cannot be expressed in 3 bytes. Terminating.\n", intermediateRep->operand);
                    exit(43);
                }
                fprintf(listingFile, "%06X", (unsigned int)value);
            }
        }// end of instruction/directive printing
        fprintf(listingFile, "\n");
        intermediateRepNode = intermediateRepNode->nextNode;
    }
    printSymtab(symtabFile, symtabList, littabList);
    fclose(listingFile);
    fclose(symtabFile);
}

//Parses each line and puts the parts into intermediateRep
void parse(char *line, int lineNum, IntermediateRep *intermediateRep){
    //if line starts with '.' it is a comment
    if(line[0] == '.'){
        intermediateRep->comment = strdup(line);
        return;
    }

    //special case for '*'
    if (line[0] == '*') {
        char whitespaceCheck[LABEL_COL_LEN + 1];
        strncpy(whitespaceCheck, &line[1], LABEL_COL_LEN);
        whitespaceCheck[LABEL_COL_LEN] = '\0';

        if (strlen(strip(whitespaceCheck)) != 0) {
            printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
            exit(15);
        }

        intermediateRep->opcode = findOpcode("*");
        intermediateRep->format = -1;
        getOperandSpecial(line, lineNum, intermediateRep, 9);
        return;
    }

    //check for required whitespace
    if(
        line[8] != ' '
    ){
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(3);
    }

    //put label, opcode, operand into intermediateRep
    getLabel(line, lineNum, intermediateRep);
    getOpcode(line, lineNum, intermediateRep);
    if (intermediateRep->format == 1 || strcmp(intermediateRep->opcode->mnemonic, "RSUB") == 0) {
        return;
    }

    //if line is not long enough to have all required parts, end
    if (strlen(line) < OPCODE_COL_LEN + 1 + LABEL_COL_LEN + 2) {
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(4);
    }

    if (
        line[15] != ' ' ||
        line[16] != ' '
        ) {
        printf("Incorrect formatting on line %d. Terminating.\n", lineNum);
        exit(16);
    }
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
        valid = valid && (isalnum(label[i]) || label[i] == '_');
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

//Gets operand from a line for most cases
void getOperand(char *line, int lineNum, IntermediateRep *intermediateRep){
    getOperandSpecial(line, lineNum, intermediateRep, 17);
}

//added specifically for '*' directive
void getOperandSpecial(char* line, int lineNum, IntermediateRep* intermediateRep, int startIndex) {
    char operand[OPERAND_COL_LEN + 1];
    strncpy(operand, &line[startIndex], OPERAND_COL_LEN);
    operand[OPERAND_COL_LEN] = '\0';
    char* strippedOperand = strip(operand);
    strcpy(intermediateRep->operand, strippedOperand);
}

//takes operand and the number of expected registers as arguments
//returns bottom 8 bits of the instruction
int format2ObjectCode(char* operand, int expected) {
    if (strlen(operand) == 1 && expected == 1) {
        int code = getRegisterCode(operand[0]);
        if (code == -1) {
            printf("Register %c does not exist or cannot be accessed in this way. Terminating.\n", operand[0]);
            exit(18);
        }
        return code << 4;
    }

    if (strlen(operand) == 3 && expected == 2) {
        if (operand[1] != ',') {
            printf("Incorrectly formatted format 2 operand %s. Terminating.\n", operand);
            exit(19);
        }
        int code1 = getRegisterCode(operand[0]);
        int code2 = getRegisterCode(operand[2]);

        if (code1 == -1 || code2 == -1) {
            printf("Register %c or %c does not exist or cannot be accessed in this way. Terminating.\n", operand[0], operand[2]);
            exit(20);
        }

        return (code1 << 4) + code2;
    }

    printf("Incorrect format 2 operand %s. Terminating.\n", operand);
    exit(17);
}

//gets register codes
int getRegisterCode(char c) {
    switch (c) {
        case 'A':
            return 0;
        case 'X':
            return 1;
        case 'L':
            return 2;
        case 'B':
            return 3;
        case 'S':
            return 4;
        case 'T':
            return 5;
        case 'F':
            return 6;
        default:
            return -1;
    }
        
}

//takes in operand, pc counter, base information, symtab, and littab
//returns xbpe and displacement bits
int format3ObjectCode(char* operand, int pc, bool canBase, int baseAddress, List* symtabList, List* littabList) {
    int xbpe = 0b0000;
    int len = strlen(operand);
    int address = 0;

    //literals cant be indexed
    if (isIndexed(operand)) {
        if (operand[0] == '=') {
            printf("Illegal addressing mode for operand %s. Terminating.\n", operand);
            exit(25);
        }
        xbpe += 0b1000;
    }

    //determine if operand is integer
    if (isdigit(operand[0])) {
        int i = 0;
        while (isdigit(operand[++i]));
        if (isIndexed(operand)) {
            if (i != len - 1) {
                printf("Improper operand %s. Terminating.", operand);
                exit(26);
            }
        }
        else {
            if (i != len) {
                printf("Improper operand %s. Terminating.", operand);
                exit(27);
            }
        }
        address = atoi(operand);
        if (address > 4095) {
            printf("Value %d not representable with format 3. Terminating.\n", address);
            exit(29);
        }
        return (xbpe << 12) + address;
    }
    
    //otherwise operand is symbol or literal
    char label[OPERAND_COL_LEN + 1];
    if (isIndexed(operand)) {
        strncpy(label, operand, OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2);
        label[OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2] = '\0';
    }
    else {
        strncpy(label, operand, OPERAND_COL_LEN);
        label[OPERAND_COL_LEN] = '\0';
    }
    
    //find symbol or literal and get address
    SymtabEntry* symEntry = findSymbol(symtabList, label);
    LitTabEntry* litEntry = findLiteral(littabList, label);
    if (symEntry == NULL && litEntry == NULL) {
        printf("Symbol %s not found. Terminating.\n", label);
        exit(28);
    }
    
    if (symEntry != NULL) {
        address = symEntry->value;
    }
    else {
        address = litEntry->address;
    }
   
    //determine if address can be reached
    int offset = address - pc;
    if (offset >= -2048 && offset <= 2047) {
        xbpe += 0b0010;
        return (xbpe << 12) + (offset & 0xfff);
    }
    else if (canBase) {
        int baseoffset = address - baseAddress;
        if (baseoffset >= 0 && baseoffset < 4096) {
            xbpe += 0b0100;
            return (xbpe << 12) + baseoffset;
        }
    }

    printf("Cannot reach address designated by %s with a format 3 instruction. Terminating.\n", operand);
    exit(30);
}

//takes in operand, symtabm and littab
//returns xbpe and address
int format4ObjectCode(char* operand, List* symtabList, List* littabList) {
    int xbpe = 0b0001;
    int len = strlen(operand);
    int address = 0;

    //literals cant be indexed
    if (isIndexed(operand)) {
        if (operand[0] == '=') {
            printf("Illegal addressing mode for operand %s. Terminating.\n", operand);
            exit(32);
        }
        xbpe += 0b1000;
    }

    //determine if operand is integer
    if (isdigit(operand[0])) {
        int i = 0;
        while (isdigit(operand[++i]));
        if (isIndexed(operand)) {
            if (i != len - 1) {
                printf("Improper operand %s. Terminating.", operand);
                exit(33);
            }
        }
        else {
            if (i != len) {
                printf("Improper operand %s. Terminating.", operand);
                exit(34);
            }
        }
        address = atoi(operand);
        return (xbpe << 20) + address;
    }

    //otherwise operand is symbol or literal
    char label[OPERAND_COL_LEN + 1];
    if (isIndexed(operand)) {
        strncpy(label, operand, OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2);
        label[OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2] = '\0';
    }
    else {
        strncpy(label, operand, OPERAND_COL_LEN);
        label[OPERAND_COL_LEN] = '\0';
    }

    //find symbol or literal and get address
    SymtabEntry* symEntry = findSymbol(symtabList, label);
    LitTabEntry* litEntry = findLiteral(littabList, label);
    if (symEntry == NULL && litEntry == NULL) {
        printf("Symbol %s not found. Terminating.\n", label);
        exit(35);
    }

    if (symEntry != NULL) {
        address = symEntry->value;
    }
    else {
        address = litEntry->address;
    }

    //assume address can be reached as its format 4
    return (xbpe << 20) + address;
}

//determine if operand uses indexed addressing
bool isIndexed(char* str) {
    return strlen(str) > 2 && str[strlen(str) - 1] == 'X' && str[strlen(str) - 2] == ',';
}

//returns the expected number of registers for format 2 instructions
int expectedNumRegisters(const char* operand) {
    if (
        strcmp(operand, "CLEAR") == 0 ||
        strcmp(operand, "SVC") == 0 ||
        strcmp(operand, "TIXR") == 0
        ) {
        return 1;
    }
    return 2;
}

//frees memory used by nodes and structs in the nodes
void freeLists(List* intermediateList, List* symtabList, List* littabList) {
    Node* node = intermediateList->head;
    while (node != NULL) {
        Node* nextNode = node->nextNode;
        IntermediateRep* intermediateRep = (IntermediateRep*)node->data;
        free(intermediateRep->comment);
        free(node);
        node = nextNode;
    }

    node = symtabList->head;
    while (node != NULL) {
        Node* nextNode = node->nextNode;
        SymtabEntry* symtabEntry = (SymtabEntry*)node->data;
        free(symtabEntry->flags);
        free(node);
        node = nextNode;
    }

    node = littabList->head;
    while (node != NULL) {
        Node* nextNode = node->nextNode;
        LitTabEntry* litTabEntry = (LitTabEntry*)node->data;
        free(litTabEntry->name);
        free(node);
        node = nextNode;
    }
}

//print symtab and littab
void printSymtab(FILE* symtabFile, List *symtabList, List *littabList) {
    //print symtable
    fprintf(symtabFile, "CSect   Symbol  Value   LENGTH  Flags:\n");
    fprintf(symtabFile, "--------------------------------------\n");
    Node* node = symtabList->head;
    while (node != NULL) {
        SymtabEntry* entry = (SymtabEntry*)node->data;
        if (entry->csect != NULL && strlen(entry->csect) != 0) {
            fprintf(symtabFile, "%-16s", entry->csect);
            fprintf(symtabFile, "%06X  %06X\n", entry->value, entry->length);
        }
        else {
            fprintf(symtabFile, "        %-8s%06X          %s\n", entry->symbol, entry->value, entry->flags);
        }
        node = node->nextNode;
    }

    //print littable
    fprintf(symtabFile, "\nLiteral Table\n");
    fprintf(symtabFile, "Name  Operand   Address  Length:\n");
    fprintf(symtabFile, "--------------------------------\n");
    node = littabList->head;
    while (node != NULL) {
        LitTabEntry* entry = (LitTabEntry*)node->data;
        long value = getConstValue(&entry->name[1]);
        char* name = strtok(entry->name + 3, "\'");
        fprintf(symtabFile, "%-6s%06X    %-9X%d\n", name, (unsigned int)value, entry->address, entry->length);
        node = node->nextNode;
    }
}
