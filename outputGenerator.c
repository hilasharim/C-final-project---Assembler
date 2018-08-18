#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "symbolsTable.h"
#include "outputGenerator.h"

#define OUTPUT_SYMBOL_ONE '/'
#define OUTPUT_SYMBOL_ZERO '.'

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
