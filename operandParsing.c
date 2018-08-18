#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"
#include "integerParsing.h"
#include "operandParsing.h"

/*Function that checks if label name is legal. Returns -1 if reserved word, 0 if not reserved but not legal, and 1 if legal.
A legal label starts with a letter and the rest of its characters are alphanumeric.*/
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

/*Function to check if a given string is legal for a .string instruction, by checking if it is enclosed
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

/*Function to parse an operand given after a .data instruction. All numbers are translated to their binary representation and inserted to dataTargetArray
starting at the specified position. Returns -1 if any token in the operand was null, -2 if any token is not a legal integer and the number of integers
added to the array otherwise. Function first splits the operand using ',' as the delimiter, then checks that none of the tokens are null (indicates
comma in a wrong position) and that all tokens can be translated to integers. Then goes over all integers one by one, translates them and inserts
the translation to the correct position in the target array.*/
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

/*Function to parse an operand given after a .string instruction. Returns 0 if the string is not legal, and the string's length including \0 otherwise.
Checks first the the operand is a legal string, then converts the ASCII values of its characters to base 2 and inserts them to dataTargetArray at the
specified position*/
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

/*Function to check if a label exists at the beginning of a string. returns 1 if legal label was found, -2 if no label was found, -1 if the label 
is a reserved word, and 0 if label not legal for another reason. copies trimmed label to the target array. Works by finding the position of the first ':',
and treating the string until that position as the label*/
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

/*Function to split a string that should be comprised of two operands to the single operands comprising it. Returns -1 if a comma was found in a 
wrong position, -2 if more or less than two operands were found and 1 otherwise. copies the trimmed operands to operandTargetArray */
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

/*Function to copy the three operands comprising addressing method 2 to operandValuesArray. Returns 0 if structure does not correspond to addressing method 2, 
and 1 otherwise.*/
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

/*Function to parse a single operand. Returns -1 if operand is not legal, -2 if label is not legal, and the operand's addressing type if the operand is legal. 
Copies the value of the operand to the operandValuesArray, max 3 parameters (for addressing 2)*/
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

/*Function to get the instruction or operator at the beginning of a string. Returns -1 if no instruction was found or the found instruction is not found in
instructionsArray, otherwise returns the index of the found instruction in instructionArray. Copies the instruction's operand (the rest of the string) to 
operand. Receives a string to find an instruction in, and an array of possible instructions. Finds the next position of a space or a tab, and checks if the word
until that postion appears in instructionsArray.*/
int getInstruction(char *string, char instructionsArray[][MAX_TOKEN_LEN+1], int instructionsArrayLen, char operand[MAX_TOKEN_LEN+1]) {
    char *delimPosSpace, *delimPosTab, *delimPos;
    char instructionNotTrimmed[MAX_TOKEN_LEN+1];
    char instructionTrimmed[MAX_TOKEN_LEN+1];
    delimPosSpace = getNextDelimiterPos(string, ' ');
    delimPosTab = getNextDelimiterPos(string, '\t');
    
    if (*delimPosSpace == '\0' && *delimPosTab == '\0') { 
        strcpy(instructionNotTrimmed, string);
        operand[0] = '\0';
    }
    else {
        delimPos = delimPosSpace <= delimPosTab ? delimPosSpace:delimPosTab;
        strncpy(instructionNotTrimmed, string, delimPos - string);
        instructionNotTrimmed[delimPos - string] = '\0';
        strncpy(operand, delimPos+1, strlen(string) - (delimPos - string +1));
        operand[strlen(string) - (delimPos - string +1)] = '\0';
    }
    trimString(instructionNotTrimmed, instructionTrimmed, strlen(instructionNotTrimmed));
    return indexOf(instructionsArray, instructionsArrayLen, instructionTrimmed);
} 