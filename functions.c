#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LEN 80
#define MAX_OPCODE_LEN 4
#define MAX_REGISTER_LEN 6
#define MAX_PARAMETERS 200
#define STRING_ENCLOSER '\"'
#define COMMENT_CHAR ';'
#define MAX_WORD_LEN 14
#define ZERO_ASCII_VAL 48


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

const int totalRegisters = 8;
const int totalInstructions = 4;
const int totalOperations = 16;

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

/*function that finds and returns a pointer to the next position in the string that is equal to the given delimiter character. if the delimiter
is not found, returns a pointer to the end of the string.*/
char* getNextDelimiterPos(char *str, char delimiter) {
    char *delimiterPos = strchr(str, delimiter);
    if (!delimiterPos) {
        delimiterPos = str + strlen(str);
    }
    return delimiterPos;
}

/*function to split a given string into an array of strings by a given delimiter character. function returns total words parsed.
function uses the getNextDelimiterPos to find the next position of the delimiter, until it reaches the end of the string, and copies the
part of the string from the last delimiter found to the current delimiter found (the delimiters themselves are not included in the copied word).
if there are two consecutive delimiters or a delimiter at the start or at the end of the string, null strings ("") are inserted into the target
word array.*/
int splitString(char *str, char targetArray[][MAX_TOKEN_LEN+1], char delimiter) {
    int currTargetArrayPos, totalParsed;
    currTargetArrayPos = totalParsed = 0;
    char *prevDelimPos, *currDelimPos;
    prevDelimPos = str-1;
    while (*str != '\0') {
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

/*function to search for string in given string array. return first position in array, or -1 if not found */
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

/*function that checks if label name is legal. return -1 if reserved word, 0 if not reserved but not legal, and 1 if legal*/
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

/*function to check if a string can be translated into a legal integer - contains a plus or minus sign at the beginning and only numeric 
values after that. return 0 if not legal, otherwise return 1*/
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

/*function to check if a string contains any whitespace characters. returns 1 if whitespace found, otherwise 0*/
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
by the specified legal enclosing character and contains only printable characters. returns 0 if not legal, otherwise 1.*/
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
void parseInteger (char *stringNumber, char *baseTwoRep) {
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
        printf("%s\n", dataTargetArray[targetStartPos+currPos]);
    }
    return 1;
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
    parseInteger("0", dataTargetArray[targetStartPos+currPos-1]); /*add the \0 at the end of the string*/
    return strlen(string)-1;
}

/*function to check if a label exists at the beginning of a string. returns 1 if legal label was found, returns -2 if no label was found. returns -1 if
the label is a reserved word, returns 0 if label not legal for another reason. copies trimmed label to the target array even if not legal.*/
int getLabel(char *string, char labelTarget[MAX_TOKEN_LEN+1], char restOfString[MAX_TOKEN_LEN+1]) {
    char *delimPos;
    char labelNotTrimmed[MAX_TOKEN_LEN+1];
    int legalLabel;
    delimPos = getNextDelimiterPos(string, ':');
    if (*delimPos == '\0') { /*':' was not found, there is no label*/
        strcpy(restOfString, string);
        return -2;
    }
    else {
        strncpy(labelNotTrimmed, string, delimPos - string);
        labelNotTrimmed[delimPos - string] = '\0';
        strcpy(restOfString, delimPos+1);
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
    //check if there are more than 2 operands
    if (numSplitParams > 2) {
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

/*function to parse a single instruction operand. returns -1 if operand is not legal, -2 if label not legal 
addressing type if the operand is legal. copies the value of the operand to the operandValuesArray,
max 3 parameters (for addressing 2), for addressing method 2 also writes the addressing method of the two operands in parantheses to the
operandAddressingMethod array*/
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

int main()
{
    char string[] = "\"abce\"";
    char dataParams[] = "4, 5, 6";
    char dataArray[MAX_PARAMETERS][MAX_WORD_LEN+1]; 
    int dc = 0;
    dc += parseDataArray(dataParams, dataArray, dc);
    dc += ParseStringInstruction (string, dataArray, dc);
    for (int i=0; i<dc; i++) {
        printf("%s\n", dataArray[i]);
    }
    
    char string[MAX_TOKEN_LEN+1] = ".data(r3,r2)";
    char operandValuesArray[MAX_PARAMETERS][MAX_TOKEN_LEN+1];
    int operandMethods[2];
    int res = parseSingleOperand(string, operandValuesArray, operandMethods);
    printf("%d\n", res);
    printf("%s %s %s\n", operandValuesArray[0], operandValuesArray[1], operandValuesArray[2]);
    printf("%d %d\n", operandMethods[0], operandMethods[1]);

    return 0;
}
