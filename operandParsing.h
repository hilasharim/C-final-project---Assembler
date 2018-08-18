#ifndef OPERAND_PARSING_FILE_H
#define OPERAND_PARSING_FILE_H

int isLabelLegal (char *label);
int isStringLegal (char *str);
int parseDataArray(char *parametersString, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos);
int parseStringInstruction (char *string, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos);
int getLabel(char *string, char labelTarget[MAX_TOKEN_LEN+1], char restOfString[MAX_TOKEN_LEN+1]);
int getTwoOperands(char *string, char operandTargetArray[][MAX_TOKEN_LEN+1]);
int parseSingleOperand(char *operand, char operandValuesArray[][MAX_TOKEN_LEN+1], int operandAddressingMethod[]);
int getInstruction(char *string, char instructionsArray[][MAX_TOKEN_LEN+1], int instructionsArrayLen, char operand[MAX_TOKEN_LEN+1]);

#endif