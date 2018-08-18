#include "constants.h"

char registers[][MAX_TOKEN_LEN+1] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

char registersCodeValues[][MAX_REGISTER_LEN+1] = {"000000", "000001", "000010", "000011",
                                                  "000100", "000101", "000110", "000111"};

char instructions[][MAX_TOKEN_LEN+1] = {".data", ".string", ".entry", ".extern"};

char operations[][MAX_TOKEN_LEN+1] = {"mov", "cmp", "add", "sub", "not", "clr", "lea",
                                            "inc", "dec", "jmp", "bne", "red", "prn", "jsr",
                                            "rts", "stop"};

char opcodes[][MAX_OPCODE_LEN+1] = {"0000", "0001", "0010", "0011", "0100", "0101", 
                                    "0110", "0111", "1000", "1001", "1010", "1011",
                                    "1100", "1101", "1110", "1111"};

int numParameters[] = {2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0};

int allowedSrcAddressingTypes[][NUM_ADDRESSING_TYPES] = {{0,1,3},{0,1,3},{0,1,3},{0,1,3},{-1},{-1},{1}};
int allowedSrcAddressingTypesNum[] = {3,3,3,3,0,0,1};

int allowedDestAddressingTypes[][NUM_ADDRESSING_TYPES] = {{1,3},{0,1,3},{1,3},{1,3},{1,3},
                                                          {1,3},{1,3},{1,3},{1,3},{1,2,3},
                                                          {1,2,3},{1,3},{0,1,3},{1,2,3}};
int allowedDestAddressingTypesNum[] = {2,3,2,2,2,2,2,2,2,3,3,2,3,3};

const int totalRegisters = 8;
const int totalInstructions = 4;
const int totalOperations = 16;