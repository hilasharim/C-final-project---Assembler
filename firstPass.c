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

/*only the destination operand code is relevant*/
int parseOneOperandAddressing013(char *operand, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int addressingMethod, reg;
    char operandValues[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressing[1]; /*dummy variable needed for function*/
    char tempNumOperand[MAX_WORD_LEN+1];
    
    addressingMethod = parseSingleOperand(operand, operandValues, operandAddressing);
    if (addressingMethod != 0 && addressingMethod != 1 && addressingMethod != 3) {
        return -1;
    }
    buildFirstCommandWord(0,0,command,0,addressingMethod,0, commandsArray[*IC]);
    (*IC)++;
    buildWordOperatorAddressing013(operandValues[0], addressingMethod, commandsArray[*IC], 1);
    (*IC)++;
    return 1;
}

/*only the destination operand code is relevant*/
int parseOneOperandAddressing13(char *operand, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int addressingMethod, reg;
    char operandValues[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressing[1]; /*dummy variable needed for single operand parsing function*/
    char tempNumOperand[MAX_WORD_LEN+1];
    
    addressingMethod = parseSingleOperand(operand, operandValues, operandAddressing);
    if (addressingMethod != 1 && addressingMethod != 3) {
        return -1;
    }
    buildFirstCommandWord(0,0,command,0,addressingMethod,0, commandsArray[*IC]);
    (*IC)++;
    buildWordOperatorAddressing013(operandValues[0], addressingMethod, commandsArray[*IC], 1);
    (*IC)++;
    return 1;
}

void buildThreeCommandWords(int command, int srcOperandAddressing, char *srcOperand, int destOperandAddressing, char *destOperand, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    buildFirstCommandWord(0,0,command,srcOperandAddressing,destOperandAddressing,0,commandsArray[*IC]);
    (*IC)++;
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

/*return -1 if comma in wrong position, -2 if more than 2 operands, 0 if wrong addressing method, 1 if legal*/
int parseOperatorWithTwoOperands01313(char *string, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int srcOperandAddressing, destOperandAddressing, exactlyTwoOperands;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    exactlyTwoOperands = getTwoOperands(string, splitOperands);
    if (exactlyTwoOperands != 1) {
        return exactlyTwoOperands;
    }
    srcOperandAddressing = parseSingleOperand(splitOperands[0], operandValues, operandAddressingArray);
    destOperandAddressing = parseSingleOperand(splitOperands[1], operandValues+1, operandAddressingArray);
    if (srcOperandAddressing != 0 && srcOperandAddressing != 1 && srcOperandAddressing != 3) {
        return 0;
    }
    if (destOperandAddressing != 1 && destOperandAddressing != 3) {
        return 0;
    }
    buildThreeCommandWords(command, srcOperandAddressing, operandValues[0], destOperandAddressing, operandValues[1], commandsArray, IC);
    return 1;
}

/*return -1 if comma in wrong position, -2 if more than 2 operands, 0 if wrong addressing method, 1 if legal*/
int parseOperatorWithTwoOperands013013(char *string, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int srcOperandAddressing, destOperandAddressing, exactlyTwoOperands;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    exactlyTwoOperands = getTwoOperands(string, splitOperands);
    if (exactlyTwoOperands != 1) {
        return exactlyTwoOperands;
    }
    srcOperandAddressing = parseSingleOperand(splitOperands[0], operandValues, operandAddressingArray);
    destOperandAddressing = parseSingleOperand(splitOperands[1], operandValues+1, operandAddressingArray);
    if (srcOperandAddressing != 0 && srcOperandAddressing != 1 && srcOperandAddressing != 3) {
        return 0;
    }
    if (destOperandAddressing != 0 && destOperandAddressing != 1 && destOperandAddressing != 3) {
        return 0;
    }
    buildThreeCommandWords(command, srcOperandAddressing, operandValues[0], destOperandAddressing, operandValues[1], commandsArray, IC);
    return 1;
}

/*return -1 if comma in wrong position, -2 if more than 2 operands, 0 if wrong addressing method, 1 if legal*/
int parseOperatorWithTwoOperands113(char *string, int command, char commandsArray[][MAX_WORD_LEN+1], int *IC) {
    int srcOperandAddressing, destOperandAddressing, exactlyTwoOperands;
    char splitOperands [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    char operandValues [MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandAddressingArray[MAX_PARAMETERS];
    
    exactlyTwoOperands = getTwoOperands(string, splitOperands);
    if (exactlyTwoOperands != 1) {
        return exactlyTwoOperands;
    }
    srcOperandAddressing = parseSingleOperand(splitOperands[0], operandValues, operandAddressingArray);
    destOperandAddressing = parseSingleOperand(splitOperands[1], operandValues+1, operandAddressingArray);
    if (srcOperandAddressing != 1) {
        return 0;
    }
    if (destOperandAddressing != 1 && destOperandAddressing != 3) {
        return 0;
    }
    buildThreeCommandWords(command, srcOperandAddressing, operandValues[0], destOperandAddressing, operandValues[1], commandsArray, IC);
    return 1;
}