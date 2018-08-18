#ifndef STRING_PARSING_FILE_H
#define STRING_PARSING_FILE_H

int isStringEmpty (char *str, int bytes);
void trimString (char *src, char *dest, int srcLen);
char* getNextDelimiterPos(char *str, char delimiter);
int splitString(char *str, char targetArray[][MAX_TOKEN_LEN+1], char delimiter);
int indexOf (char stringArray[][MAX_TOKEN_LEN+1], int arrayLen, char searchString[]);
int hasWhitespace (char *str);
#endif