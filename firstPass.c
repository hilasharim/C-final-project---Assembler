#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"
#include "integerParsing.h"
#include "operandParsing.h"
#include "symbolsTable.h"
#include "firstPass.h"

static void buildFirstCommandWord(int param1Addressing, int param2Addressing, int command, int srcAddressing, int destAddressing, int ARE, char commandTarget[]) {
    const char numTranslations[][3] = {"00", "01", "10", "11"};
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, numTranslations[param1Addressing], PARAMETER_LEN);
    strncpy(commandTarget+PARAMETER_LEN, numTranslations[param2Addressing], PARAMETER_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN, opcodes[command], MAX_OPCODE_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN, numTranslations[srcAddressing], ADDRESSING_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN+ADDRESSING_LEN, numTranslations[destAddressing], ADDRESSING_LEN);
    strncpy(commandTarget+2*PARAMETER_LEN+MAX_OPCODE_LEN+2*ADDRESSING_LEN, numTranslations[ARE], ARE_LEN);
}

static void buildRegistersWord(char *srcRegister, char *destRegister, char commandTarget[]) {
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

static void buildIntegerWord(char *intergerString, char commandTarget[]) {
    char tempNumOperand[MAX_WORD_LEN+1];
    parseInteger(intergerString, tempNumOperand);
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, tempNumOperand+ARE_LEN, MAX_WORD_LEN-ARE_LEN);
}

/*Parse operand of command that should have no operators. if the operator is null, copies the commad code into the position
indicated by IC and increments IC by 1*/
static int parseOperatorWithNoOperands(char *operand, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
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

static void buildWordOperatorAddressing013(char *operand, int operandAddressing, char commandTarget[], int destFlag) {
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

static void buildThreeCommandWords(int param1Addressing, int param2Addressing, int command, int srcOperandAddressing, char *srcOperand, int destOperandAddressing, char *destOperand, char commandsArray[][MAX_WORD_LEN+1], int *IC, int addressing2Flag) {
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
static int parseOneOperandCommand(char *operand, int command, int operandAllowedAddressingVals[], int numAllowedAddressingVals, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
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
static int parseOperatorWithTwoOperands(char *string, int command, int srcOperandAllowedAddressingVals[], int numSrcOperandAllowedAddressingVals, int destOperandAllowedAddressingVals[], int numDestOperandAllowedAddressingVals, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
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

/*return -1 if null found on split, -2 if not integer, 0 if label already in list*/
static int dataStringInstruction(int lineNumber, int instruction, char *operand, char dataArray[][MAX_WORD_LEN+1], char label[MAX_TOKEN_LEN+1], int existsLabelFlag, labelList *allLabelsList, int *DC) {
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

static int externInstruction(int lineNumber, char *operand, labelList *allLabelsList) {
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

static int entryInstruction(int lineNumber, char *operand) {
    if (!isLabelLegal(operand)) {
        fprintf(stderr, "Error in line %d: Illegal label name in .entry instruction\n", lineNumber);
        return 0;
    }
    return 1;
}

static int parseInstructions(int lineNumber, char *operand, int *DC, int instructionIndex, char dataArray[][MAX_WORD_LEN+1], char label[MAX_TOKEN_LEN+1], int existsLabelFlag, labelList *allLabelsList) {
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

static int parseOperator(int lineNumber, char *operand, int *IC, int operatorIndex, char commandsArray[][MAX_WORD_LEN+1], int existsLabelFlag, char label[MAX_TOKEN_LEN+1], labelList *allLabelsList) {
    char trimmedOperand[MAX_TOKEN_LEN+1];
    int operatorParseResult = 0;
    trimString(operand, trimmedOperand, strlen(operand));
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

static int parseLine(char *line, int lineNumber, int *DC, int *IC, char dataArray[][MAX_WORD_LEN+1], char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList) {
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

int parseFileFirstPass(FILE *fp, int *DC, int *IC, char dataArray[][MAX_WORD_LEN+1], char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList) {
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
        if (!parseLine(trimmedLine, lineNumber, DC, IC, dataArray, commandsArray, allLabelsList)) {
            errorFlag=1;
        }
        lineNumber++;
    }
    return errorFlag;
}