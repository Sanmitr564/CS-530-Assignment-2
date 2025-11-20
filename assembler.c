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


static int isDirective(const Opcode *op) {
    return op != NULL && op->formats == 0;
}

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
        return 0;
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

    // type == 'X'
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
    return strtol(operand, operand + len - 1, 16);
}

static long getCharValue(char* operand) {

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

//global storage for pass 1 results. In memory for pass 2
static List intermediateList;
static List symtabList;
static List littabList;

//Performs pass 1
void assemble(FILE *input, FILE *output){
    int lineNum = 0;
    char buffer[LINE_MAX_LEN + 1];
    char* dirtyLine;                    //Prevents editing buffer (probably not necessary? its here just in case)
    int len = 0;
    int size = 0;
    int locctr = 0;
    unsigned int startAddress = 0;
    unsigned int programLength = 0;

    //initialize lists with heads so addNode works
    intermediateList.head = intermediateList.tail = NULL;
    symtabList.head = symtabList.tail = NULL;
    littabList.head = littabList.tail = NULL;

    //Go through each line
    while(fgets(buffer, LINE_MAX_LEN + 1, input) != NULL){
        //if line is empty get next line
        if(strlen(buffer) == 0){
            continue;
        }

        dirtyLine = buffer;
        lineNum++;
        IntermediateRep *intermediateRep = (IntermediateRep*)calloc(1, sizeof(IntermediateRep));

        parse(dirtyLine, lineNum, intermediateRep);
        
        // store and continue no change to LOCCTR 
        if (intermediateRep->comment != NULL) {
            createAndAppendNode(&intermediateList, intermediateRep);
            continue;
        }

        const char *mnemonic = intermediateRep->opcode->mnemonic;

        //handles START
        if (strcmp(mnemonic, "START") == 0) {
            startAddress = parseIntAuto(intermediateRep->operand);
            locctr = startAddress;
            intermediateRep->address = locctr;
            createAndAppendNode(&intermediateList, intermediateRep);

            SymtabEntry* symtabEntry = (SymtabEntry*)calloc(1, sizeof(SymtabEntry));
            strcpy(symtabEntry->csect, intermediateRep->label);
            symtabEntry->symbol[0] = '\0';
            symtabEntry->value = startAddress;
            createAndAppendNode(&symtabList, symtabEntry);
            continue;
        }

        //inserts label into SYMTAB if present
        if (strlen(intermediateRep->label) > 0) {
            if (findSymbol(&symtabList, intermediateRep->label) != NULL) {
                printf("Duplicate label %s on line %d. Terminating.\n", intermediateRep->label, lineNum);
                exit(11);
            }
            SymtabEntry *symtabEntry = (SymtabEntry*)calloc(1, sizeof(SymtabEntry));
            strcpy(symtabEntry->symbol, intermediateRep->label);
            symtabEntry->value = (unsigned int)locctr;
            symtabEntry->flags = strdup("R");
            symtabEntry->length = 0;
            symtabEntry->csect[0] = '\0';
            createAndAppendNode(&symtabList, symtabEntry);
        }

        //record current address
        intermediateRep->address = locctr;

        //register literal if operand begins with =. Do not place yet
        if (intermediateRep->operand[0] == '=') {
            //keep the full literal text as key
            if (findLiteral(&littabList, intermediateRep->operand) == NULL) {
                LitTabEntry *lit = (LitTabEntry*)calloc(1, sizeof(LitTabEntry));
                lit->name = strdup(intermediateRep->operand);
                //calculate length after =
                int litLen = byteLikeLength(intermediateRep->operand + 1);
                lit->length = litLen;
                lit->address = -1;
                lit->operand = 0; // this could hold a number value but is unused here
                createAndAppendNode(&littabList, lit);
            }
        }

        int locctrIncrement = 0; //amount to add to LOCCTR

        //classify mnemonic and set how far to increment LOCCTR
        if (isDirective(intermediateRep->opcode)) {
            if (strcmp(mnemonic, "BYTE") == 0) {
                locctrIncrement = byteLikeLength(intermediateRep->operand);
            } else if (strcmp(mnemonic, "WORD") == 0) {
                locctrIncrement = 3;
            } else if (strcmp(mnemonic, "RESB") == 0) {
                locctrIncrement = (int)parseIntAuto(intermediateRep->operand);
            } else if (strcmp(mnemonic, "RESW") == 0) {
                locctrIncrement = 3 * (int)parseIntAuto(intermediateRep->operand);
            } else if (strcmp(mnemonic, "BASE") == 0 || strcmp(mnemonic, "NOBASE") == 0) {
                locctrIncrement = 0; // pass 1 bookkeeping only
            } else if (strcmp(mnemonic, "*") == 0) {
                //operand must be a literal we have seen or new one
                const char *litKey = intermediateRep->operand;
                if (litKey == NULL || litKey[0] != '=') {
                    printf("Invalid literal placement on line %d. Terminating.\n", lineNum);
                    exit(12);
                }
                LitTabEntry *lit = findLiteral(&littabList, litKey);
                if (lit == NULL) {
                    lit = (LitTabEntry*)calloc(1, sizeof(LitTabEntry));
                    lit->name = strdup(litKey);
                    lit->length = byteLikeLength(litKey + 1);
                    lit->address = -1;
                    createAndAppendNode(&littabList, lit);
                }
                if (lit->length <= 0) {
                    printf("Invalid literal on line %d. Terminating.\n", lineNum);
                    exit(13);
                }
                if (lit->address == -1) {
                    lit->address = locctr;
                    locctrIncrement = lit->length;
                } else {
                    locctrIncrement = 0; // already placed
                }
            } else if (strcmp(mnemonic, "END") == 0) {
                //Append END record
                createAndAppendNode(&intermediateList, intermediateRep);

                //place all unassigned literals at current LOCCTR
                Node *cur = littabList.head;
                while (cur != NULL) {
                    LitTabEntry *lit = (LitTabEntry*)cur->data;
                    if (lit != NULL && lit->address == -1 && lit->length > 0) {
                        lit->address = locctr;
                        locctr += lit->length;
                    }
                    cur = cur->nextNode;
                }

                programLength = locctr - startAddress;
                if (((SymtabEntry*)symtabList.head->data)->csect[0] != '\0') {
                    ((SymtabEntry*)symtabList.head->data)->length = programLength;
                }
                //end pass 1

                break;
            } else {
                locctrIncrement = 0; //other recognized directives have no effect on LOCCTR in our pass 1
            }
        } else {
            //instruction size is determined by the parsed format 
            locctrIncrement = intermediateRep->format;
        }

        createAndAppendNode(&intermediateList, intermediateRep);
        locctr += locctrIncrement;
    }

    if (strcmp(((IntermediateRep*)intermediateList.tail->data)->opcode->mnemonic, "END") != 0) {
        printf("Program ended without END directive. Terminating.\n");
        exit(16);
    }

    Node* debugNode = symtabList.head;
    while (debugNode != NULL) {
        debugNode = debugNode->nextNode;
    }

    pass2();
}

void pass2() {
    Node* intermediateRepNode = intermediateList.head;
    bool canBase = false;
    char* baseLabel = NULL;
    int baseAddress = 0;

    //TODO: Replace print with print to file
    while (intermediateRepNode != NULL) {
        IntermediateRep* intermediateRep = (IntermediateRep*)intermediateRepNode->data;

        if (intermediateRep->comment != NULL && strlen(intermediateRep->comment) != 0) {
            printf("%s", intermediateRep->comment);
            intermediateRepNode = intermediateRepNode->nextNode;
            continue;
        }

        printf("%04X    ", intermediateRep->address);
        printf("%-8s", intermediateRep->label);
        if (intermediateRep->format == 4) {
            printf("+");
        }
        else {
            printf(" ");
        }
        printf("%-8s", intermediateRep->opcode->mnemonic);
        if (
            intermediateRep->operand[0] == '#' ||
            intermediateRep->operand[0] == '@' ||
            intermediateRep->operand[0] == '='
            ) {
            printf("%-26s", intermediateRep->operand);
        }
        else {
            printf(" %-25s", intermediateRep->operand);
        }

        if (intermediateRep->format != -1) {
            int instruction = intermediateRep->opcode->opcode;

            if (intermediateRep->format == 1) {

            }
            else if (strcmp(intermediateRep->opcode->mnemonic, "RSUB") == 0) {
                instruction += 0b11;
                instruction = instruction << ((intermediateRep->format - 1) * 8);
            }
            else if (intermediateRep->format == 2) {
                instruction = (instruction << 8) + format2ObjectCode(intermediateRep->operand, expectedNumRegisters(intermediateRep->opcode->mnemonic));
            }
            else {
                int ni = 0b11;
                if (intermediateRep->operand[0] == '#') {
                    ni = 0b01;
                }
                else if (intermediateRep->operand[0] == '@') {
                    ni = 0b10;
                }
                instruction += ni;
                
                if (intermediateRep->format == 3) {
                    if (intermediateRep->operand[0] == '#' ||
                        intermediateRep->operand[0] == '@'
                        ) {
                        instruction = (instruction << 16) + format3ObjectCode(&intermediateRep->operand[1], intermediateRep->address + 3, canBase, baseAddress);
                    }
                    else {
                        instruction = (instruction << 16) + format3ObjectCode(intermediateRep->operand, intermediateRep->address + 3, canBase, baseAddress);
                    }
                }
                else if (intermediateRep->format == 4) {
                    if (intermediateRep->operand[0] == '#' ||
                        intermediateRep->operand[0] == '@'
                        ) {
                        instruction = (instruction << 24) + format4ObjectCode(&intermediateRep->operand[1]);
                    }
                    else {
                        instruction = (instruction << 24) + format4ObjectCode(intermediateRep->operand);
                    }
                }

            }

            printf("%0*X", intermediateRep->format * 2, instruction);
        }
        else {
            if (strcmp(intermediateRep->opcode->mnemonic, "BASE") == 0) {
                canBase = true;
                SymtabEntry* symbol = findSymbol(&symtabList, intermediateRep->operand);
                if (symbol == NULL) {
                    printf("Could not find symbol %s. Terminating.\n", intermediateRep->operand);
                    exit(31);
                }
                baseAddress = symbol->value;
            }
        }
        printf("\n");
        intermediateRepNode = intermediateRepNode->nextNode;
    }
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

//Gets operand from a line in most cases
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

int format3ObjectCode(char* operand, int pc, bool canBase, int baseAddress) {
    int xbpe = 0b0000;
    int len = strlen(operand);
    int address = 0;

    if (isIndexed(operand)) {
        if (operand[0] == '=') {
            printf("Illegal addressing mode for operand %s. Terminating.\n", operand);
            exit(25);
        }
        xbpe += 0b1000;
    }

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
    
    char label[OPERAND_COL_LEN + 1];
    if (isIndexed(operand)) {
        strncpy(label, operand, OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2);
        label[OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2] = '\0';
    }
    else {
        strncpy(label, operand, OPERAND_COL_LEN);
        label[OPERAND_COL_LEN] = '\0';
    }
    
    SymtabEntry* symEntry = findSymbol(&symtabList, label);
    LitTabEntry* litEntry = findLiteral(&littabList, label);
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

int format4ObjectCode(char* operand) {
    int xbpe = 0b0001;
    int len = strlen(operand);
    int address = 0;

    if (isIndexed(operand)) {
        if (operand[0] == '=') {
            printf("Illegal addressing mode for operand %s. Terminating.\n", operand);
            exit(32);
        }
        xbpe += 0b1000;
    }

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

    char label[OPERAND_COL_LEN + 1];
    if (isIndexed(operand)) {
        strncpy(label, operand, OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2);
        label[OPERAND_COL_LEN < len - 2 ? OPERAND_COL_LEN : len - 2] = '\0';
    }
    else {
        strncpy(label, operand, OPERAND_COL_LEN);
        label[OPERAND_COL_LEN] = '\0';
    }

    SymtabEntry* symEntry = findSymbol(&symtabList, label);
    LitTabEntry* litEntry = findLiteral(&littabList, label);
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
    return (xbpe << 20) + address;
}

bool isIndexed(char* str) {
    return strlen(str) > 2 && str[strlen(str) - 1] == 'X' && str[strlen(str) - 2] == ',';
}

int expectedNumRegisters(char* operand) {
    if (
        strcmp(operand, "CLEAR") == 0 ||
        strcmp(operand, "SVC") == 0 ||
        strcmp(operand, "TIXR") == 0
        ) {
        return 1;
    }
    return 2;
}