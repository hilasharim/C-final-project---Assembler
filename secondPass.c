#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"
#include "integerParsing.h"
#include "operandParsing.h"
#include "symbolsTable.h"
#include "secondPass.h"

/*Function to build a memory word for a label. Receives the label's address and whether it is realocatable or external. builds the 12 most significant
bits based on the translation of the label's address to and integer represented in base 2, and the 2 most insignificant bits are ARE, based on the RFlag
and the EFlag*/
static void buildLabelAddressWord(int labelAddress, int RFlag, int EFlag, char commandTarget[]) {
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

/*Function to parse a label operand in the second pass. Checks first that the label exists in the labels list and gets its values. If it does exist,
builds the memory word for the label. If the label is marked as external, adds it to the externals list.*/
static int parseLabelOperandSecondPass(char labelName[], char commandTarget[], labelList *allLabelsList, labelList *externList, int commandPosition) {
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

/*Function to parse a single operand in the second pass. Returns 0 if label not found, 1 if successful. Function always skips the first word.
If the addressing method of the operator is 0 or 3, only increments IC. Otherwise, builds the memory word for the label in addressing method 1 and 2. If
addressing method 2, sends the internal parameters to be parsed.*/
static int parseOneOperandCommandSecondPass(char *operand, char commandsArray[][MAX_WORD_LEN+1], int *IC, labelList *allLabelsList, labelList *externList) {
    int addressingMethod, currOperand;
    char operandValues[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressing[MAX_PARAMETERS];
    addressingMethod = parseSingleOperand(operand, operandValues, operandAddressing);
    (*IC)++; /*skip first word*/
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

/*Function to handle an operator expecting two operands in the second pass. Splits the operands and gets their addressing method, increments IC as
needed and sends each operand with addressing method 1 to be parsed.*/
static int parseOperatorWithTwoOperandsSecondPass(char *string, char commandsArray[][MAX_WORD_LEN+1], int *IC, labelList *allLabelsList, labelList *externList) {
    int srcOperandAddressing, destOperandAddressing;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    getTwoOperands(string, splitOperands);
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

/*Function to parse an operand based on the operator type in the second pass. Receives the operand, the instructions counter, 
the operator index, the label list and the externals list, and sends the operand to be parsed based on the number of parameters the operator
is expected to have.*/
static int parseOperatorSecondPass(int lineNumber, char *operand, int *IC, int operatorIndex, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    int operatorParseResult = 0;
    trimString(operand, trimmedOperand, strlen(operand));
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

/*Function to parse a .entry instruction in the second pass. Tries to set the entryFlag of the label in the labels list. Returns 1 is successful,
and 0 if the label is not found in the label list or if the label is already declared as external.*/
static int parseEntrySecondPass(int lineNumber, char labelname[MAX_TOKEN_LEN+1], labelList *allLabelsList) {
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

/*Function to parse a single line in the second pass. Returns 0 if an error was found in the line and 1 otherwise.
Ignores the label at the beginning of the line, if it exists. Gets the instruction and sends the operand to be parsed according to instruction found.
As this is the second pass, label names and instruction names must be legal, so this is not confirmed again in this function.*/
static int parseLineSecondPass(char *line, int lineNumber, int *IC, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList) {
    char label[MAX_TOKEN_LEN+1], restOfString[MAX_TOKEN_LEN+1], trimmedRestOfString[MAX_TOKEN_LEN+1], operand[MAX_TOKEN_LEN+1];
    int instruction, parseInstructionResult;
    
    getLabel(line, label, restOfString);
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

/*Function that receives a file pointer to an input file, a pointer to the instructions counter, the commands array that was generated in the first
pass, the label list generated in the first pass, and an empty externals list to save labels marked as externals and the memory location of the commands
that use them, and parses the file line by line. Returns 1 if an error was found in any line, and 0 otherwise.*/
int parseFileSecondPass(FILE *fp, int *IC, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList) {
    int errorFlag, lineNumber;
    char line[MAX_LINE_LEN+1];
    char trimmedLine[MAX_LINE_LEN+1];
    errorFlag = 0;
    lineNumber = 1;
    while (fgets(line, MAX_LINE_LEN+1, fp)) {
        if (isStringEmpty(line, strlen(line))) {
            lineNumber++;
            continue;
        }
        trimString(line, trimmedLine, strlen(line));
        if (trimmedLine[0] == COMMENT_CHAR) {
            lineNumber++;
            continue;
        }
        if (!parseLineSecondPass(trimmedLine, lineNumber, IC, commandsArray, allLabelsList, externList)) {
            errorFlag=1;
        }
        lineNumber++;
    }
    return errorFlag;
}