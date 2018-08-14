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

void buildFirstCommandWord(int param1Addressing, int param2Addressing, int command, int srcAddressing, int destAddressing, int ARE, char commandTarget[]) {
    const char numTranslations[][3] = {"00", "01", "10", "11"};
    strcpy(commandTarget, EMPTY_COMMAND);
    strncpy(commandTarget, numTranslations[param1Addressing], 2);
    strncpy(commandTarget+2, numTranslations[param2Addressing], 2);
    strncpy(commandTarget+4, opcodes[command], MAX_OPCODE_LEN);
    strncpy(commandTarget+8, numTranslations[srcAddressing], 2);
    strncpy(commandTarget+10, numTranslations[destAddressing], 2);
    strncpy(commandTarget+12, numTranslations[ARE], 2);
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