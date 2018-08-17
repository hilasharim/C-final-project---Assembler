#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE_LEN 80
#define MAX_TOKEN_LEN 30
#define MAX_OPCODE_LEN 4
#define MAX_REGISTER_LEN 6
#define ARE_LEN 2
#define PARAMETER_LEN 2
#define ADDRESSING_LEN 2
#define STRING_ENCLOSER '\"'
#define COMMENT_CHAR ';'
#define IM_ADDRESSING_CHAR '#'
#define MAX_WORD_LEN 14
#define ZERO_ASCII_VAL 48
#define MAX_PARAMETERS 200
#define MAX_ASCII_LEN 3
#define NUM_ADDRESSING_TYPES 4
#define EMPTY_COMMAND "00000000000000"
#define MEMORY_START_POS 100

typedef struct label {
    char name[MAX_TOKEN_LEN+1];
    int value;
    char dataFlag;
    char codeFlag;
    char externFlag;
    char entryFlag;
    char realocFlag;
    struct label *next;
} label;

typedef struct labelList {
    label *head;
    label *last;
} labelList;

char registers[][MAX_TOKEN_LEN+1] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

char registersCodeValues[][MAX_REGISTER_LEN+1] = {"000000", "000001", "000010", "000011",
                                                  "000100", "000101", "000110", "000111"};

char instructions[][MAX_TOKEN_LEN+1] = {".data", ".string", ".entry", ".extern"};

char operations[][MAX_TOKEN_LEN+1] = {"mov", "cmp", "add", "sub", "not", "clr", "lea",
                                            "inc", "dec", "jmp", "bne", "red", "prn", "jsr",
                                            "rts", "stop"};

char opcodes[][MAX_OPCODE_LEN+1] = {"0000", "0001", "0010", "0011", "0100", "0101", 
                                    "0110", "0111", "1000", "1001", "1010", "1011",
                                    "1100", "1101", "1110", "1111"};

int numParameters[] = {2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0};

int allowedSrcAddressingTypes[][NUM_ADDRESSING_TYPES] = {{0,1,3},{0,1,3},{0,1,3},{0,1,3},{},
                                                          {},{1}};
int allowedSrcAddressingTypesNum[] = {3,3,3,3,0,0,1};

int allowedDestAddressingTypes[][NUM_ADDRESSING_TYPES] = {{1,3},{0,1,3},{1,3},{1,3},{1,3},
                                                          {1,3},{1,3},{1,3},{1,3},{1,2,3},
                                                          {1,2,3},{1,3},{0,1,3},{1,2,3}};
int allowedDestAddressingTypesNum[] = {2,3,2,2,2,2,2,2,2,3,3,2,3,3};

const int totalRegisters = 8;
const int totalInstructions = 4;
const int totalOperations = 16;

/*function to create new empty list*/
labelList* createList() {
    labelList *newList = (labelList*)malloc(sizeof(labelList));
    if (!newList) {
        fprintf(stderr, "Unable to allocate memory. Terminating.\n");
        exit(1);
    }
    else {
        newList -> head = NULL;
        newList -> last = NULL;
        return newList;
    }
}

void addLabel(labelList *list, char* newName, int newVal, char newDataFlag, char newCodeFlag, char newExternFlag, char newEntryFlag, char newRealocFlag) {
    label *newLabel = (label*)malloc(sizeof(label));
    if (!newLabel) {
        fprintf(stderr, "Unable to allocate memory. Terminating.\n");
        exit(1);
    }
    else {
        strcpy(newLabel -> name, newName);
        newLabel -> value = newVal;
        newLabel -> dataFlag = newDataFlag;
        newLabel -> codeFlag = newCodeFlag;
        newLabel -> externFlag = newExternFlag;
        newLabel -> entryFlag = newEntryFlag;
        newLabel -> realocFlag = newRealocFlag;
        newLabel -> next = NULL;
        if (list -> head == NULL) { /*list is empty*/
            list -> head = newLabel;
            list -> last = newLabel;
        }
        else { /*list is not empty*/
            list -> last -> next = newLabel;
            list -> last = newLabel;
        }
    }
}

/*function to free list memory. frees all list members, and list struct itself*/
void freeList (labelList *list) {
    label *temp, *current;
    int count = 0;
    current = list -> head;
    while (current != NULL) {
        temp = current;
        current = current -> next;
        free(temp);
        count++;
    }
    free(list);
}

/*function to print list for debugging*/
void printList (labelList *list) {
    label *current = list -> head;
    while (current != NULL) {
        printf("%s %d\n", current -> name, current -> value);
        current = current -> next;
    }
}

/*function to search list for a label with a specified name. returns 1 if found, 0 otherwise*/
int containsName (labelList *list, char *searchTerm) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, searchTerm) == 0) {
            found = 1;
        }
        current = current -> next;
    }
    return found;
}

/*function to add constant value to all labels marked as 'data'. goes over list element-by-element,
if dataFlag is 1, adds the given constant value to the label's value*/
void incrementDataLabels (labelList *list, int incrementValue) {
    label *current = list -> head;
    while (current != NULL) {
        if (current -> dataFlag == 1) {
            current -> value += incrementValue;
        }
        current = current -> next;
    }
}

/*return 1 if found and updated, 0 if not found, -1 if found but declared as extern*/
int setEntryFlag (labelList *list, char labelName[MAX_TOKEN_LEN+1]) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, labelName) == 0) {
            found = 1;
            if (current -> externFlag == 1) {
                return -1;
            }
            current -> entryFlag = 1;
        }
        current = current -> next;
    }
    return found;
}

/*return 0 if label not found in list and 1 otherwise. copy relevant values to target variables*/
int getLabelValueARE(labelList *list, char labelName[MAX_TOKEN_LEN+1], int *valueTarget, int *RFlagTarget, int *EFlagTarget) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, labelName) == 0) {
            found = 1;
            *valueTarget = current -> value;
            *RFlagTarget = current -> realocFlag;
            *EFlagTarget = current -> externFlag;
        }
        current = current -> next;
    }
    return found;
}

/*function to check if the given number of bytes in the string are only whitespace characters. goes over string character by character.*/
int isStringEmpty (char *str, int bytes) {
    while (bytes--) {
        if (!isspace(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/*function that returns the position of the first non-whitespace character in a string. assumes string is not only whitespace
(does not check string boundaries)*/
int getFirstNonWhiteSpaceCharacter(char *str) {
    int currPos = 0;
    while (isspace(str[currPos]))
        currPos++;
    return currPos;
}

/*function that returns the position of the last non-whitespace character in a string. assumes string is not only whitespace
(does not check string boundaries)*/
int getLastNonWhiteSpaceCharacter(char *str, int strLen) {
    int currPos = strLen - 1;
    while (isspace(str[currPos]))
        currPos--;
    return currPos;
}

/*function that accepts a string, the string's length and a destination string, and copies the original string into the destination without surrounding
whitespace characters*/
void trimString (char *src, char *dest, int srcLen) {
    if (isStringEmpty(src, srcLen)) {
        *dest = '\0';
    }
    else {
        int firstChar = getFirstNonWhiteSpaceCharacter(src);
        int lastChar = getLastNonWhiteSpaceCharacter(src, srcLen);
        strncpy(dest, src+firstChar, lastChar-firstChar+1);
        dest[lastChar-firstChar+1] = '\0';
    }
}

char* getNextDelimiterPos(char *str, char delimiter) {
    char *delimiterPos = strchr(str, delimiter);
    if (!delimiterPos) {
        delimiterPos = str + strlen(str);
    }
    return delimiterPos;
}

int splitString(char *str, char targetArray[][MAX_TOKEN_LEN+1], char delimiter) {
    int currTargetArrayPos, totalParsed;
    currTargetArrayPos = totalParsed = 0;
    char *prevDelimPos, *currDelimPos;
    prevDelimPos = str-1;
    currDelimPos = str;
    while (*currDelimPos != '\0') {
        currDelimPos = getNextDelimiterPos(str, delimiter);
        strncpy(targetArray[currTargetArrayPos], prevDelimPos+1, currDelimPos-(prevDelimPos+1));
        targetArray[currTargetArrayPos][currDelimPos-(prevDelimPos+1)] = '\0';
        prevDelimPos = currDelimPos;
        str = currDelimPos + 1;
        totalParsed++;
        currTargetArrayPos++;
    }
    if (*currDelimPos != '\0') { /*delimiter at end of string*/
        *targetArray[currTargetArrayPos] = '\0';
        totalParsed++;
    } 
    return totalParsed;
}

/*search for string in given string array. return first position in array, or -1 if not found */
int indexOf (char stringArray[][MAX_TOKEN_LEN+1], int arrayLen, char searchString[]) {
    int foundIndex = -1;
    int currPosition = 0;
    while (foundIndex < 0 && currPosition < arrayLen) {
        if (strcmp(searchString, stringArray[currPosition]) == 0) {
            foundIndex = currPosition;
        }
        currPosition++;
    }
    return foundIndex;
}

int indexOfInt (int intArray[], int arrayLen, int searchTerm) {
    int foundIndex = -1;
    int currPosition = 0;
    while (foundIndex < 0 && currPosition < arrayLen) {
        if (intArray[currPosition] == searchTerm) {
            foundIndex = currPosition;
        }
        currPosition++;
    }
    return foundIndex;
}

/*function that checks if label name is legal. return -1 if reserved word, 0 if not but 
not legal, and 1 if legal*/
int isLabelLegal (char *label) {
    int pos;
    if (indexOf(registers, totalRegisters, label) >= 0 || indexOf(instructions, totalInstructions, label) >= 0 || indexOf(operations, totalOperations, label) >= 0) {
        return -1;
    }
    if (!isalpha(label[0])) {
        return 0;
    }
    for (pos = 1; pos < strlen(label); pos++) {
        if (!isalnum(label[pos])) {
            return 0;
        }
    }
    return 1;
}

/*function to check if a string can be translated into a legal integer - contains a
plus or minus sign at the beginning and only numeric values after that. return 0 if not legal,
otherwise return 1*/
int isInteger(char *str) {
    if (*str == '-' || *str == '+') {
        str++;
    }
    while (*str != '\0') {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/*function to check if a string contains any whitespace characters*/
int hasWhitespace (char *str) {
    while (*str != '\0') {
        if (isspace(*str)) {
            return 1;
        }
        str++;
    }
    return 0;
}

/*function to check if a given string is legal for a .string instruction, by checking if it is enclosed
by the specified character and contains only printable characters. returns 0 if not legal, otherwise 1.*/
int isStringLegal (char *str) {
    if (str[0] != STRING_ENCLOSER || str[strlen(str)-1] != STRING_ENCLOSER) {
        return 0;
    }
    
    while (*str != '\0') {
        if (!isprint(*str)) {
            return 0;
        }
        str++;
    }
    
    return 1;
}

/*function to convert a string representing positive integer in base 10 to a string of its representation
in base two. if the representation in base two is longer than the max word length, the most significant
bits are truncated. works by dividing the given number by two, and inserting the remainder as the
current bit, right to left. will continue until number is zero or out of space*/
void baseTenToBaseTwo (char *baseTenNumber, char *baseTwoNumber) {
    int number, remaind, currPos;
    currPos = MAX_WORD_LEN - 1;
    sscanf(baseTenNumber, "%d", &number);
    do {
        remaind = number % 2;
        number /= 2;
        baseTwoNumber[currPos] = ZERO_ASCII_VAL + remaind;
        currPos--;
    } while (number > 0 && currPos >=0);
    
    if (currPos != -1) {
        while (currPos >= 0) {
            baseTwoNumber[currPos] = ZERO_ASCII_VAL;
            currPos--;
        }
    }
    baseTwoNumber[MAX_WORD_LEN] = '\0';
}

/*function to convert a base two representation of a positive integer to its two's complement
representation of the negative number. goes over all bits right to left, until a '1' is encountered.
leaves the first '1' unchanged, and flips all other bits to the left. if the two's complement can't
be represented with given number of bits, information is lost.*/
void twosComplement (char *baseTwoPositiveNumber, char *twosComplementRepresentation) {
    int currPos = MAX_WORD_LEN - 1;
    while (baseTwoPositiveNumber[currPos] == '0' && currPos >= 0) {
        twosComplementRepresentation[currPos] = baseTwoPositiveNumber[currPos];
        currPos--;
    }
    if (currPos >= 0) { //ignore first '1' if present
        twosComplementRepresentation[currPos] = baseTwoPositiveNumber[currPos];
        currPos--;
    }
    while (currPos >= 0) {
        if (baseTwoPositiveNumber[currPos] == '0') {
            twosComplementRepresentation[currPos] = '1';
        }
        else {
            twosComplementRepresentation[currPos] = '0';
        }
        currPos--;
    }
}

/*function to parse an integer to its base two representation. works by converting the positive
part to base two. if the original number was negative, find the two's complement of the positive number.
function assumes that the number is a legal integer.*/
void parseInteger (char *stringNumber, char baseTwoRep[MAX_WORD_LEN]) {
    char tempNum[MAX_WORD_LEN+1];
    if (stringNumber[0] == '+') {
        baseTenToBaseTwo(stringNumber+1, baseTwoRep);
    }
    else if (stringNumber[0] == '-') {
        baseTenToBaseTwo(stringNumber+1, tempNum);
        twosComplement(tempNum, baseTwoRep);
    }
    else {
        baseTenToBaseTwo(stringNumber, baseTwoRep);
    }
}

int parseDataArray(char *parametersString, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos) {
    char stringSplitRes[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char stringSplitTrimRes[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int numSplitParams, currPos, nullPos;
    
    numSplitParams = splitString(parametersString, stringSplitRes, ','); //split the string into tokens
    //trim all tokens
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        trimString(stringSplitRes[currPos], stringSplitTrimRes[currPos], strlen(stringSplitRes[currPos]));
    }
    //check if any token in null
    if ((nullPos = indexOf(stringSplitTrimRes, numSplitParams, "")) >= 0) {
        return -1;
    }
    //check that all tokens are legal integers
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        if (!isInteger(stringSplitTrimRes[currPos])) {
            return -2;
        }
    }
    //convert all integers to base 2 and store them in the target array, starting at targetStartPos
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        parseInteger(stringSplitTrimRes[currPos], dataTargetArray[targetStartPos+currPos]);
    }
    return numSplitParams;
}

int parseStringInstruction (char *string, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos) {
    int currPos;
    char currAsciiVal[MAX_ASCII_LEN+1];
    if (!isStringLegal(string)) {
        return 0;
    }
    //parse ASCII values of string characters while skipping "". save base 2 values in dataTargetArray
    for (currPos = 1; currPos < strlen(string)-1; currPos++) {
        sprintf(currAsciiVal, "%d", string[currPos]);
        parseInteger(currAsciiVal, dataTargetArray[targetStartPos+currPos-1]);
    }
    parseInteger("0", dataTargetArray[targetStartPos+currPos-1]);
    return strlen(string)-1;
}

/*function to check if a label exists at the beginning of a string. returns 1 if legal label
was found, returns -2 if no label was found. returns -1 if
the label is a reserved word, returns 0 if label not legal for another reason.
copies trimmed label to the target array.*/
int getLabel(char *string, char labelTarget[MAX_TOKEN_LEN+1], char restOfString[MAX_TOKEN_LEN+1]) {
    char *delimPos;
    char labelNotTrimmed[MAX_TOKEN_LEN+1];
    int legalLabel;
    delimPos = getNextDelimiterPos(string, ':');
    if (*delimPos == '\0') { /*':' was not found, there is no label and no split needed*/
        strcpy(restOfString, string);
        return -2;
    }
    else {
        strncpy(labelNotTrimmed, string, delimPos - string);
        labelNotTrimmed[delimPos - string] = '\0';
        strncpy(restOfString, delimPos+1, strlen(string) - (delimPos - string +1));
        restOfString[strlen(string) - (delimPos - string +1)] = '\0';
        trimString(labelNotTrimmed, labelTarget, strlen(labelNotTrimmed));
        legalLabel = isLabelLegal(labelTarget);
        return legalLabel;
    }
}

/*return two operands comprising a two operand operator, return -1 if comma in wrong position,
-2 if more than 2 operands, 1 otherwise*/
int getTwoOperands(char *string, char operandTargetArray[][MAX_TOKEN_LEN+1]) {
    int numSplitParams, currPos, nullPos;
    char internalParams[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    numSplitParams = splitString(string, internalParams, ',');
    //trim all tokens
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        trimString(internalParams[currPos], operandTargetArray[currPos], strlen(internalParams[currPos]));
    }
    //check if any token in null
    if ((nullPos = indexOf(operandTargetArray, numSplitParams, "")) >= 0) {
        return -1;
    }
    //check if there are more than 2 operands or less than 2
    if (numSplitParams != 2) {
        return -2;
    }
    return 1;
}

/*copy the three operands comprising addressing method 2 to operandValuesArray. return 0 if
structure does not correspond to addressing method 2, and 1 otherwise*/
int parseAddressingMethod2(char *operand, char operandValuesArray[][MAX_TOKEN_LEN+1]) {
    char *delimPos;
    if (operand[strlen(operand)-1] != ')') {
        return 0;
    }
    delimPos = getNextDelimiterPos(operand, '(');
    if (*delimPos == '\0') {
        return 0;
    }
    else {
        strncpy(operandValuesArray[0], operand, delimPos - operand);
        operandValuesArray[0][delimPos - operand] = '\0';
        operand[strlen(operand)-1] = '\0';
        if(getTwoOperands(delimPos+1, operandValuesArray+1) != 1) {
            return 0;
        }
    }
    return 1;
} 

/*function to parse a single instruction operand. returns -1 if operand is not legal, -2 if label not legal, 
addressing type if the operand is legal. copies the value of the operand to the operandValuesArray,
max 3 parameters (for addressing 2)*/
int parseSingleOperand(char *operand, char operandValuesArray[][MAX_TOKEN_LEN+1], int operandAddressingMethod[]) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    int legalLabel, operandOneAddressingMethod, operandTwoAddressingMethod;
    trimString(operand, trimmedOperand, strlen(operand));
    if (hasWhitespace(trimmedOperand)) {
        return -1;
    }
    if (indexOf(registers, totalRegisters, trimmedOperand) >= 0) {
        strcpy(operandValuesArray[0], trimmedOperand);
        return 3;
    }
    if (isLabelLegal(trimmedOperand) == 1) {
        strcpy(operandValuesArray[0], trimmedOperand);
        return 1;
    }
    if (trimmedOperand[0] == IM_ADDRESSING_CHAR && isInteger(trimmedOperand+1)) {
        strcpy(operandValuesArray[0], trimmedOperand+1);
        return 0;
    }
    if (parseAddressingMethod2(trimmedOperand, operandValuesArray)) {
        if (isLabelLegal(operandValuesArray[0]) == 1) {
            operandOneAddressingMethod = parseSingleOperand(operandValuesArray[1], operandValuesArray+1, operandAddressingMethod);
            operandTwoAddressingMethod = parseSingleOperand(operandValuesArray[2], operandValuesArray+2, operandAddressingMethod);
            if (operandOneAddressingMethod >= 0 && operandTwoAddressingMethod >= 0) {
                operandAddressingMethod[0] = operandOneAddressingMethod;
                operandAddressingMethod[1] = operandTwoAddressingMethod;
                return 2;
            } 
        }
        else {
            return -2;
        }
    }
    else {
        return -1;
    }
}

void buildFirstCommandWord(int param1Addressing, int param2Addressing, int command, int srcAddressing, int destAddressing, int ARE, char commandTarget[]) {
    const char numTranslations[][3] = {"00", "01", "10", "11"};
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, numTranslations[param1Addressing], PARAMETER_LEN);
    strncpy(commandTarget+PARAMETER_LEN, numTranslations[param2Addressing], PARAMETER_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN, opcodes[command], MAX_OPCODE_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN, numTranslations[srcAddressing], ADDRESSING_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN+ADDRESSING_LEN, numTranslations[destAddressing], ADDRESSING_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN+2*ADDRESSING_LEN, numTranslations[ARE], ARE_LEN);
}

void buildRegistersWord(char *srcRegister, char *destRegister, char commandTarget[]) {
    int srcRegisterNum, destRegisterNum;
    srcRegisterNum = indexOf(registers, totalRegisters, srcRegister);
    destRegisterNum = indexOf(registers, totalRegisters, destRegister);
    
    strcpy(commandTarget, EMPTY_COMMAND);
    /*bits 8-13*/
    if (srcRegisterNum >= 0) {
        strncpy(commandTarget,registersCodeValues[srcRegisterNum],MAX_REGISTER_LEN);
    }
    /*bits 2-7*/
    if (destRegisterNum >= 0) {
        strncpy(commandTarget+6, registersCodeValues[destRegisterNum],MAX_REGISTER_LEN);
    }
}

void buildIntegerWord(char *intergerString, char commandTarget[]) {
    char tempNumOperand[MAX_WORD_LEN+1];
    parseInteger(intergerString, tempNumOperand);
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, tempNumOperand+ARE_LEN, MAX_WORD_LEN-ARE_LEN);
}

/*Parse operand of command that should have no operators. if the operator is null, copies the commad code into the position
indicated by IC and increments IC by 1*/
int parseOperatorWithNoOperands(char *operand, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    char commandCode[MAX_WORD_LEN+1];
    trimString(operand, trimmedOperand, strlen(operand));
    if (strcmp(trimmedOperand, "\0") != 0) {
        return 0;
    }
    else {
        buildFirstCommandWord(0, 0, command, 0, 0, 0, commandsArray[*IC]);
        (*IC)++;
        return 1;
    }
}

void buildWordOperatorAddressing013(char *operand, int operandAddressing, char commandTarget[], int destFlag) {
    switch(operandAddressing) {
        case 0:
            buildIntegerWord(operand, commandTarget);
            break;
        case 1:
            break;
        case 3:
            if (destFlag) {
                buildRegistersWord("", operand, commandTarget);
            }
            else {
                buildRegistersWord(operand, "", commandTarget);
            }
            break;
        default:
            break;
    }
}

void buildThreeCommandWords(int param1Addressing, int param2Addressing, int command, int srcOperandAddressing, char *srcOperand, int destOperandAddressing, char *destOperand, char commandsArray[][MAX_WORD_LEN+1], int *IC, int addressing2Flag) {
    buildFirstCommandWord(param1Addressing, param2Addressing, command, srcOperandAddressing, destOperandAddressing,0,commandsArray[*IC]);
    (*IC)++;
    if (addressing2Flag) {
        srcOperandAddressing = param1Addressing;
        destOperandAddressing = param2Addressing;
        (*IC)++;
    }
    if (srcOperandAddressing == 3 && destOperandAddressing == 3) {
        buildRegistersWord(srcOperand, destOperand, commandsArray[*IC]);
        (*IC)++;
        return;
    }
    buildWordOperatorAddressing013(srcOperand, srcOperandAddressing, commandsArray[*IC], 0);
    (*IC)++;
    buildWordOperatorAddressing013(destOperand, destOperandAddressing, commandsArray[*IC], 1);
    (*IC)++;
}

/*return -1 if wrong addressing method, 0 if wrong for internal parameters, 1 if legal*/
int parseOneOperandCommand(char *operand, int command, int operandAllowedAddressingVals[], int numAllowedAddressingVals, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int addressingMethod, addressingMethodIndex;
    char operandValues[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressing[MAX_PARAMETERS]; 
    addressingMethod = parseSingleOperand(operand, operandValues, operandAddressing);
    addressingMethodIndex = indexOfInt(operandAllowedAddressingVals, numAllowedAddressingVals, addressingMethod);
    if (addressingMethodIndex < 0) {
        return addressingMethod;
    }
    if (addressingMethod == 0 || addressingMethod == 1 || addressingMethod == 3) {
        buildFirstCommandWord(0, 0, command, 0, addressingMethod, 0, commandsArray[*IC]);
        (*IC)++;
        buildWordOperatorAddressing013(operandValues[0], addressingMethod, commandsArray[*IC], 1);
        (*IC)++;
    }
    else if (addressingMethod == 2) {
        if (operandAddressing[0] < 0 || operandAddressing[0] == 2 || operandAddressing[1] < 0 || operandAddressing[1] == 2) {
            return 0;
        }
        buildThreeCommandWords(operandAddressing[0], operandAddressing[1], command, 0, operandValues[1], addressingMethod, operandValues[2], commandsArray, IC, 1);
    }
    return 1;
}

/*return -1 if comma in wrong position, -2 if more than 2 operands, 0 if wrong addressing method, 1 if legal*/
int parseOperatorWithTwoOperands(char *string, int command, int srcOperandAllowedAddressingVals[], int numSrcOperandAllowedAddressingVals, int destOperandAllowedAddressingVals[], int numDestOperandAllowedAddressingVals, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int srcOperandAddressing, destOperandAddressing, exactlyTwoOperands;
    int srcOperandAddressingIndex, destOperandAddressingIndex;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    exactlyTwoOperands = getTwoOperands(string, splitOperands);
    if (exactlyTwoOperands != 1) {
        return exactlyTwoOperands;
    }
    srcOperandAddressing = parseSingleOperand(splitOperands[0], operandValues, operandAddressingArray);
    destOperandAddressing = parseSingleOperand(splitOperands[1], operandValues+1, operandAddressingArray);
    srcOperandAddressingIndex = indexOfInt(srcOperandAllowedAddressingVals, numSrcOperandAllowedAddressingVals, srcOperandAddressing);
    destOperandAddressingIndex = indexOfInt(destOperandAllowedAddressingVals, numDestOperandAllowedAddressingVals, destOperandAddressing);
    if (srcOperandAddressingIndex == -1 || destOperandAddressingIndex == -1) {
        return 0;
    }
    buildThreeCommandWords(0, 0, command, srcOperandAddressing, operandValues[0], destOperandAddressing, operandValues[1], commandsArray, IC, 0);
    return 1;
}

/*return -1 if no instruction was found or not in array, otherwise return the index of the found instruction in instructionArray*/
int getInstruction(char *string, char instructionsArray[][MAX_TOKEN_LEN+1], int instructionsArrayLen, char operand[MAX_TOKEN_LEN+1]) {
    char *delimPos;
    char instructionNotTrimmed[MAX_TOKEN_LEN+1];
    char instructionTrimmed[MAX_TOKEN_LEN+1];
    int instructionIndex;
    delimPos = getNextDelimiterPos(string, ' ');
    
    if (*delimPos == '\0') { /*' ' was not found, check if instruction with no operands*/
        strcpy(instructionNotTrimmed, string);
        operand[0] = '\0';
    }
    else {
        strncpy(instructionNotTrimmed, string, delimPos - string);
        instructionNotTrimmed[delimPos - string] = '\0';
        strncpy(operand, delimPos+1, strlen(string) - (delimPos - string +1));
        operand[strlen(string) - (delimPos - string +1)] = '\0';
    }
    trimString(instructionNotTrimmed, instructionTrimmed, strlen(instructionNotTrimmed));
    return indexOf(instructionsArray, instructionsArrayLen, instructionTrimmed);
}

//return -1 if null found on split, -2 if not integer, 0 if label already in list
int dataStringInstruction(int lineNumber, int instruction, char *operand, char dataArray[][MAX_WORD_LEN+1], char label[MAX_TOKEN_LEN+1], int existsLabelFlag, labelList *allLabelsList, int *DC) {
    int instructionParseResult;
    if (existsLabelFlag) {
        if (containsName(allLabelsList, label)) {
            fprintf(stderr, "Error in line %d: label has already been declared\n", lineNumber);
            return 0;
        }
        else {
            addLabel(allLabelsList, label, (*DC)+MEMORY_START_POS, 1, 0, 0, 0, 1);
        }
    }
    if (instruction == 0) { /*data*/
        instructionParseResult = parseDataArray(operand, dataArray, *DC);
        if (instructionParseResult == -1) {
            fprintf(stderr, "Error in line %d: comma in unexpected position\n", lineNumber);
            return 0;
        }
        if (instructionParseResult == -2) {
            fprintf(stderr, "Error in line %d: all parameters in .data instruction must be integers\n", lineNumber);
            return 0;
        }
        (*DC) += instructionParseResult;
    }
    else { /*string*/
        instructionParseResult = parseStringInstruction (operand, dataArray, *DC);
        if (!instructionParseResult) {
            fprintf(stderr, "Error in line %d: Illegal string parameter\n", lineNumber);
            return 0;
        }
        (*DC) += instructionParseResult;
    }
    return 1;
}

int externInstruction(int lineNumber, char *operand, labelList *allLabelsList) {
    if (!isLabelLegal(operand)) {
        fprintf(stderr, "Error in line %d: Illegal label name in .extern instruction\n", lineNumber);
        return 0;
    }
    if (containsName(allLabelsList, operand)) {
        fprintf(stderr, "Error in line %d: label has already been declared\n", lineNumber);
        return 0;
    }
    addLabel(allLabelsList, operand, 0, 0, 0, 1, 0, 0);
    return 1;
}

int entryInstruction(int lineNumber, char *operand) {
    if (!isLabelLegal(operand)) {
        fprintf(stderr, "Error in line %d: Illegal label name in .entry instruction\n", lineNumber);
        return 0;
    }
    return 1;
}

int parseInstructions(int lineNumber, char *operand, int *DC, int instructionIndex, char dataArray[][MAX_WORD_LEN+1], char label[MAX_TOKEN_LEN+1], int existsLabelFlag, labelList *allLabelsList) {
    int instructionParseResult = 0;
    char trimmedOperand[MAX_TOKEN_LEN+1];
    trimString(operand, trimmedOperand, strlen(operand));
    /*.data or .string*/
    if (instructionIndex == 0 || instructionIndex == 1) {
        instructionParseResult = dataStringInstruction(lineNumber, instructionIndex, trimmedOperand, dataArray, label, existsLabelFlag, allLabelsList, DC);
    }
    /*.entry*/
    if (instructionIndex == 2) {
        instructionParseResult = entryInstruction(lineNumber,trimmedOperand);
    }
    /*.extern*/
    if (instructionIndex == 3) {
        instructionParseResult = externInstruction(lineNumber,trimmedOperand, allLabelsList);
    }
    return instructionParseResult;
}

int parseOperator(int lineNumber, char *operand, int *IC, int operatorIndex, char commandsArray[][MAX_WORD_LEN+1], int existsLabelFlag, char label[MAX_TOKEN_LEN+1], labelList *allLabelsList) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    trimString(operand, trimmedOperand, strlen(operand));
    int operatorParseResult = 0;
    if (existsLabelFlag) {
        if (containsName(allLabelsList, label)) {
            fprintf(stderr, "Error in line %d: label has already been declared\n", lineNumber);
            return 0;
        }
        else {
            addLabel(allLabelsList, label, (*IC)+MEMORY_START_POS, 0, 1, 0, 0, 1);
        }
    }
    /*two operand commands*/
    if (numParameters[operatorIndex] == 2) {
        operatorParseResult = parseOperatorWithTwoOperands(operand, operatorIndex, allowedSrcAddressingTypes[operatorIndex], allowedSrcAddressingTypesNum[operatorIndex], allowedDestAddressingTypes[operatorIndex], allowedDestAddressingTypesNum[operatorIndex], commandsArray, IC);
        if (operatorParseResult == -2) {
            fprintf(stderr, "Error in line %d: Illegal number of operands. Expected two\n", lineNumber);
            return 0;
        }
        if (operatorParseResult == -1) {
            fprintf(stderr, "Error in line %d: Comma in unexpected position\n", lineNumber);
            return 0;
        }
        if (operatorParseResult == 0) {
            fprintf(stderr, "Error in line %d: Illegal addressing method for operator\n", lineNumber);
            return 0;
        }
    }
    /*one operand commands*/
    else if (numParameters[operatorIndex] == 1) {
        operatorParseResult = parseOneOperandCommand(operand, operatorIndex, allowedDestAddressingTypes[operatorIndex], allowedDestAddressingTypesNum[operatorIndex], commandsArray, IC);
        if (operatorParseResult == -1) {
            fprintf(stderr, "Error in line %d: Illegal addressing method for operator\n", lineNumber);
            return 0;
        }
        if (operatorParseResult == 0) {
            fprintf(stderr, "Error in line %d: Illegal addressing method for internal parameters in operand\n", lineNumber);
            return 0;
        }
    }
    /*no operand commands*/
    else {
        operatorParseResult = parseOperatorWithNoOperands(operand, operatorIndex, commandsArray, IC);
        if (!operatorParseResult) {
            fprintf(stderr, "Error in line %d: operator expects 0 operands\n", lineNumber);
            return 0;
        }
    }
    return 1;
}

int parseLine(char *line, int lineNumber, int *DC, int *IC, char dataArray[][MAX_WORD_LEN+1], char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList) {
    char label[MAX_TOKEN_LEN+1], restOfString[MAX_TOKEN_LEN+1], trimmedRestOfString[MAX_TOKEN_LEN+1], operand[MAX_TOKEN_LEN+1];
    int existsLabelFlag, getlabelResult, instruction, parseInstructionResult;
    existsLabelFlag = getlabelResult = instruction = 0;
    
    getlabelResult = getLabel(line, label, restOfString);
    if (getlabelResult == -1) {
        fprintf(stderr, "Error in line %d: reserved word used as label\n", lineNumber);
        return 0;
    }
    if (getlabelResult == 0) {
        fprintf(stderr, "Error in line %d: Illegal label name\n", lineNumber);
        return 0;
    }
    if (getlabelResult == 1) {
        existsLabelFlag = 1;
    }
    trimString(restOfString, trimmedRestOfString, strlen(restOfString));
    /*check if .data .string .entry .extern*/
    instruction = getInstruction(trimmedRestOfString, instructions, totalInstructions, operand);
    if (instruction >= 0) {
        parseInstructionResult = parseInstructions(lineNumber, operand, DC, instruction, dataArray, label, existsLabelFlag, allLabelsList);
    }
    else {
        /*check if recognized operation (mov, bne etc.)*/
        instruction = getInstruction(trimmedRestOfString, operations, totalOperations, operand);
        if (instruction >= 0) {
            parseInstructionResult = parseOperator(lineNumber, operand, IC, instruction, commandsArray, existsLabelFlag, label, allLabelsList);
        }
        else {
            fprintf(stderr, "Error in line %d: No legal instruction or operator found\n", lineNumber);
            return 0;
        }
    }
    return parseInstructionResult;
}

//from here second pass
void buildLabelAddressWord(int labelAddress, int RFlag, int EFlag, char commandTarget[]) {
    const char ARE[][ARE_LEN] = {"00", "10", "01"};
    char addressAsString[MAX_WORD_LEN+1];
    char tempNumOperand[MAX_WORD_LEN+1];
    sprintf(addressAsString, "%d", labelAddress);
    parseInteger(addressAsString, tempNumOperand);
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, tempNumOperand+ARE_LEN, MAX_WORD_LEN-ARE_LEN);
    if (RFlag) {
        strncpy(commandTarget + MAX_WORD_LEN - ARE_LEN, ARE[1], ARE_LEN);
    }
    else if(EFlag) {
        strncpy(commandTarget + MAX_WORD_LEN - ARE_LEN, ARE[2], ARE_LEN);
    }
}

int parseLabelOperandSecondPass(char labelName[], char commandTarget[], labelList *allLabelsList, labelList *externList, int commandPosition) {
    int labelFoundFlag, labelValue, labelRFlag, labelEFlag;
    labelFoundFlag = getLabelValueARE(allLabelsList, labelName, &labelValue, &labelRFlag, &labelEFlag);
    if (!labelFoundFlag) {
        return 0;
    }
    buildLabelAddressWord(labelValue, labelRFlag, labelEFlag, commandTarget);
    if (labelEFlag) {
        addLabel(externList, labelName, commandPosition+MEMORY_START_POS, 0, 0, 0, 0, 0);
    }
    return 1;
}

/*return 0 if label not found, 1 if successful*/
int parseOneOperandCommandSecondPass(char *operand, char commandsArray[][MAX_WORD_LEN+1], int *IC, labelList *allLabelsList, labelList *externList) {
    int addressingMethod, currOperand;
    char operandValues[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressing[MAX_PARAMETERS];
    addressingMethod = parseSingleOperand(operand, operandValues, operandAddressing);
    (*IC)++; //skip first word
    if (addressingMethod == 1 || addressingMethod == 2) {
        if (!parseLabelOperandSecondPass(operandValues[0], commandsArray[*IC], allLabelsList, externList, *IC)) {
            return 0;
        }
    }
    (*IC)++;
    
    if (addressingMethod == 2 && operandAddressing[0] == 3 && operandAddressing[1] == 3) {
        (*IC)++;
    }
    else if (addressingMethod == 2) {
        for (currOperand = 0; currOperand < 2; currOperand++) {
            if (operandAddressing[currOperand] == 1) {
                if (!parseLabelOperandSecondPass(operandValues[currOperand+1], commandsArray[*IC], allLabelsList, externList, *IC)) {
                    return 0;
                }
            }
            (*IC)++;
        }
    }
    return 1;
}

int parseOperatorWithTwoOperandsSecondPass(char *string, char commandsArray[][MAX_WORD_LEN+1], int *IC, labelList *allLabelsList, labelList *externList) {
    int srcOperandAddressing, destOperandAddressing, exactlyTwoOperands;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    exactlyTwoOperands = getTwoOperands(string, splitOperands);
    srcOperandAddressing = parseSingleOperand(splitOperands[0], operandValues, operandAddressingArray);
    destOperandAddressing = parseSingleOperand(splitOperands[1], operandValues+1, operandAddressingArray);
    (*IC)++; /*skip first word*/
    if (srcOperandAddressing == 3 && destOperandAddressing == 3) {
        (*IC)++;
        return 1;
    }
    if (srcOperandAddressing == 1) {
        if (!parseLabelOperandSecondPass(splitOperands[0], commandsArray[*IC], allLabelsList, externList, *IC)) {
            return 0;
        }
    }
    (*IC)++;
    if (destOperandAddressing == 1) {
        if (!parseLabelOperandSecondPass(splitOperands[1], commandsArray[*IC], allLabelsList, externList, *IC)) {
            return 0;
        }
    }
    (*IC)++;
    return 1;
}

int parseOperatorSecondPass(int lineNumber, char *operand, int *IC, int operatorIndex, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    trimString(operand, trimmedOperand, strlen(operand));
    int operatorParseResult = 0;
    /*two operand commands*/
    if (numParameters[operatorIndex] == 2) {
        operatorParseResult = parseOperatorWithTwoOperandsSecondPass(trimmedOperand, commandsArray, IC, allLabelsList, externList);
    }
    /*one operand commands*/
    else if (numParameters[operatorIndex] == 1) {
        operatorParseResult = parseOneOperandCommandSecondPass(trimmedOperand, commandsArray, IC, allLabelsList, externList);
    }
    /*no operand commands - nothing to do, just increment IC*/
    else {
        operatorParseResult = 1;
        (*IC)++;
    }
    if (!operatorParseResult) {
        fprintf(stderr, "Error in line %d: Usage of undeclared label\n", lineNumber);
        return 0;
    }
    return 1;
}

int parseEntrySecondPass(int lineNumber, char labelname[MAX_TOKEN_LEN+1], labelList *allLabelsList) {
    int parseResult = setEntryFlag(allLabelsList, labelname);
    if (parseResult == 0 ) {
        fprintf(stderr, "Error in line %d: .entry label not found in symbol table\n", lineNumber);
        return 0;
    }
    if (parseResult == -1) {
        fprintf(stderr, "Error in line %d: .entry label already declared as external\n", lineNumber);
        return 0;
    }
    return 1;
}

int parseLineSecondPass(char *line, int lineNumber, int *IC, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList) {
    char label[MAX_TOKEN_LEN+1], restOfString[MAX_TOKEN_LEN+1], trimmedRestOfString[MAX_TOKEN_LEN+1], operand[MAX_TOKEN_LEN+1];
    int instruction, parseInstructionResult, getlabelResult;
    
    getlabelResult = getLabel(line, label, restOfString);
    trimString(restOfString, trimmedRestOfString, strlen(restOfString));
    /*check if .data .string .entry .extern*/
    instruction = getInstruction(trimmedRestOfString, instructions, totalInstructions, operand);
    if (instruction >= 0) {
        if (instruction == 2) {
            parseInstructionResult = parseEntrySecondPass(lineNumber, operand, allLabelsList);
        }
        else {
            parseInstructionResult = 1;
        }
    }
    else {
        /*must be recognized operation (mov, bne etc.)*/
        instruction = getInstruction(trimmedRestOfString, operations, totalOperations, operand);
        parseInstructionResult = parseOperatorSecondPass(lineNumber, operand, IC, instruction, commandsArray, allLabelsList, externList);
    }
    return parseInstructionResult;
}

int main()
{
    char commandsArray[100][MAX_WORD_LEN+1];
    char dataArray[100][MAX_WORD_LEN+1];
    int IC, DC, errorFlag, parseInstructionResult;
    IC = DC = errorFlag = parseInstructionResult = 0;
    labelList *allLabelsList = createList();
    char trimmedLine[MAX_TOKEN_LEN+1];
    char lines[][MAX_TOKEN_LEN+1] = {"; file ps.as", "\n", ".entry LENGTH", ".extern W", 
        "MAIN: mov r3, LENGTH", "LOOP: jmp L1(#-1,r6)", 
        "prn #-5", "bne W(r4,r5)", "sub r1, r4", "bne L3", "L1: inc K", ".entry LOOP", "bne LOOP(K,W)",
        "END: stop", "STR: .string \"abcdef\"", "LENGTH: .data 6, -9 ,15", "K: .data 22", ".extern L3"};
    for (int row = 0; row <19; row++) {
        if (isStringEmpty(lines[row], strlen(lines[row]))) {
            continue;
        }
        trimString(lines[row], trimmedLine, strlen(lines[row]));
        if (trimmedLine[0] == COMMENT_CHAR) {
            continue;
        }
        if (!parseLine(trimmedLine, row+1, &DC, &IC, dataArray, commandsArray, allLabelsList)) {
            errorFlag++;
        }
    }
    incrementDataLabels (allLabelsList, IC);
    IC = 0;
    if (errorFlag > 0) {
        printf("There were %d errors\n", errorFlag);
        freeList(allLabelsList);
        exit(0);
    }
    labelList *externList = createList();
    errorFlag = 0;
    for (int row = 0; row <19; row++) {
        if (isStringEmpty(lines[row], strlen(lines[row]))) {
            continue;
        }
        trimString(lines[row], trimmedLine, strlen(lines[row]));
        if (trimmedLine[0] == COMMENT_CHAR) {
            continue;
        }
        if (!parseLineSecondPass(trimmedLine, row+1, &IC, commandsArray, allLabelsList, externList)) {
            errorFlag++;
        }
    }
    
    for (int i=0; i < IC; i++) {
        printf("%s\n", commandsArray[i]);
    }
    for (int j=0; j < DC; j++) {
        printf("%s\n", dataArray[j]);
    }
    printList(externList);
    freeList(allLabelsList);
    freeList(externList);

    return 0;
}