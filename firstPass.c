#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"
#include "integerParsing.h"
#include "operandParsing.h"
#include "symbolsTable.h"
#include "firstPass.h"

/*Function to build the first memory word of every operator. Receives as input the required parameters regarding the operands and the index of the command.
Builds the word according to the given parameters and the opcode of the given command*/
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

/*Function to build a register memory word, given two registers. Copies the register values corresponding to the given registers to the correct
bits in the word. To build a word for one register, call this function with a null string to replace the unwanted register.*/
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

/*Function to build the memory word of an integer. Converts the integer to base 2, copies the 12 most insignificant bits to the 12 most significant bits.
The last two bits are reserved for ARE, which in this case are 00.*/
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

/*Function to build a memory word for an operand with addressing methods 0, 1 or 3. If the addressing method is 0 - calls buildIntegerWord,
if the addressing method is 1 - skips and increments IC, if the addressing method is 3 - calls buildRegistersWord*/
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

/*Function to build three memory words. Recieves the parameter addressing, operator addressing, the command and the operands, builds the words and
inserts them into the specified position in the commands array. If both addressing methods are registers, only one word is built.*/
static void buildThreeCommandWords(int param1Addressing, int param2Addressing, int command, int srcOperandAddressing, char *srcOperand, int destOperandAddressing, char *destOperand, char commandsArray[][MAX_WORD_LEN+1], int *IC, int addressing2Flag) {
    buildFirstCommandWord(param1Addressing, param2Addressing, command, srcOperandAddressing, destOperandAddressing,0,commandsArray[*IC]);
    (*IC)++;
    /*If addressing method 2, treat the internal parameters as operands*/
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

/*Function to handle an operator that expects one operand. Returns 1 if successful, 0 if the addressing method is 2 and an internal parameter has an
illegal addressing method, and -1 if the operand itself has an illegal addressing method. The function receives as a parameter an array of the legal
addressing methods for the operand, and checks that its actual addressing method appears in the array. Calls a function to build the first memory
word of the command itself, then calls a function to build the rest of the memory words, based on the addressing method.*/
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

/*Function to handle an operator that expects 2 operands. Returns 1 if successful, 0 if an operand has an illegal addressing method, -2 if more or
less than 2 operands are found, and -1 if a comma is found in an illegal position. Uses the function to get the 2 operands, then sends each operand
to parsing separately, and checks that both addressing methods are legal. Then sends the operands to build their corresponding memory words*/
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

/*Function to parse .data and .string instructions. Returns 1 if successful, -1 if comma was found in an illegal position, -1 if a non-integer was
passed to a .data instruction and 0 if the label is already in the labels list. If a label exists, checks first that it does not already exist in the 
labels list. If if doesn't, adds the label to the list as realocatable, with a data flag. Then sends the operand for parsing based on the instruction*/
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

/*Function to pase an extern instruction. Checks that the label following the instruction is legal and that it does not already exist in the labels list.
If both test pass, adds the label to the label list as an external label with value 0, and returns 1. If one of the tests fail, returns 0.*/
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

/*Function to parse an entry instruction. checks if the label following the instruction is legal, and returns 1 it is. Otherwise returns 0.*/
static int entryInstruction(int lineNumber, char *operand) {
    if (!isLabelLegal(operand)) {
        fprintf(stderr, "Error in line %d: Illegal label name in .entry instruction\n", lineNumber);
        return 0;
    }
    return 1;
}

/*Function to parse and operand following an instruction. Sends the operator to parsing based on the instruction type. Returns the result of the corresponding
parsing function*/
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

/*Function to parse an operand based on the operator type. Receives the operand, the instructions counter, the target commands array,
whether there was a label, the label's name and the labels list. Returns 0 if an error is encountered, and 1 otherwise.
If a label exists, adds it to the list if the name does not already exist. checks how many parameters the operator should have, and sends the operand
to the corresponding parsing function.*/
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

/*Function to parse a single line in the first pass. Returns 0 if an error was found in the line and 1 otherwise.
Function checks if a legal label exists at the beginning of the line, then gets the instruction or the operator, and sends the operand to parsing
according to the found instruction.*/
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

/*Function that receives a file pointer to an input file, pointers to the instructions counter and the data counter, and arrays to save instructions
and data, and parses the file line by line. Returns 1 if an error was found in any line, and 0 otherwise.*/
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