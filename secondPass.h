#ifndef SECOND_PASS_FILE_H
#define SECOND_PASS_FILE_H

/*Function that receives a file pointer to an input file, a pointer to the instructions counter, an array to save instructions, the labels list and an
empty labels list to save external labels in, and parses the file line by line. In this second pass, all labels are translated to memory words based on
their value, and external labels are added to the externals list. Returns 1 if an error was found in any line, and 0 otherwise*/
int parseFileSecondPass(FILE *fp, int *IC, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList);

#endif