#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "symbolsTable.h"

/*Function to create new empty list. initialized list head and last to NULL and returns a pointer to the new list.*/
labelList* createList() {
    labelList *newList = (labelList*)malloc(sizeof(labelList));
    if (!newList) {
        fprintf(stderr, "Unable to allocate memory. Terminating.\n");
        exit(1);
    }
    else {
        newList -> head = NULL;
        newList -> last = NULL;
        return newList;
    }
}

/*Function to add a label to the end of a given labelList. Takes as input the label name and value, as well as whether it is a data label or a code
label, and whether it is a label marked as entry, as external or as realocatable. Initializes all label values to the ones provided*/
void addLabel(labelList *list, char* newName, int newVal, char newDataFlag, char newCodeFlag, char newExternFlag, char newEntryFlag, char newRealocFlag) {
    label *newLabel = (label*)malloc(sizeof(label));
    if (!newLabel) {
        fprintf(stderr, "Unable to allocate memory. Terminating.\n");
        exit(1);
    }
    else {
        strcpy(newLabel -> name, newName);
        newLabel -> value = newVal;
        newLabel -> dataFlag = newDataFlag;
        newLabel -> codeFlag = newCodeFlag;
        newLabel -> externFlag = newExternFlag;
        newLabel -> entryFlag = newEntryFlag;
        newLabel -> realocFlag = newRealocFlag;
        newLabel -> next = NULL;
        if (list -> head == NULL) { /*list is empty*/
            list -> head = newLabel;
            list -> last = newLabel;
        }
        else { /*list is not empty*/
            list -> last -> next = newLabel;
            list -> last = newLabel;
        }
    }
}

/*Function to free list memory. frees all list members, and list struct itself*/
void freeList (labelList *list) {
    label *temp, *current;
    int count = 0;
    current = list -> head;
    while (current != NULL) {
        temp = current;
        current = current -> next;
        free(temp);
        count++;
    }
    free(list);
}

/*Function to search list for a label with a specified name. returns 1 if found, 0 otherwise*/
int containsName (labelList *list, char *searchTerm) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, searchTerm) == 0) {
            found = 1;
        }
        current = current -> next;
    }
    return found;
}

/*function to add constant value to all labels marked as 'data'. goes over list element-by-element, if dataFlag is 1, 
adds the given constant value to the label's value*/
void incrementDataLabels (labelList *list, int incrementValue) {
    label *current = list -> head;
    while (current != NULL) {
        if (current -> dataFlag == 1) {
            current -> value += incrementValue;
        }
        current = current -> next;
    }
}

/*Function to set the entry flag of a given label in a labelList to 1. Returns 1 if found and updated, 0 if not found, -1 if found but declared as extern*/
int setEntryFlag (labelList *list, char labelName[MAX_TOKEN_LEN+1]) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, labelName) == 0) {
            found = 1;
            if (current -> externFlag == 1) {
                return -1;
            }
            current -> entryFlag = 1;
        }
        current = current -> next;
    }
    return found;
}

/*Function to retrieve the value of a label by its name, as well as whether it is defined as realocatable or external. 
Returns 0 if label not found in list and 1 otherwise. copy relevant values to target variables*/
int getLabelValueARE(labelList *list, char labelName[MAX_TOKEN_LEN+1], int *valueTarget, int *RFlagTarget, int *EFlagTarget) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && !found) {
        if (strcmp(current -> name, labelName) == 0) {
            found = 1;
            *valueTarget = current -> value;
            *RFlagTarget = current -> realocFlag;
            *EFlagTarget = current -> externFlag;
        }
        current = current -> next;
    }
    return found;
}

/*Function to print the names and values of all members of a given list to the given file. Returns the number of members printed*/
int printListToFile (FILE *fp, labelList *list) {
    int numPrinted;
    label *current = list -> head;
    numPrinted = 0;
    while (current != NULL) {
        fprintf(fp, "%s    %d\n", current -> name, current -> value);
        current = current -> next;
        numPrinted++;
    }
    return numPrinted;
}

/*Function that goes over the list member by member and if the label is defined as entry, print its name and value to the given file.
Returns the number of members printed*/
int printEntryLabelsToFile (FILE *fp, labelList *list) {
    int numPrinted;
    label *current = list -> head;
    numPrinted = 0;
    while (current!= NULL) {
        if (current -> entryFlag == 1) {
            fprintf(fp, "%s    %d\n", current -> name, current -> value);
            numPrinted++;
        }
        current = current -> next;
    }
    return numPrinted;
}