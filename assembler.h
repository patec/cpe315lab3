#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "simulator.h"

#define LINE_LENGTH 100
#define WORD_SIZE 10
#define INST_SIZE 32
#define SYMBOL_TABLE_SIZE 500
#define INITIAL_PC 0
#define PROG_SIZE 0x800000

typedef struct {
   char symbol[40];
   int loc;
} symbolEntry;

typedef struct {
   int inst;
   int type;
} line;

static symbolEntry symbolTable[SYMBOL_TABLE_SIZE];
static line assembledLines[PROG_SIZE];

static int numSymbols = 0;

int parseLineForSymbolTable(char *line, int numLines);

int constructSymbolTable(FILE *code);

char getInstruction(char *word, int *code);

int trimComment(char *word);

int getRegisterNumber(char *reg);

int parseLineGeneral(char *line, int curLine);

int assemble(FILE *code);

void printAssembled(int numLines);

void printSymbolTable(int numLines);

#endif
