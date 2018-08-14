#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABEL_LEN 30

typedef struct label {
    char name[MAX_LABEL_LEN+1];
    int value;
    char dataFlag;
    char codeFlag;
    char externFlag;
    char entryFlag;
    char realocFlag;
    struct label *next;
} label;

typedef struct labelList {
    label *head;
    label *last;
} labelList;

/*function to create new empty list*/
labelList* createList() {
    labelList *newList = (labelList*)malloc(sizeof(labelList));
    if (!newList) {
        fprintf(stderr, "Unable to allocate memory. Terminaating.");
        exit(1);
    }
    else {
        newList -> head = NULL;
        newList -> last = NULL;
        return newList;
    }
}

void addLabel(labelList *list, char* newName, int newVal, char newDataFlag, char newCodeFlag, char newExternFlag, char newEntryFlag, char newRealocFlag) {
    label *newLabel = (label*)malloc(sizeof(label));
    if (!newLabel) {
        fprintf(stderr, "Unable to allocate memory. Terminaating.");
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
            list -> last = newLabel
        }
        else { /*list is not empty*/
            list -> last -> next = newLabel;
            list -> last = newLabel;
        }
    }
}

/*function to free list memory. frees all list members, and list struct itself*/
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
    return count;
}

/*function to print list for debugging*/
void printList (labelList *list) {
    label *current = list -> head;
    while (current != NULL) {
        printf("%s\n", current -> name);
        current = current -> next;
    }
}

/*function to search list for a label with a specified name. returns 1 if found, 0 otherwise*/
int containsName (labelList *list, char *searchTerm) {
    int found = 0;
    label *current = list -> head;
    while (current != NULL && found == 0) {
        if (strcmp(current -> name, searchTerm) == 0) {
            found = 1;
        }
        current = current -> next;
    }
    return found;
}

/*function to add constant value to all labels marked as 'data'. goes over list element-by-element,
if dataFlag is 1, adds the given constant value to the label's value*/
void incrementDataLabels (labelList *list, int incrementValue) {
    label *current = list -> head;
    while (current != NULL) {
        if (current -> dataFlag == 1) {
            current -> value += incrementValue;
        }
        current = current -> next;
    }
}