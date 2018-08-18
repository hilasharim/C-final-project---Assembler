#ifndef OUTPUT_GENERATOR_FILE_H
#define OUTPUT_GENERATOR_FILE_H

/*Function to generate an object file from the commands array and data array generated in the first and second passes.*/
void printObjectFile(char *originalFileName, char commandsArray[][MAX_WORD_LEN+1], char dataArray[][MAX_WORD_LEN+1], int IC, int DC);
/*Function to create an output file containing all labels declared as externals and the positions of the commands in which they are used.
Receives a prefix for the file name and the externals list generated in the second pass*/
void printExternalsFile(char *originalFileName, labelList *externList);
/*Function to create an output file containing all labels declared as entry and their values. Receives a prefix for the file name and the labels list generated
in the first and second passes*/
void printEntriesFile(char *originalFileName, labelList *allLabelsList);
#endif