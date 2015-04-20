#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define NUM_LINES
#define LINE_LENGTH 100
#define WORD_SIZE 10
#define INST_SIZE 32


static int *assembledLines;
/**
 * Check beginning of each line for symbol
 */
void parseLineForSymbolTable(char *line) {
   const char *format = " ,";
   char *word;

   word = strtok(line, format);
   //Only need to check first word of line for symbol
   if (word[strlen(word) - 1] == ':') {
      // Add to symbol table
   }
}

/**
 * First Pass - check each line for a symbol
 * Return number of lines in file
 */
int constructSymbolTable(FILE *code) {
   char line[LINE_LENGTH];
   int numLines = 0;

   while (fgets(line, LINE_LENGTH, code)) {
      parseLineForSymbolTable(line); 
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
   } else if (!strcmp(word, "sltu")) { //R
      opFormat = 'R';
      *code |= 0x2b;
      printf("%x\n", *code);

   } else if (!strcmp(word, "sltiu")) { //I

   } else if (!strcmp(word, "beq")) { //I

   } else if (!strcmp(word, "bne")) { //I

   } else if (!strcmp(word, "lw")) { //I

   } else if (!strcmp(word, "sw")) { //I

   } else if (!strcmp(word, "j")) { //J

   } else if (!strcmp(word, "jr")) { //R

   } else if (!strcmp(word, "jal")) { //J

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

   if (!strcmp(reg, "$zero") || !strcmp(reg, "$0")) {
      num = 0;
   } else if (!strcmp(reg, "$at") || !strcmp(reg, "$1")) {
      num = 1;
   } else if (!strcmp(reg, "$v0") || !strcmp(reg, "$2")) {
      num = 2; 
   } else if (!strcmp(reg, "$v1") || !strcmp(reg, "$3")) {
      num = 3; 
   } else if (!strcmp(reg, "$a0") || !strcmp(reg, "$4")) {
      num = 4;
   } else if (!strcmp(reg, "$a1") || !strcmp(reg, "$5")) {
      num = 5;
   } else if (!strcmp(reg, "$a2") || !strcmp(reg, "$6")) {
      num = 6;
   } else if (!strcmp(reg, "$a3") || !strcmp(reg, "$7")) {
      num = 7;
   } else if (!strcmp(reg, "$t0") || !strcmp(reg, "$8")) {
      num = 8;
   } else if (!strcmp(reg, "$t1") || !strcmp(reg, "$9")) {
      num = 9;
   } else if (!strcmp(reg, "$t2") || !strcmp(reg, "$10")) {
      num = 10;
   } else if (!strcmp(reg, "$t3") || !strcmp(reg, "$11")) {
      num = 11;
   } else if (!strcmp(reg, "$t4") || !strcmp(reg, "$12")) {
      num = 12;
   } else if (!strcmp(reg, "$t5") || !strcmp(reg, "$13")) {
      num = 13;
   } else if (!strcmp(reg, "$t6") || !strcmp(reg, "$14")) {
      num = 14;
   } else if (!strcmp(reg, "$t7") || !strcmp(reg, "$15")) {
      num = 15;
   } else if (!strcmp(reg, "$s0") || !strcmp(reg, "$16")) {
      num = 16;
   } else if (!strcmp(reg, "$s1") || !strcmp(reg, "$17")) {
      num = 17;
   } else if (!strcmp(reg, "$s2") || !strcmp(reg, "$18")) {
      num = 18;
   } else if (!strcmp(reg, "$s3") || !strcmp(reg, "$19")) {
      num = 19;
   } else if (!strcmp(reg, "$s4") || !strcmp(reg, "$20")) {
      num = 20;
   } else if (!strcmp(reg, "$s5") || !strcmp(reg, "$21")) {
      num = 21;
   } else if (!strcmp(reg, "$s6") || !strcmp(reg, "$22")) {
      num = 22;
   } else if (!strcmp(reg, "$s7") || !strcmp(reg, "$23")) {
      num = 23;
   } else if (!strcmp(reg, "$t8") || !strcmp(reg, "$24")) {
      num = 24;
   } else if (!strcmp(reg, "$t9") || !strcmp(reg, "$25")) {
      num = 25;
   } else if (!strcmp(reg, "$k0") || !strcmp(reg, "$26")) {
      num = 26;
   } else if (!strcmp(reg, "$k1") || !strcmp(reg, "$27")) {
      num = 27;
   } else if (!strcmp(reg, "$gp") || !strcmp(reg, "$28")) {
      num = 28;
   } else if (!strcmp(reg, "$sp") || !strcmp(reg, "$29")) {
      num = 29;
   } else if (!strcmp(reg, "$fp") || !strcmp(reg, "$30")) {
      num = 30;
   } else if (!strcmp(reg, "$ra") || !strcmp(reg, "$31")) {
      num = 31;
   } else {
      num = -1;
   }

   return num;
}

/**
 * General parsing of line
 */
void parseLineGeneral(char *line, int curLine) {
   const char *format = " \t,";
   char *word;
   int code = 0, reg, instLoc = 0, isComment;
   char opFormat = 0;
   char buffer[INST_SIZE];

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
      } else if (opFormat == 'S') {
         reg = getRegisterNumber(word); 
         if (reg != -1) {
            if (instLoc == 0) {
               code |= reg << 11; //rd
            } else if (instLoc == 1) {
               code |= reg << 16; //rt
            } else if (instLoc == 2) {
               code |= reg << 6; //shamt
            }
         }
         instLoc++;
      } else if (opFormat == 'I') {
         if (instLoc == 2) {
            if (strstr(word, "0x") != NULL)
               code |= strtol(word, NULL, 16) & 0xFFFF;
            else
               code |= strtol(word, NULL, 10) & 0xFFFF; //and word so only 16 bytes are copied
         } else {
            reg = getRegisterNumber(word); 
            if (reg != -1) {
               if (instLoc == 0)
                  code |= reg << 16;
               else if (instLoc == 1)
                  code |= reg << 25;
            }
         }
         instLoc++;
      } else if (opFormat == 'J') {

      }

      //Rest of line is comment
      //Put this at the end because there may be no space between previous word and comment
      if (isComment)
         break;

      
      word = strtok(NULL, format);
   }

   assembledLines[curLine] = code;
}

/**
 * Second pass - assemble the program
 */
void assemble(FILE *code) {
   char line[LINE_LENGTH];
   int curLine = 0;

   while (fgets(line, LINE_LENGTH, code)) {
      parseLineGeneral(line, curLine); 
      curLine++;
   }
}

void printAssembled(int numLines) {
   int i;

   for (i = 0; i < numLines; i++)
      printf("%08X\n", assembledLines[i]);
}

int main(int argc, char **argv) {
   FILE *code;
   int numLines;

   code = fopen(argv[1], "r");
   numLines = constructSymbolTable(code);
   assembledLines = calloc(sizeof(int), numLines);

   fclose(code);
   code = fopen(argv[1], "r");
   assemble(code);

   printAssembled(numLines);

   return 0;
}
