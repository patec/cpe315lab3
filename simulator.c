#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "simulator.h"

#define LINE_LENGTH 100
#define WORD_SIZE 10
#define INST_SIZE 32
#define NUM_REGISTERS 32
#define SYMBOL_TABLE_SIZE 500
#define INITIAL_PC 0
#define PROG_SIZE 0x80000000
#define PROG_START 0x04000000

typedef struct {
   char symbol[40];
   int loc;
} symbolEntry;

typedef struct {
   int inst;
   int type;
} line;

typedef struct {
   line inst;
   int pc;
   int rs;
   int rt;
   int rd;
   int shamt;
   int imm;
   int opCode;
   int funcCode;
   int aluOut;
   int flush;
   int busy;
   // LOGIC TO WRITE BACK
} status;

static symbolEntry symbolTable[SYMBOL_TABLE_SIZE];
static line assembledLines[1000]; //TODO: hack, changed from PROG_SIZE
static int registers[NUM_REGISTERS];

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
      *code |= AND_CODE;
   } else if (!strcmp(word, "or")) { //R
      opFormat = 'R';
      *code |= OR_CODE;
   } else if (!strcmp(word, "ori")) { //I
      opFormat = 'I';
      *code |= ORI_CODE;
   } else if (!strcmp(word, "add")) { //R
      opFormat = 'R';
      *code |= ADD_CODE;
   } else if (!strcmp(word, "addu")) { //R
      opFormat = 'R';
      *code |= ADDU_CODE;
   } else if (!strcmp(word, "addi")) { //I
      opFormat = 'I';
      *code |= ADDI_CODE;
   } else if (!strcmp(word, "addiu")) { //I
      opFormat = 'I';
      *code |= ADDIU_CODE;
   } else if (!strcmp(word, "sll")) { //S = R
      opFormat = 'S';
      *code |= SLL_CODE;
   } else if (!strcmp(word, "srl")) { //S = R
      opFormat = 'S';
      *code |= SRL_CODE;
   } else if (!strcmp(word, "sra")) { //S = R
      opFormat = 'S';
      *code |= SRA_CODE;
   } else if (!strcmp(word, "sub")) { //R
      opFormat = 'R';
      *code |= SUB_CODE;
   } else if (!strcmp(word, "slt")) { //R
      opFormat = 'R';
      *code |= SLT_CODE; 
   } else if (!strcmp(word, "slti")) { //I
      opFormat = 'I';
      *code |= SLTIU_CODE;
   } else if (!strcmp(word, "sltu")) { //R
      opFormat = 'R';
      *code |= SLTU_CODE;
   } else if (!strcmp(word, "sltiu")) { //I
      opFormat = 'I';
      *code |= SLTIU_CODE;
   } else if (!strcmp(word, "beq")) { //I
      opFormat = 'B';
      *code |= BEQ_CODE;
   } else if (!strcmp(word, "bne")) { //I
      opFormat = 'B';
      *code |= BNE_CODE;
   } else if (!strcmp(word, "lui")) { //I
      opFormat = 'I';
      *code |= LUI_CODE;
   } else if (!strcmp(word, "lw")) { //I
      opFormat = 'I';
      *code |= LW_CODE;
   } else if (!strcmp(word, "sw")) { //I
      opFormat = 'I';
      *code |= SW_CODE;
   } else if (!strcmp(word, "j")) { //J
      opFormat = 'J';
      *code |= J_CODE;
   } else if (!strcmp(word, "jr")) { //R
      opFormat = 'U';
      *code |= JR_CODE;
   } else if (!strcmp(word, "jal")) { //J
      opFormat ='J';
      *code |= JAL_CODE;
   } else if (!strcmp(word, "syscall")) {
      opFormat = 'T';
      *code |= SYSCALL_CODE;
   } else if (!strcmp(word, ".word")) {
      opFormat = 'W';
   } else if (!strcmp(word, ".byte")) {
      opFormat = 'D';
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
   int curByte = 0;
   

   if (line == NULL || strlen(line) == 0)
      return 0;

   word = strtok(line, format);
   while (word != NULL) {
      isComment = trimComment(word);

      if (opFormat == '\0') { 
         opFormat = getInstruction(word, &code);
         assembledLines[curLine].type = code;
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
         if (instLoc == 2 || strstr(word, "0x")) {
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
      } else if (opFormat == 'T') {
            break;
      } else if (opFormat == 'W') {
         assembledLines[curLine].type = -1;
         assembledLines[curLine++].inst = (int) word;
      } else if (opFormat == 'D') {
         assembledLines[curLine << (curByte % 32)].type = -1;
         assembledLines[curLine << (curByte % 32)].inst = (char) word;
         if (curByte % 32 == 0) {
            curLine++; 
         }
      }

      //Rest of line is comment
      //Put this at the end because there may be no space between previous word and comment
      if (isComment)
         break;

      
      word = strtok(NULL, format);
   }

   if (code) {
      assembledLines[curLine].inst = code;
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
      curLine += parseLineGeneral(line, curLine);
   }

   return curLine;
}

void printAssembled(int numLines) {
   int i;

   for (i = 0; i < numLines; i++) {
      printf("%08X\n", assembledLines[i].inst);
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

void initRegisters() {
   int i;

   for (i = 0; i < NUM_REGISTERS; i++) {
      registers[i] = 0; 
   }

   registers[28] = PROG_SIZE / 4;
   registers[29] = PROG_SIZE - 8;
   registers[31] = INITIAL_PC;
}

int runCommand(line *inst, int *memRefs, int *clockCycles, int lineNum) {
   int rs, rt, rd, imm, shamt, address, pc = lineNum * 4 + INITIAL_PC, oldPc;

   printf("%08X\n", inst->inst);
   if (inst->type == AND_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = registers[rs] & registers[rt];
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == OR_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = registers[rs] | registers[rt];
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type ==  ORI_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = registers[rs] | (short) imm;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == ADD_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = registers[rs] + registers[rt];
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type ==  ADDU_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = (unsigned) registers[rs] + (unsigned) registers[rt];
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == ADDI_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = registers[rs] + (short) imm;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == ADDIU_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = (unsigned) registers[rs] & (unsigned short) imm;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == SLL_CODE) {
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      shamt = (inst->inst >> 6) & 0x1F;
      registers[rd] = registers[rt] << shamt;
      pc += 4;
      *clockCycles += shamt + 5;
   } else if (inst->type == SRL_CODE) {
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      shamt = (inst->inst >> 6) & 0x1F;
      registers[rd] = registers[rt] >> shamt;
      pc += 4;
      *clockCycles += shamt + 5;
   } else if (inst->type == SRA_CODE) {
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      shamt = (inst->inst >> 6) & 0x1F;
      registers[rd] = (unsigned) registers[rt] >> shamt;
      pc += 4;
       *clockCycles += shamt + 5;
   } else if (inst->type == SUB_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = registers[rs] - registers[rt];
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == SLT_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = registers[rs] < registers[rt] ? 1 : 0;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == SLTI_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = registers[rs] < imm ? 1 : 0;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == SLTU_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      rd = (inst->inst >> 11) & 0x1F;
      registers[rd] = (unsigned) registers[rs] < (unsigned) registers[rt] ? 1 : 0;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == SLTIU_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = (unsigned) registers[rs] < (unsigned) imm ? 1 : 0;
      pc += 4;
      *clockCycles += 4;
   } else if (inst->type == BEQ_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      address = (inst->inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[rs] == registers[rt]) {
         pc += address;
      } else {
         pc += 4; 
      }
      *clockCycles += 3;
   } else if (inst->type == BNE_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      address = (inst->inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[rs] != registers[rt]) {
         pc += address;
      } else {
         pc += 4;
      }
      *clockCycles += 3;
   } else if (inst->type == LUI_CODE) {
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = (imm << 16) & 0xFFFF0000;
      pc += 4;
      *clockCycles += 4;
      *memRefs += 1;
   } else if (inst->type == LW_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      registers[rt] = assembledLines[registers[rs] + imm].inst;
      pc += 4;
      *clockCycles += 5;
      *memRefs = 1;
   } else if (inst->type == SW_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      rt = (inst->inst >> 16) & 0x1F;
      imm = inst->inst & 0xFFFF;
      assembledLines[registers[rs] + imm].inst = registers[rt];
      pc += 4;
      *clockCycles += 4;
      *memRefs += 1;
   } else if (inst->type == J_CODE) {
      pc = (inst->inst & 0x1FFFFFF) * 4;
      *clockCycles += 3;
   } else if (inst->type == JR_CODE) {
      rs = (inst->inst >> 21) & 0x1F;
      oldPc = pc;
      pc = registers[rs] - 4; 
      registers[31] = oldPc - 4;
      *clockCycles += 3;
   } else if (inst->type == JAL_CODE) {
      registers[31] = pc + 8; 
      pc = (inst->inst & 0x1FFFFFF) * 4;
      *clockCycles += 3;
   } else if (inst->type == SYSCALL_CODE) {
      if (registers[2]  == 10)
        return -1; 
   } else {
      pc += 4;
   }

   return (pc - INITIAL_PC) / 4;
}

void initStatus(status *s) {
   s->pc = 0;
   s->rs = -1;
   s->rs = -1;
   s->rd = -1;
   s->imm = 0;
   s->opCode = -1;
   s->funcCode = -1;
   s->aluOut = 0;
   s->flush = 0;
   s->busy = 0;
}

status instructionFetch(int i) {
   status s;
   initStatus(&s);
   
   s.pc += 4;
   s.inst = assembledLines[i];
   s.busy = 1;
   
   return s;
}

status instructionDecode(status s) {
   int lineNum = 0;
   int rs, rt, rd, imm, shamt, address, pc = lineNum * 4 + INITIAL_PC, oldPc;

   printf("%08X\n", s.inst.inst);
   if (s.inst.type == AND_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.type >> 11) & 0x1F;
      //registers[rd] = registers[rs] & registers[rt];
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == OR_CODE) {
      s.rs = (s.inst.type >> 21) & 0x1F;
      s.rt = (s.inst.type >> 16) & 0x1F;
      s.rd = (s.inst.type >> 11) & 0x1F;
      //registers[rd] = registers[rs] | registers[rt];
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type ==  ORI_CODE) {
      s.rs = (s.inst.type >> 21) & 0x1F;
      s.rt = (s.inst.type >> 16) & 0x1F;
      s.imm = s.inst.type & 0xFFFF;
      //registers[rt] = registers[rs] | (short) imm;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == ADD_CODE) {
      s.rs = (s.inst.type >> 21) & 0x1F;
      s.rt = (s.inst.type >> 16) & 0x1F;
      s.rd = (s.inst.type >> 11) & 0x1F;
      //registers[rd] = registers[rs] + registers[rt];
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type ==  ADDU_CODE) {
      s.rs = (s.inst.type >> 21) & 0x1F;
      s.rt = (s.inst.type >> 16) & 0x1F;
      s.rd = (s.inst.type >> 11) & 0x1F;
      //registers[rd] = (unsigned) registers[rs] + (unsigned) registers[rt];
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == ADDI_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      //registers[rt] = registers[rs] + (short) imm;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == ADDIU_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      //registers[rt] = (unsigned) registers[rs] & (unsigned short) imm;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == SLL_CODE) {
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      s.shamt = (s.inst.inst >> 6) & 0x1F;
      //registers[rd] = registers[rt] << shamt;
      //pc += 4;
      //*clockCycles += shamt + 5;
   } else if (s.inst.type == SRL_CODE) {
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      s.shamt = (s.inst.inst >> 6) & 0x1F;
      //registers[rd] = registers[rt] >> shamt;
      //pc += 4;
      //*clockCycles += shamt + 5;
   } else if (s.inst.type == SRA_CODE) {
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      s.shamt = (s.inst.inst >> 6) & 0x1F;
      //registers[rd] = (unsigned) registers[rt] >> shamt;
      //pc += 4;
      //*clockCycles += shamt + 5;
   } else if (s.inst.type == SUB_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      //registers[rd] = registers[rs] - registers[rt];
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == SLT_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      //registers[rd] = registers[rs] < registers[rt] ? 1 : 0;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == SLTI_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      // registers[rt] = registers[rs] < imm ? 1 : 0;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == SLTU_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.rd = (s.inst.inst >> 11) & 0x1F;
      //registers[rd] = (unsigned) registers[rs] < (unsigned) registers[rt] ? 1 : 0;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == SLTIU_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      //registers[rt] = (unsigned) registers[rs] < (unsigned) imm ? 1 : 0;
      //pc += 4;
      //*clockCycles += 4;
   } else if (s.inst.type == BEQ_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      /*address = (inst->inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[rs] == registers[rt]) {
         pc += address;
      } else {
         pc += 4; 
      }
      *clockCycles += 3;*/
   } else if (s.inst.type == BNE_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      /*address = (inst->inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[rs] != registers[rt]) {
         pc += address;
      } else {
         pc += 4;
      }
      *clockCycles += 3;*/
   } else if (s.inst.type == LUI_CODE) {
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      /*registers[rt] = (imm << 16) & 0xFFFF0000;
      pc += 4;
      *clockCycles += 4;
      *memRefs += 1;*/
   } else if (s.inst.type == LW_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      /*registers[rt] = assembledLines[registers[rs] + imm].inst;
      pc += 4;
      *clockCycles += 5;
      *memRefs = 1;*/
   } else if (s.inst.type == SW_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      s.rt = (s.inst.inst >> 16) & 0x1F;
      s.imm = s.inst.inst & 0xFFFF;
      /*assembledLines[registers[rs] + imm].inst = registers[rt];
      pc += 4;
      *clockCycles += 4;
      *memRefs += 1;*/
   } else if (s.inst.type == J_CODE) {
      s.pc = (s.inst.inst & 0x1FFFFFF) * 4;
      //*clockCycles += 3;
   } else if (s.inst.type == JR_CODE) {
      s.rs = (s.inst.inst >> 21) & 0x1F;
      oldPc = pc;
      pc = registers[rs] - 4; 
      //registers[31] = oldPc - 4;
      //*clockCycles += 3;
   } else if (s.inst.type == JAL_CODE) {
      //registers[31] = pc + 8; 
      //pc = (inst->inst & 0x1FFFFFF) * 4;
      //*clockCycles += 3;
   } else if (s.inst.type == SYSCALL_CODE) {
      if (registers[2]  == 10)
         s.pc = -1;
        //return -1; 
   } else {
      pc += 4;
   }

   //return (pc - INITIAL_PC) / 4;
   
   
   s.busy = 1;
   
   return s;
}

status execute(status s) {
   int lineNum = 0;
   int cc;
   int *clockCycles = &cc;
   int rs, rt, rd, imm, shamt, address, pc = lineNum * 4 + INITIAL_PC, oldPc;

   printf("%08X\n", s.inst.inst);
   if (s.inst.type == AND_CODE) {
      registers[rd] = registers[rs] & registers[rt];
   } else if (s.inst.type == OR_CODE) {
      registers[rd] = registers[rs] | registers[rt];
   } else if (s.inst.type ==  ORI_CODE) {
      registers[rt] = registers[rs] | (short) imm;
   } else if (s.inst.type == ADD_CODE) {
      registers[rd] = registers[rs] + registers[rt];
   } else if (s.inst.type ==  ADDU_CODE) {
      registers[rd] = (unsigned) registers[rs] + (unsigned) registers[rt];
   } else if (s.inst.type == ADDI_CODE) {
      registers[rt] = registers[rs] + (short) imm;
   } else if (s.inst.type == ADDIU_CODE) {
      registers[rt] = (unsigned) registers[rs] & (unsigned short) imm;
   } else if (s.inst.type == SLL_CODE) {
      registers[rd] = registers[rt] << shamt;
      *clockCycles += shamt + 5;
   } else if (s.inst.type == SRL_CODE) {
      registers[rd] = registers[rt] >> shamt;
      *clockCycles += shamt + 5;
   } else if (s.inst.type == SRA_CODE) {
      registers[rd] = (unsigned) registers[rt] >> shamt;
       *clockCycles += shamt + 5;
   } else if (s.inst.type == SUB_CODE) {
      registers[rd] = registers[rs] - registers[rt];
   } else if (s.inst.type == SLT_CODE) {
      registers[rd] = registers[rs] < registers[rt] ? 1 : 0;
   } else if (s.inst.type == SLTI_CODE) {
      registers[rt] = registers[rs] < imm ? 1 : 0;
   } else if (s.inst.type == SLTU_CODE) {
      registers[rd] = (unsigned) registers[rs] < (unsigned) registers[rt] ? 1 : 0;
   } else if (s.inst.type == SLTIU_CODE) {
      registers[rt] = (unsigned) registers[rs] < (unsigned) imm ? 1 : 0;
   } else if (s.inst.type == BEQ_CODE) {
      address = (s.inst.inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[rs] == registers[rt]) {
         s.pc += address;
         s.flush = 1;
      } else {
         s.pc += 4; 
      }
   } else if (s.inst.type == BNE_CODE) {
      address = (s.inst.inst & 0xFFFF);
      if (address & 0x8000)
         address += 0xFFFF0000;
      address = address * 4;
      if (registers[s.rs] != registers[s.rt]) {
         s.pc += address;
         s.flush = 1;
      } else {
         pc += 4;
      }
   } else if (s.inst.type == LUI_CODE) {
      registers[s.rt] = (s.imm << 16) & 0xFFFF0000;
      //*memRefs += 1;
   } else if (s.inst.type == LW_CODE) {
      registers[s.rt] = assembledLines[registers[s.rs] + s.imm].inst;
      //*memRefs = 1;
   } else if (s.inst.type == SW_CODE) {
      assembledLines[registers[s.rs] + s.imm].inst = registers[s.rt];
      //*memRefs += 1;
   } else if (s.inst.type == J_CODE) {
      s.pc = (s.inst.inst & 0x1FFFFFF) * 4;
   } else if (s.inst.type == JR_CODE) {
      oldPc = s.pc;
      s.pc = registers[s.rs] - 4; 
      registers[31] = oldPc - 4;
   } else if (s.inst.type == JAL_CODE) {
      registers[31] = s.pc + 8; 
      s.pc = (s.inst.inst & 0x1FFFFFF) * 4;
   } else if (s.inst.type == SYSCALL_CODE) {
      if (registers[2]  == 10)
        s.pc -1; 
   } else {
      pc += 4;
   }

   //return (pc - INITIAL_PC) / 4;
   s.busy = 1;
   
   return s;
}

status memoryAccess(status s) {
   //skip if no memory access
   s.busy = 1;
   
   return s;
}

status writeBack(status s) {
   // write back to registers
   s.busy = 1;
   
   return s;
}
   
void runProgramPipeline(int numLines) {
   char cmd;
   int i = 0, j, memRefs = 0, clockCycles = 0, instExec = 0, totClock = 0;
   status fetchReturn, decodeReturn, executeReturn, memReturn, wbReturn;
   

   initRegisters();

   while (i < numLines && i >= 0) {
      printf("Enter command (s for single step, r for run, q for quit): ");
      scanf(" %c", &cmd);

      if (cmd == 's') {
         clockCycles = 0;
         memRefs = 0; //TODO: add totalMemRefs?
         i = runCommand(&assembledLines[i], &memRefs, &clockCycles, i);
         instExec++;
         totClock += clockCycles;

         printf("Instructions executed (step): %d\n", 1);
         printf("Instructions executed (total): %d\n", instExec);
         printf("Memory references: %d\n", memRefs);
         printf("Clock cycles (step): %d\n", clockCycles);
         printf("Clock cycles (total): %d\n", totClock);
         for (j = 0; j < NUM_REGISTERS; j++) {
            printf("R%d = %08X\n", j, registers[j]); 
         }
      } else if (cmd == 'r') {
         while (i < numLines && executeReturn.pc >= 0) {
            if (memReturn.busy == 1 && wbReturn.busy != 1) {
               wbReturn = writeBack(memReturn);
               memReturn.busy = 0;
            }
            if (executeReturn.busy == 1 && memReturn.busy != 1) {
               memReturn = writeBack(executeReturn);
               executeReturn.busy = 0;
            }
            // check for memReturn.flush
            if (decodeReturn.busy == 1 && executeReturn.busy != 1) {
               executeReturn = execute(decodeReturn);
               decodeReturn.busy = 0;
            }
            if (fetchReturn.busy == 1 && decodeReturn.busy != 1) {
               decodeReturn = instructionDecode(fetchReturn);
               fetchReturn.busy = 0;
            }
            if (fetchReturn.busy != 1) {
               fetchReturn = instructionFetch(i);
            }
         
         
         
            //i = runCommand(&assembledLines[i], &memRefs, &totClock, i);
            if (executeReturn.pc > 0)
               instExec++;
         }

         printf("Instructions executed: %d\n", instExec);
         printf("Memory references: %d\n", memRefs);
         printf("Clock cycles: %d\n", totClock);
         for (j = 0; j < NUM_REGISTERS; j++) {
            printf("R%d = %08X\n", j, registers[j]); 
         }
      } else if (cmd == 'q') {
         i = -1;
      } else {
         printf("Invalid Command.\n");
      }
   }
}

void runProgram(int numLines) {
   char cmd;
   int i = 0, j, memRefs = 0, clockCycles = 0, instExec = 0, totClock = 0;

   initRegisters();

   while (i < numLines && i >= 0) {
      printf("Enter command (s for single step, r for run, q for quit): ");
      scanf(" %c", &cmd);

      if (cmd == 's') {
         clockCycles = 0;
         memRefs = 0;
         i = runCommand(&assembledLines[i], &memRefs, &clockCycles, i);
         instExec++;
         totClock += clockCycles;

         printf("Instructions executed (step): %d\n", 1);
         printf("Instructions executed (total): %d\n", instExec);
         printf("Memory references: %d\n", memRefs);
         printf("Clock cycles (step): %d\n", clockCycles);
         printf("Clock cycles (total): %d\n", totClock);
         for (j = 0; j < NUM_REGISTERS; j++) {
            printf("R%d = %08X\n", j, registers[j]); 
         }
      } else if (cmd == 'r') {
         while (i < numLines && i >= 0) {
            i = runCommand(&assembledLines[i], &memRefs, &totClock, i);
            if (i > 0)
               instExec++;
         }

         printf("Instructions executed: %d\n", instExec);
         printf("Memory references: %d\n", memRefs);
         printf("Clock cycles: %d\n", totClock);
         for (j = 0; j < NUM_REGISTERS; j++) {
            printf("R%d = %08X\n", j, registers[j]); 
         }
      } else if (cmd == 'q') {
         i = -1;
      } else {
         printf("Invalid Command.\n");
      }
   }
}

int main(int argc, char **argv) {
   FILE *code;
   int numLines = 0;

   code = fopen(argv[1], "r");
   
   numLines = constructSymbolTable(code);

   fclose(code);
   code = fopen(argv[1], "r");
   assemble(code);
   runProgram(numLines);

   return 0;
}
