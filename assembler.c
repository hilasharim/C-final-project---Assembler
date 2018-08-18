#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "symbolsTable.h"
#include "firstPass.h"
#include "secondPass.h"
#include "outputGenerator.h"

int main(int argc, char *argv[]) {
    int currArgv, errorFlag;
    FILE *currFilePointer;
    char completeFileName[MAX_FILE_NAME+1];
    labelList *allLabelsList, *externList;
    
    if (argc < 2) {
        fprintf(stderr, "Missing command line parameter. Must provide at least one file.\n");
        exit(1);
    }
	/*go over all files*/
    for (currArgv = 1; currArgv < argc; currArgv++) {
        char commandsArray[MAX_COMMAND_ARRAY_LEN][MAX_WORD_LEN+1];
        char dataArray[MAX_COMMAND_ARRAY_LEN][MAX_WORD_LEN+1];
        int IC, DC;
        strcpy(completeFileName, argv[currArgv]);
        strcat(completeFileName, INPUT_FILE_EXTENSION);
        if ((currFilePointer = fopen(completeFileName, "r")) == NULL) {
            fprintf(stdout, "Unable to open file %s\n", completeFileName);
            continue;
        }
        fprintf(stdout, "Parsing file: %s, first pass\n", completeFileName);
        IC = DC = 0;
        allLabelsList = createList();
        errorFlag = parseFileFirstPass(currFilePointer, &DC, &IC, dataArray, commandsArray, allLabelsList);
        if (errorFlag) {
            fprintf(stdout, "Errors found in first pass in file %s\n", completeFileName);
            fclose(currFilePointer);
            freeList(allLabelsList);
            continue;
        }
        /*increment data labels by IC to separate data from commmands*/
        incrementDataLabels (allLabelsList, IC);
        IC = errorFlag = 0;
        externList = createList();
        fprintf(stdout, "Parsing file: %s, second pass\n", completeFileName);
        /*return to the beginning of the file*/
        fseek(currFilePointer, 0, SEEK_SET);
        errorFlag = parseFileSecondPass(currFilePointer, &IC, commandsArray, allLabelsList, externList);
        if (errorFlag) {
            fprintf(stdout, "Errors found in second pass in file %s\n", completeFileName);
            fclose(currFilePointer);
            freeList(allLabelsList);
            freeList(externList);
            continue;
        }
        fprintf(stdout, "Generating output files\n");
        printObjectFile(argv[currArgv], commandsArray, dataArray, IC, DC);
        printExternalsFile(argv[currArgv], externList);
        printEntriesFile(argv[currArgv], allLabelsList);
        fclose(currFilePointer);
        freeList(allLabelsList);
        freeList(externList);
    }
    return 0;
} 