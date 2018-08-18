#ifndef STRING_PARSING_FILE_H
#define STRING_PARSING_FILE_H

/*Function to check if the given number of bytes in the string are only whitespace characters.*/
int isStringEmpty (char *str, int bytes);
/*Function to trim whitespace characters from beginning and end of the src string. the trimmed string is copied into dest*/
void trimString (char *src, char *dest, int srcLen);
/*Function that looks for the first appearance of a given character in a string. returns a pointer to the corresponding location in the string*/
char* getNextDelimiterPos(char *str, char delimiter);
/*Function to split a string into tokens by a given delimiter character. the found tokens are copied into the targetArray. Returns the total
number of tokens copied*/
int splitString(char *str, char targetArray[][MAX_TOKEN_LEN+1], char delimiter);
/*Function to search for string in given string array. return first position in array, or -1 if not found */
int indexOf (char stringArray[][MAX_TOKEN_LEN+1], int arrayLen, char searchString[]);
/*Function to check if a string contains any whitespace characters. returns 1 if any whitespace found, otherwise returns 0.*/
int hasWhitespace (char *str);
#endif