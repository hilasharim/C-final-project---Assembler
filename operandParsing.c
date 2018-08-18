#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"
#include "integerParsing.h"
#include "operandParsing.h"

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

int parseDataArray(char *parametersString, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos) {
    char stringSplitRes[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char stringSplitTrimRes[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int numSplitParams, currPos, nullPos;
    
    numSplitParams = splitString(parametersString, stringSplitRes, ','); /*split the string into tokens*/
    /*trim all tokens*/
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        trimString(stringSplitRes[currPos], stringSplitTrimRes[currPos], strlen(stringSplitRes[currPos]));
    }
    /*check if any token in null*/
    if ((nullPos = indexOf(stringSplitTrimRes, numSplitParams, "")) >= 0) {
        return -1;
    }
    /*check that all tokens are legal integers*/
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        if (!isInteger(stringSplitTrimRes[currPos])) {
            return -2;
        }
    }
    /*convert all integers to base 2 and store them in the target array, starting at targetStartPos*/
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
    /*parse ASCII values of string characters while skipping "". save base 2 values in dataTargetArray*/
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
    /*trim all tokens*/
    for (currPos = 0; currPos < numSplitParams; currPos++) {
        trimString(internalParams[currPos], operandTargetArray[currPos], strlen(internalParams[currPos]));
    }
    /*check if any token in null*/
    if ((nullPos = indexOf(operandTargetArray, numSplitParams, "")) >= 0) {
        return -1;
    }
    /*check if there are more than 2 operands or less than 2*/
    if (numSplitParams != 2) {
        return -2;
    }
    return 1;
}

/*copy the three operands comprising addressing method 2 to operandValuesArray. return 0 if
structure does not correspond to addressing method 2, and 1 otherwise*/
static int parseAddressingMethod2(char *operand, char operandValuesArray[][MAX_TOKEN_LEN+1]) {
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
    int operandOneAddressingMethod, operandTwoAddressingMethod;
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
    return -1;
}

/*return -1 if no instruction was found or not in array, otherwise return the index of the found instruction in instructionArray*/
int getInstruction(char *string, char instructionsArray[][MAX_TOKEN_LEN+1], int instructionsArrayLen, char operand[MAX_TOKEN_LEN+1]) {
    char *delimPos;
    char instructionNotTrimmed[MAX_TOKEN_LEN+1];
    char instructionTrimmed[MAX_TOKEN_LEN+1];
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