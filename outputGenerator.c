#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "symbolsTable.h"
#include "outputGenerator.h"

#define OUTPUT_SYMBOL_ONE '/'
#define OUTPUT_SYMBOL_ZERO '.'

/*Function that receives a memory words encoded as 0's and 1's and translates them into the required format, given by OUTPUT_SYMBOL_ONE
and OUTPUT_SYMBOL_ZERO*/
static void translateWordForPrint(char *wordToTranslate, char *destinationWord) {
    int currChar;
    for (currChar = 0; currChar < MAX_WORD_LEN; currChar++) {
        if (wordToTranslate[currChar] == '0') {
            destinationWord[currChar] = OUTPUT_SYMBOL_ZERO;
        }
        else {
            destinationWord[currChar] = OUTPUT_SYMBOL_ONE;
        }
    }
    destinationWord[currChar] = '\0';
}

/*Function to generate an object file. Receives the commandsArray and dataArray generated in the first and second passes, and the total number
of commands (IC) and the total number of data words (DC). goes over the arrays word by word - first the commands array and then the data array, translates
each word to the required output format, and prints it to the output file*/
void printObjectFile(char *originalFileName, char commandsArray[][MAX_WORD_LEN+1], char dataArray[][MAX_WORD_LEN+1], int IC, int DC) {
    char printWord[MAX_WORD_LEN+1];
    char outputFileName[MAX_FILE_NAME+1];
    int currCommand = 0;
    FILE *outputFilePointer;
    strcpy(outputFileName, originalFileName);
    strcat(outputFileName, OUTPUT_OBJECT_FILE_EXTENSION);
    if ((outputFilePointer = fopen(outputFileName, "w")) == NULL) {
        fprintf(stdout, "Unable to open file: %s for writing output\n", outputFileName);
    }
    fprintf(outputFilePointer, "    %d    %d\n", IC, DC);
    for (currCommand = 0; currCommand < IC; currCommand++) {
        translateWordForPrint(commandsArray[currCommand], printWord);
        fprintf(outputFilePointer, "%04d    %s\n", currCommand + MEMORY_START_POS, printWord);
    }
    for (currCommand = 0; currCommand < DC; currCommand++) {
        translateWordForPrint(dataArray[currCommand], printWord);
        fprintf(outputFilePointer, "%04d    %s\n", currCommand + IC + MEMORY_START_POS, printWord);
    }
    fclose(outputFilePointer);
}

/*Function to create an output file containing all labels declared as externals and the positions of the commands in which they are used.
Receives a prefix for the file name and the externals list generated in the second pass, and uses the printListToFile function to print the
names and values of all members of the list*/
void printExternalsFile(char *originalFileName, labelList *externList) {
    char outputFileName[MAX_FILE_NAME+1];
    FILE *outputFilePointer;
    int numPrinted = 0;
    strcpy(outputFileName, originalFileName);
    strcat(outputFileName, OUTPUT_EXTERNALS_FILE_EXTENSION);
    if ((outputFilePointer = fopen(outputFileName, "w")) == NULL) {
        fprintf(stdout, "Unable to open file: %s for writing output\n", outputFileName);
    }
    numPrinted = printListToFile (outputFilePointer, externList);
    fclose(outputFilePointer);
    if (numPrinted == 0) {
        remove(outputFileName);
    }
}

/*Function to create an output file containing all labels declared as entry and their values. Receives a prefix for the file name and the labels list generated
in the first and second passes, and uses the printEntryLabelsToFile function to print the names and values of all members of the list declared as entry*/
void printEntriesFile(char *originalFileName, labelList *allLabelsList) {
    char outputFileName[MAX_FILE_NAME+1];
    FILE *outputFilePointer;
    int numPrinted = 0;
    strcpy(outputFileName, originalFileName);
    strcat(outputFileName, OUTPUT_ENTRIES_FILE_EXTENSION);
    if ((outputFilePointer = fopen(outputFileName, "w")) == NULL) {
        fprintf(stdout, "Unable to open file: %s for writing output\n", outputFileName);
    }
    numPrinted = printEntryLabelsToFile (outputFilePointer, allLabelsList);
    fclose(outputFilePointer);
    if (numPrinted == 0) {
        remove(outputFileName);
    }
}
