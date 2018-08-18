#ifndef FIRST_PASS_FILE_H
#define FIRST_PASS_FILE_H

/*Function that receives a file pointer to an input file, pointers to the instructions counter and the data counter, and arrays to save instructions
and data, and parses the file line by line. In this first pass, all data will be translated, the symbols table is created, and all words that do 
not correspond to labels are added to the commands array. Returns 1 if an error was found in any line, and 0 otherwise*/
int parseFileFirstPass(FILE *fp, int *DC, int *IC, char dataArray[][MAX_WORD_LEN+1], char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList);

#endif