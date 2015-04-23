#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define NUM_LINES
#define LINE_LENGTH 100
#define WORD_SIZE 10
#define INST_SIZE 32
#define SYMBOL_TABLE_SIZE 500
#define INITIAL_PC 0x400024

typedef struct {
   char symbol[40];
   int loc;
} symbolEntry;

static symbolEntry symbolTable[SYMBOL_TABLE_SIZE];
static int *assembledLines;

int numSymbols = 0;

/**
 * Check beginning of each line for symbol
 */
int parseLineForSymbolTable(char *line, int numLines) {
   const char *format = " \t,\n";
   char *word, *end;
   int len;

   if ((int)strlen(line) == 0) {
      return 0;
   }
   
   word = strtok(line, format);
   if (word == NULL || strlen(word) == 0 || strchr(word, '#') != NULL) {
      return 0;
   }

   //Only need to check first word of line for symbol
   if ((end = strchr(word, ':')) != NULL) {
      *(end + 1) = '\0';
      //printf("adding symbol: %s\n", word);
      strcpy(symbolTable[numSymbols].symbol, word);
      symbolTable[numSymbols].symbol[strlen(word) - 1] = '\0';
      symbolTable[numSymbols].loc = (numLines) * 4 + INITIAL_PC;
      numSymbols++;
   }

   return 1;
}

/**
 * First Pass - check each line for a symbol
 * Return number of lines in file
 */
int constructSymbolTable(FILE *code) {
   char line[LINE_LENGTH];
   int numLines = 0;

   while (fgets(line, LINE_LENGTH, code)) {
      if (parseLineForSymbolTable(line,  numLines))
         numLines++;
   }
   
   return numLines;
}

/**
 * Set the instruction op/function code for each instruction, and return type.
 * The S type is actually the R type, but for a shift command as the format is different.
 */
char getInstruction(char *word, int *code) {
   int i;
   char opFormat = 0;

   //Make word lowercase for matching
   for (i = 0; word[i]; i++)
      word[i] = tolower(word[i]);

   //Begin matching
   if (!strcmp(word, "and")) { //R
      opFormat = 'R';
      *code |= 0x24;
   } else if (!strcmp(word, "or")) { //R
      opFormat = 'R';
      *code |= 0x25;
   } else if (!strcmp(word, "add")) { //R
      opFormat = 'R';
      *code |= 0x20;
   } else if (!strcmp(word, "addu")) { //R
      opFormat = 'R';
      *code |= 0x21;
   } else if (!strcmp(word, "addi")) { //I
      opFormat = 'I';
      *code |= 0x08 << 26;
   } else if (!strcmp(word, "addiu")) { //I
      opFormat = 'I';
      *code |= 0x09 << 26;
   } else if (!strcmp(word, "sll")) { //S = R
      opFormat = 'S';
      *code |= 0;
   } else if (!strcmp(word, "srl")) { //S = R
      opFormat = 'S';
      *code |= 0x02;
   } else if (!strcmp(word, "sra")) { //R = R
      opFormat = 'S';
      *code |= 0x03;
   } else if (!strcmp(word, "sub")) { //R
      opFormat = 'R';
      *code |= 0x22;
   } else if (!strcmp(word, "slt")) { //R
      opFormat = 'R';
      *code |= 0x2a; 
   } else if (!strcmp(word, "sltu")) { //R
      opFormat = 'R';
      *code |= 0x2b;
   } else if (!strcmp(word, "sltiu")) { //I
      opFormat = 'I';
      *code |= 0x0b << 26;
   } else if (!strcmp(word, "beq")) { //I
      opFormat = 'B';
      *code |= 0x04 << 26;
   } else if (!strcmp(word, "bne")) { //I
      opFormat = 'B';
      *code |= 0x05 << 26;
   } else if (!strcmp(word, "lw")) { //I
      opFormat = 'I';
      *code |= 0x23 << 26;
   } else if (!strcmp(word, "sw")) { //I
      opFormat = 'I';
      *code |= 0x2b << 26;
   } else if (!strcmp(word, "j")) { //J
      opFormat = 'J';
      *code |= 0x02 << 26;
   } else if (!strcmp(word, "jr")) { //R
      opFormat = 'U';
      *code |= 0x08;
   } else if (!strcmp(word, "jal")) { //J
      opFormat ='J';
      *code |= 0x03 << 26;
   } else {
      opFormat = '\0';
   }

   return opFormat;
}

/**
 * Trims comment from word, returns whether there was one
 */
int trimComment(char *word) {
   char *c = strchr(word, '#');

   if (c != NULL) {
      *c = '\0';
      return 1;
   }
   return 0;
}

/**
 * Parse word to get register number
 */
int getRegisterNumber(char *reg) {
   int num;

   if (reg[strlen(reg) - 1] == ')')
      reg[strlen(reg) - 1] = '\0';

   if (!strcmp(reg, "zero") || !strcmp(reg, "0")) {
      num = 0;
   } else if (!strcmp(reg, "at")) {
      num = 1;
   } else if (!strcmp(reg, "v0")) {
      num = 2; 
   } else if (!strcmp(reg, "v1")) {
      num = 3; 
   } else if (!strcmp(reg, "a0")) {
      num = 4;
   } else if (!strcmp(reg, "a1")) {
      num = 5;
   } else if (!strcmp(reg, "a2")) {
      num = 6;
   } else if (!strcmp(reg, "a3")) {
      num = 7;
   } else if (!strcmp(reg, "t0")) {
      num = 8;
   } else if (!strcmp(reg, "t1")) {
      num = 9;
   } else if (!strcmp(reg, "t2")) {
      num = 10;
   } else if (!strcmp(reg, "t3")) {
      num = 11;
   } else if (!strcmp(reg, "t4")) {
      num = 12;
   } else if (!strcmp(reg, "t5")) {
      num = 13;
   } else if (!strcmp(reg, "t6")) {
      num = 14;
   } else if (!strcmp(reg, "t7")) {
      num = 15;
   } else if (!strcmp(reg, "s0")) {
      num = 16;
   } else if (!strcmp(reg, "s1")) {
      num = 17;
   } else if (!strcmp(reg, "s2")) {
      num = 18;
   } else if (!strcmp(reg, "s3")) {
      num = 19;
   } else if (!strcmp(reg, "s4")) {
      num = 20;
   } else if (!strcmp(reg, "s5")) {
      num = 21;
   } else if (!strcmp(reg, "s6")) {
      num = 22;
   } else if (!strcmp(reg, "s7")) {
      num = 23;
   } else if (!strcmp(reg, "t8")) {
      num = 24;
   } else if (!strcmp(reg, "t9")) {
      num = 25;
   } else if (!strcmp(reg, "k0")) {
      num = 26;
   } else if (!strcmp(reg, "k1")) {
      num = 27;
   } else if (!strcmp(reg, "gp")) {
      num = 28;
   } else if (!strcmp(reg, "sp")) {
      num = 29;
   } else if (!strcmp(reg, "fp")) {
      num = 30;
   } else if (!strcmp(reg, "ra")) {
      num = 31;
   } else {
      num = -1;
   }

   return num;
}

/**
 * General parsing of line
 */
int parseLineGeneral(char *line, int curLine) {
   const char *format = " \t,\n$:";
   const char *formatReg = " \t\n()";
   char *word;
   char *immediate;
   char *regStr;
   int code = 0, reg, instLoc = 0, isComment;
   char opFormat = 0;
   char buffer[INST_SIZE];
   int i = 0;
   int setSymbol = 0;
   int jumpSymbol = 0;
   

   if (line == NULL || strlen(line) == 0)
      return 0;

   word = strtok(line, format);
   while (word != NULL) {
      isComment = trimComment(word);

      if (opFormat == '\0') {
         opFormat = getInstruction(word, &code);
      } else if (opFormat == 'R') {
         reg = getRegisterNumber(word); 
         if (reg != -1) {
            if (instLoc == 0) {
               code |= reg << 11; //rd
            } else if (instLoc == 1) {
               code |= reg << 21; //rs
            } else if (instLoc == 2) {
               code |= reg << 16; //rt
            }
         }
         instLoc++;
      } else if (opFormat == 'U') {
         reg = getRegisterNumber(word); 
         if (reg != -1) {
            if (instLoc == 0) {
               code |= reg << 21; //rd
            }
         }
         instLoc++;
      } else if (opFormat == 'S') {
         reg = getRegisterNumber(word); 
         if (instLoc == 2) {
            code |= (strtol(word, NULL, 10) & 0x1F) << 6; //shamt
         } else {
            if (reg != -1) {
               if (instLoc == 0) {
                  code |= reg << 11; //rd
               } else if (instLoc == 1) {
                  code |= reg << 16; //rt
               }
            }
         }
         instLoc++;
      } else if (opFormat == 'I') {
         for (i = 0; i < numSymbols; i++) {
            if (!strcmp(symbolTable[i].symbol, word)) {
               code |= ((symbolTable[i].loc - (curLine * 4 + INITIAL_PC )) / 4) & 0xFFFF;
               jumpSymbol = 1;
            }
         }
         if (instLoc == 2) {
            if (strstr(word, "0x") != NULL)
               code |= strtol(word, NULL, 16) & 0xFFFF;
            else
               code |= strtol(word, NULL, 10) & 0xFFFF; //and word so only 16 bytes are copied
         } else {
            reg = getRegisterNumber(word); 
            if (reg != -1) {
               if (instLoc == 0) {
                  code |= reg << 16;
               }
               else if (instLoc == 1) {
                  code |= reg << 21;
               }
            }
            else {
               code |= strtol(word, NULL, 10);
               regStr = strtok(NULL, format);
               if (regStr != NULL) {
                  reg = getRegisterNumber(regStr);
                  if (reg != -1) {
                     code |= reg << 21;
                  }
               }
            }
               
         }
         instLoc++;
      } else if (opFormat == 'B') {
         if (instLoc == 2) {
            for (i = 0; i < numSymbols; i++) {
               if (!strcmp(symbolTable[i].symbol, word)) {
                  code |= ((symbolTable[i].loc - (curLine * 4 + INITIAL_PC )) / 4) & 0xFFFF;
                  jumpSymbol = 1;
               }
            }
         } else {
            reg = getRegisterNumber(word); 
            if (reg != -1) {
               if (instLoc == 0) {
                  code |= reg << 21;
               }
               else if (instLoc == 1) {
                  code |= reg << 16;
               }
            }
            else {
               immediate = strtok(word, formatReg);
               if (immediate != NULL)
                  code |= strtol(immediate, NULL, 10);
               regStr = strtok(NULL, formatReg);
               if (regStr != NULL) {
                  reg = getRegisterNumber(regStr);
                  if (reg != -1) {
                     code |= reg << 21;
                  }
               }
            }
         }
         instLoc++;
      } else if (opFormat == 'J') {
         for (i = 0; i < numSymbols; i++) {
            if (!strcmp(symbolTable[i].symbol, word)) {
               code |= symbolTable[i].loc / 4;
               jumpSymbol = 1;
            }
         }
         if (jumpSymbol == 0) {
            reg = getRegisterNumber(word);
            if (reg != -1) {
               code |= (reg & 0x1F);
            }
            else if (strstr(word, "0x") != NULL && !setSymbol) {
               code |= (strtol(word, NULL, 16) & 0xFFFF) << 5;
            }
            else {
               code |= (strtol(word, NULL, 10) & 0xFFFF) << 5; //and word so only 16 bytes are copied
            }  
         }
         jumpSymbol = 0;
      }

      //Rest of line is comment
      //Put this at the end because there may be no space between previous word and comment
      if (isComment)
         break;

      
      word = strtok(NULL, format);
   }

   if (code) {
      assembledLines[curLine] = code;
      return 1;
   }

   return 0;
}

/**
 * Second pass - assemble the program
 */
int assemble(FILE *code) {
   char line[LINE_LENGTH];
   int curLine = 0;

   while (fgets(line, LINE_LENGTH, code)) {
      if (parseLineGeneral(line, curLine))
         curLine++;
   }
   return curLine;
}

void printAssembled(int numLines) {
   int i;

   for (i = 0; i < numLines; i++) {
      printf("%08X\n", assembledLines[i]);
   }
}

void printSymbolTable(int numLines) {
   int i;

   for( i = 0; i < numLines; i++) {
      if (strlen(symbolTable[i].symbol) != 0) {
         printf("Symbol: %s @ line: %d\n", symbolTable[i].symbol, symbolTable[i].loc);
      }
   }
}
int main(int argc, char **argv) {
   FILE *code;
   int numLines = 0;

   code = fopen(argv[1], "r");
   
   numLines = constructSymbolTable(code);
   assembledLines = calloc(sizeof(int), numLines);

   fclose(code);
   code = fopen(argv[1], "r");
   printAssembled(assemble(code));

   free(assembledLines);
   return 0;
}
