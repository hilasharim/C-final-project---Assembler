#ifndef SECOND_PASS_FILE_H
#define SECOND_PASS_FILE_H

int parseFileSecondPass(FILE *fp, int *IC, char commandsArray[][MAX_WORD_LEN+1], labelList *allLabelsList, labelList *externList);

#endif