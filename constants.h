#ifndef FILE_CONSTANTS_H
#define FILE_CONSTANTS_H

#define INPUT_FILE_EXTENSION ".as"
#define OUTPUT_OBJECT_FILE_EXTENSION ".ob"
#define OUTPUT_ENTRIES_FILE_EXTENSION ".ent"
#define OUTPUT_EXTERNALS_FILE_EXTENSION ".ext"
#define MAX_FILE_NAME 1024
#define MAX_LINE_LEN 81
#define MAX_TOKEN_LEN 30
#define MAX_OPCODE_LEN 4
#define MAX_REGISTER_LEN 6
#define ARE_LEN 2
#define PARAMETER_LEN 2
#define ADDRESSING_LEN 2
#define STRING_ENCLOSER '\"'
#define COMMENT_CHAR ';'
#define IM_ADDRESSING_CHAR '#'
#define MAX_WORD_LEN 14
#define ZERO_ASCII_VAL 48
#define MAX_PARAMETERS 200
#define MAX_ASCII_LEN 3
#define NUM_ADDRESSING_TYPES 4
#define EMPTY_COMMAND "00000000000000"
#define MEMORY_START_POS 100
#define MAX_COMMAND_ARRAY_LEN 200

extern char registers[][MAX_TOKEN_LEN+1];
extern char registersCodeValues[][MAX_REGISTER_LEN+1];
extern char instructions[][MAX_TOKEN_LEN+1];
extern char operations[][MAX_TOKEN_LEN+1];
extern char opcodes[][MAX_OPCODE_LEN+1];
extern int numParameters[];
extern int allowedSrcAddressingTypes[][NUM_ADDRESSING_TYPES];
extern int allowedSrcAddressingTypesNum[];
extern int allowedDestAddressingTypes[][NUM_ADDRESSING_TYPES];
extern int allowedDestAddressingTypesNum[];
extern const int totalRegisters;
extern const int totalInstructions;
extern const int totalOperations;

#endif