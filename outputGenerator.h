#ifndef OUTPUT_GENERATOR_FILE_H
#define OUTPUT_GENERATOR_FILE_H

void printObjectFile(char *originalFileName, char commandsArray[][MAX_WORD_LEN+1], char dataArray[][MAX_WORD_LEN+1], int IC, int DC);
void printExternalsFile(char *originalFileName, labelList *externList);
void printEntriesFile(char *originalFileName, labelList *allLabelsList);
#endif