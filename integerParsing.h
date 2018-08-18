#ifndef INTEGER_PARSING_FILE_H
#define INTEGER_PARSING_FILE_H

/*Function to search for an integer in an array of integers. returns the first position in the array in which it is found, or -1 if it is not found*/
int indexOfInt (int intArray[], int arrayLen, int searchTerm);
/*Function to check if a string can be translated into a legal integer. Returns 0 if not legal, otherwise returns 1*/
int isInteger(char *str);
/*Function to parse an integer, either positive or negative to its base two representation. Negative numbers are represented in 2's complement*/
void parseInteger (char *stringNumber, char baseTwoRep[MAX_WORD_LEN]);

#endif