#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define LINE_LENGTH 100
#define WORD_SIZE 10

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

   printf("getting instructin for %s\n", word);

   //Make word lowercase for matching
   for (i = 0; word[i]; i++)
      word[i] = tolower(word[i]);

   //Begin matching
   if (!strcmp(word, "and")) { //R

   } else if (!strcmp(word, "or")) { //R
   
   } else if (!strcmp(word, "add")) { //R
   
   } else if (!strcmp(word, "addu")) { //R

   } else if (!strcmp(word, "addiu")) { //I

   } else if (!strcmp(word, "sll")) { //R

   } else if (!strcmp(word, "srl")) { //R

   } else if (!strcmp(word, "sra")) { //R

   } else if (!strcmp(word, "sub")) { //R

   } else if (!strcmp(word, "sltu")) { //R START HERE
      opFormat = 'R';
      *code |= 0x2b;
      printf("%x\n", code);

   } else if (!strcmp(word, "sltiu")) { //I

   } else if (!strcmp(word, "beq")) { //I

   } else if (!strcmp(word, "bne")) { //I

   } else if (!strcmp(word, "lw")) { //I

   } else if (!strcmp(word, "sw")) { //I

   } else if (!strcmp(word, "j")) { //J

   } else if (!strcmp(word, "jr")) { //R

   } else if (!strcmp(word, "jal")) { //J

   } else {

   }
   return opFormat;
}

/**
 * General parsing of line
 */
void parseLineGeneral(char *line) {
   const char *format = " ,";
   char *word;
   int instFound = 0;
   int code = 0;
   char opFormat = 0;

   word = strtok(line, format);
   printf("gen: %s\n", word);
   while (word != NULL) {
      //Rest of line is comment
      if (word[0] == '#')
         break;

      if (opFormat == 0) {
         opFormat = getInstruction(word, &code);
      }
      
      word = strtok(NULL, format);
   }
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
