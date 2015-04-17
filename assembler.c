#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define LINE_LENGTH 100
#define WORD_SIZE 10
#define INST_SIZE 32

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
 */
void constructSymbolTable(FILE *code) {
   char line[LINE_LENGTH];

   printf("constructing symbol table\n");
   while (fgets(line, LINE_LENGTH, code)) {
      parseLineForSymbolTable(line); 
   }
   printf("done\n");
}

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
   } else if (!strcmp(word, "addiu")) { //I
      opFormat = 'I';
      *code |= 0x9 << 26;
   } else if (!strcmp(word, "sll")) { //R
      opFormat = 'R';
      *code |= 0;
   } else if (!strcmp(word, "srl")) { //R
      opFormat = 'R';
      *code |= 0x02;
   } else if (!strcmp(word, "sra")) { //R
      opFormat = 'R';
      *code |= 0x03;
   } else if (!strcmp(word, "sub")) { //R
      opFormat = 'R';
      *code |= 0x22;
   } else if (!strcmp(word, "sltu")) { //R START HERE
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

void printBinary(int code) {
   int c;

   for (c = 31; c >= 0; c--) {
      if ((code >> c) & 1)
         putchar('1');
      else
         putchar('0');
   }

   putchar('\n');
}

int getRegisterNumber(char *reg) {
   int num;

   if (!strcmp(reg, "$zero")) {
      num = 0;
   } else if (!strcmp(reg, "$at")) {
      num = 1;
   } else if (!strcmp(reg, "$v0")) {
      num = 2; 
   } else if (!strcmp(reg, "$v1")) {
      num = 3; 
   } else if (!strcmp(reg, "$a0")) {
      num = 4;
   } else if (!strcmp(reg, "$a1")) {
      num = 5;
   } else if (!strcmp(reg, "$a2")) {
      num = 6;
   } else if (!strcmp(reg, "$a3")) {
      num = 7;
   } else if (!strcmp(reg, "$t0")) {
      num = 8;
   } else if (!strcmp(reg, "$t1")) {
      num = 9;
   } else if (!strcmp(reg, "$t2")) {
      num = 10;
   } else if (!strcmp(reg, "$t3")) {
      num = 11;
   } else if (!strcmp(reg, "$t4")) {
      num = 12;
   } else if (!strcmp(reg, "$t5")) {
      num = 13;
   } else if (!strcmp(reg, "$t6")) {
      num = 14;
   } else if (!strcmp(reg, "$t7")) {
      num = 15;
   } else if (!strcmp(reg, "$s0")) {
      num = 16;
   } else if (!strcmp(reg, "$s1")) {
      num = 17;
   } else if (!strcmp(reg, "$s2")) {
      num = 18;
   } else if (!strcmp(reg, "$s3")) {
      num = 19;
   } else if (!strcmp(reg, "$s4")) {
      num = 20;
   } else if (!strcmp(reg, "$s5")) {
      num = 21;
   } else if (!strcmp(reg, "$s6")) {
      num = 22;
   } else if (!strcmp(reg, "$s7")) {
      num = 23;
   } else if (!strcmp(reg, "$t8")) {
      num = 24;
   } else if (!strcmp(reg, "$t9")) {
      num = 25;
   } else if (!strcmp(reg, "$k0")) {
      num = 26;
   } else if (!strcmp(reg, "$k1")) {
      num = 27;
   } else if (!strcmp(reg, "$gp")) {
      num = 28;
   } else if (!strcmp(reg, "$sp")) {
      num = 29;
   } else if (!strcmp(reg, "$fp")) {
      num = 30;
   } else if (!strcmp(reg, "$ra")) {
      num = 31;
   } else {
      num = -1;
   }

   return num;
}

/**
 * General parsing of line
 */
void parseLineGeneral(char *line) {
   const char *format = " ,";
   char *word;
   int code = 0, reg, instLoc = 0;
   char opFormat = 0;
   char buffer[INST_SIZE];

   word = strtok(line, format);
   while (word != NULL) {
      //Rest of line is comment
      if (word[0] == '#')
         break;

      if (opFormat == '\0') {
         opFormat = getInstruction(word, &code);
      } else if (opFormat == 'R') {
         reg = getRegisterNumber(word); 
         if (reg != -1) {
            if (instLoc == 0)
               code |= reg << 11; //rd
            else if (instLoc == 1)
               code |= reg << 21; //rs
            else if (instLoc == 2)
               code |= reg << 16; //rt
         }
         instLoc++;
      } else if (opFormat == 'I') {
         if (instLoc == 2) {
            code |= atoi(word); 
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
      
      word = strtok(NULL, format);
   }
   printBinary(code);
}

/**
 * Second pass - assemble the program
 */
void assemble(FILE *code) {
   char line[LINE_LENGTH];

   while (fgets(line, LINE_LENGTH, code)) {
      printf("parsing line general for %s\n", line);
      parseLineGeneral(line); 
   }
   printf("done assembling\n");
}

int main(int argc, char **argv) {
   FILE *code;

   code = fopen(argv[1], "r");
   constructSymbolTable(code);
   fclose(code);
   code = fopen(argv[1], "r");
   assemble(code);

   return 0;
}
