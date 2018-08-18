#ifndef FIRST_PASS_FILE_H
#define FIRST_PASS_FILE_H

int parseFileFirstPass(FILE *fp, int *DC, int *IC, char dataArray[][MAX_WORD_LEN+1], char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList);

#endif