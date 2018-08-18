#ifndef SYMBOLS_TABLE_FILE_H
#define SYMBOLS_TABLE_FILE_H

typedef struct label {
    char name[MAX_TOKEN_LEN+1];
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

/*Function to create a new empty labelList*/
labelList* createList();
/*Function to add a new label to the end of an existing labelList. Initializes label name, value and flags according to the given parameters*/
void addLabel(labelList *list, char* newName, int newVal, char newDataFlag, char newCodeFlag, char newExternFlag, char newEntryFlag, char newRealocFlag);
/*Function to free memory allocated for a list. Frees all list members memory and the memory of the list itself*/
void freeList (labelList *list);
/*Function to search a label in a labelList by its name. Returns 1 if found, 0 otherwise*/
int containsName (labelList *list, char *searchTerm);
/*Function to add a constant number to the values of all list members marked as data labels*/
void incrementDataLabels (labelList *list, int incrementValue);
/*Function to set the entry flag of a given label in a labelList to 1. Returns 1 if found and updated, 0 if not found, -1 if found but declared as extern*/
int setEntryFlag (labelList *list, char labelName[MAX_TOKEN_LEN+1]);
/*Function to retrieve the value of a label by its name, as well as whether it is defined as realocatable or external.
Returns 0 if label not found in list and 1 otherwise.*/
int getLabelValueARE(labelList *list, char labelName[MAX_TOKEN_LEN+1], int *valueTarget, int *RFlagTarget, int *EFlagTarget);
/*Function to print list member names and values to a given file. Returns the number of members printed*/
int printListToFile (FILE *fp, labelList *list);
/*Function to go over the labelList and print all label names and values of labels marked as entry labels.
Returns the number of members printed*/
int printEntryLabelsToFile (FILE *fp, labelList *list);

#endif