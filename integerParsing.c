#include <stdio.h>
#include <ctype.h>
#include "constants.h"
#include "integerParsing.h"

/*Function to search for an integer in an array of integers. returns the first position in the array in which it is found, or -1 if it is not found*/
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

/*function to check if a string can be translated into a legal integer - contains a plus or minus sign at the beginning and only numeric 
values after that. Returns 0 if not legal, otherwise returns 1*/
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

/*Function to convert a string representing positive integer in base 10 to a string of its representation in base two. if the representation 
in base two is longer than the max word length, the most significant bits are truncated. works by dividing the given number by two, and inserting 
the remainder as the current bit, right to left. will continue until number is zero or out of space*/
static void baseTenToBaseTwo (char *baseTenNumber, char *baseTwoNumber) {
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

/*Function to convert a base two representation of a positive integer to its two's complement representation of the negative number. 
goes over all bits right to left, until a '1' is encountered. leaves the first '1' unchanged, and flips all other bits to the left. if the two's complement 
can't be represented with given number of bits, the most significant bits are truncated.*/
static void twosComplement (char *baseTwoPositiveNumber, char *twosComplementRepresentation) {
    int currPos = MAX_WORD_LEN - 1;
    while (baseTwoPositiveNumber[currPos] == '0' && currPos >= 0) {
        twosComplementRepresentation[currPos] = baseTwoPositiveNumber[currPos];
        currPos--;
    }
    if (currPos >= 0) { /*ignore first '1' if present*/
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

/*Function to parse an integer to its base two representation. works by converting the positive part to base two. if the original number was negative, 
finds the two's complement of the positive number. Assumes that the number is a legal integer.*/
void parseInteger (char *stringNumber, char baseTwoRep[MAX_WORD_LEN]) {
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