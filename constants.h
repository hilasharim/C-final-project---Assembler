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

/*array of reserved register names r0-r7*/
extern char registers[][MAX_TOKEN_LEN+1];
/*array of binary code values of each register - the binary translation of its number*/
extern char registersCodeValues[][MAX_REGISTER_LEN+1];
/*array of reserved instruction names*/
extern char instructions[][MAX_TOKEN_LEN+1];
/*array of reserved operation names*/
extern char operations[][MAX_TOKEN_LEN+1];
/*array of binary code values of each operation that appears in the operations array*/
extern char opcodes[][MAX_OPCODE_LEN+1];
/*array of the number of operands required for each operator that appears in the operations array*/
extern int numParameters[];
/*array of the allowed addressing method of the source operand for each operator that appears in the operations array, if required*/
extern int allowedSrcAddressingTypes[][NUM_ADDRESSING_TYPES];
/*array of the number of allowed addressing method of the source operand for each operator that appears in the operations array, if required*/
extern int allowedSrcAddressingTypesNum[];
/*array of the allowed addressing method of the destination operand for each operator that appears in the operations array, if required*/
extern int allowedDestAddressingTypes[][NUM_ADDRESSING_TYPES];
/*array of the number of allowed addressing method of the destination operand for each operator that appears in the operations array, if required*/
extern int allowedDestAddressingTypesNum[];
/*total number of registers*/
extern const int totalRegisters;
/*total number of instructions*/
extern const int totalInstructions;
/*total number of operators*/
extern const int totalOperations;

#endif