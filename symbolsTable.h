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

labelList* createList();
void addLabel(labelList *list, char* newName, int newVal, char newDataFlag, char newCodeFlag, char newExternFlag, char newEntryFlag, char newRealocFlag);
void freeList (labelList *list);
int containsName (labelList *list, char *searchTerm);
void incrementDataLabels (labelList *list, int incrementValue);
int setEntryFlag (labelList *list, char labelName[MAX_TOKEN_LEN+1]);
int getLabelValueARE(labelList *list, char labelName[MAX_TOKEN_LEN+1], int *valueTarget, int *RFlagTarget, int *EFlagTarget);
int printListToFile (FILE *fp, labelList *list);
int printEntryLabelsToFile (FILE *fp, labelList *list);
void printList (labelList *list);

#endif