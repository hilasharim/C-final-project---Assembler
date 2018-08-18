#ifndef INTEGER_PARSING_FILE_H
#define INTEGER_PARSING_FILE_H

int indexOfInt (int intArray[], int arrayLen, int searchTerm);
int isInteger(char *str);
void baseTenToBaseTwo (char *baseTenNumber, char *baseTwoNumber);
void twosComplement (char *baseTwoPositiveNumber, char *twosComplementRepresentation);
void parseInteger (char *stringNumber, char baseTwoRep[MAX_WORD_LEN]);

#endif