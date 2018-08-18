#include <ctype.h>
#include <string.h>
#include "constants.h"
#include "stringParsing.h"

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
static int getFirstNonWhiteSpaceCharacter(char *str) {
    int currPos = 0;
    while (isspace(str[currPos]))
        currPos++;
    return currPos;
}

/*function that returns the position of the last non-whitespace character in a string. assumes string is not only whitespace
(does not check string boundaries)*/
static int getLastNonWhiteSpaceCharacter(char *str, int strLen) {
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

/*Function that looks for the first appearance of a given character in a string. returns a pointer to the corresponding location in the string*/
char* getNextDelimiterPos(char *str, char delimiter) {
    char *delimiterPos = strchr(str, delimiter);
    if (!delimiterPos) {
        delimiterPos = str + strlen(str);
    }
    return delimiterPos;
}

/*Function to split a string into tokens by a given delimiter character. the found tokens are copied into the targetArray. Returns the total
number of tokens copied. Function uses the getNextDelimiterPos on the string until no more delimiters are found. for each delimiter position, the
string from the last delimiter position (not including) is copied until the position of the current delimiter position*/
int splitString(char *str, char targetArray[][MAX_TOKEN_LEN+1], char delimiter) {
    int currTargetArrayPos, totalParsed;
    char *prevDelimPos, *currDelimPos;
    currTargetArrayPos = totalParsed = 0;
    prevDelimPos = str-1;
    currDelimPos = str;
    while (*currDelimPos != '\0') {
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

/*Function to search for string in given string array. return first position in array, or -1 if not found */
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

/*Function to check if a string contains any whitespace characters. returns 1 if any whitespace found, otherwise returns 0.
Goes over string character by character*/
int hasWhitespace (char *str) {
    while (*str != '\0') {
        if (isspace(*str)) {
            return 1;
        }
        str++;
    }
    return 0;
}

