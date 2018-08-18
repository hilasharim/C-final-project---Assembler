#ifndef OPERAND_PARSING_FILE_H
#define OPERAND_PARSING_FILE_H

/*Function that checks if label name is legal. Returns -1 if reserved word, 0 if not reserved but not legal, and 1 if legal.*/
int isLabelLegal (char *label);
/*Function to check if a given string is legal for a .string instruction*/
int isStringLegal (char *str);
/*Function to parse an operand given after a .data instruction. All numbers are translated to their binary representation and inserted to dataTargetArray
starting at the specified position. Returns -1 if any token in the operand was null, -2 if any token is not a legal integer and the number of integers
added to the array otherwise.*/
int parseDataArray(char *parametersString, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos);
/*Function to parse an operand given after a .string instruction. Returns 0 if the string is not legal, and the string's length including \0 otherwise.*/
int parseStringInstruction (char *string, char dataTargetArray[][MAX_WORD_LEN+1], int targetStartPos);
/*Function to check if a label exists at the beginning of a string. returns 1 if legal label was found, -2 if no label was found, -1 if the label 
is a reserved word, and 0 if label not legal for another reason.*/
int getLabel(char *string, char labelTarget[MAX_TOKEN_LEN+1], char restOfString[MAX_TOKEN_LEN+1]);
/*Function to split a string that should be comprised of two operands to the single operands comprising it. Returns -1 if a comma was found in a 
wrong position, -2 if more or less than two operands were found and 1 otherwise.*/
int getTwoOperands(char *string, char operandTargetArray[][MAX_TOKEN_LEN+1]);
/*Function to parse a single operand. Returns -1 if operand is not legal, -2 if label is not legal, and the operand's addressing type if the operand is legal. 
Copies the value of the operand to the operandValuesArray, max 3 parameters (for addressing 2)*/
int parseSingleOperand(char *operand, char operandValuesArray[][MAX_TOKEN_LEN+1], int operandAddressingMethod[]);
/*Function to get the instruction or operator at the beginning of a string. Returns -1 if no instruction was found or the found instruction is not found in
instructionsArray, otherwise returns the index of the found instruction in instructionArray. Copies the instruction's operand (the rest of the string) to 
operand. Receives a string to find an instruction in, and an array of possible instructions.*/
int getInstruction(char *string, char instructionsArray[][MAX_TOKEN_LEN+1], int instructionsArrayLen, char operand[MAX_TOKEN_LEN+1]);

#endif