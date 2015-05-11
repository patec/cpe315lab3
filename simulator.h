#ifndef SIMULATOR_H
#define SIMULATOR_H

#define AND_CODE 0x24
#define OR_CODE 0x25
#define ORI_CODE 0x0D << 26
#define ADD_CODE 0x20
#define ADDU_CODE 0x21
#define ADDI_CODE 0x08 << 26
#define ADDIU_CODE 0x09 << 26
#define SLL_CODE 0
#define SRL_CODE 0x02
#define SRA_CODE 0x03
#define SUB_CODE 0x22
#define SLT_CODE 0x2a
#define SLTI_CODE 0x0a
#define SLTU_CODE 0x2b
#define SLTIU_CODE 0x0b << 26
#define BEQ_CODE 0x04 << 26
#define BNE_CODE 0x05 << 26
#define LUI_CODE 0x0F << 26
#define LW_CODE 0x23 << 26
#define SW_CODE 0x2b << 26
#define J_CODE 0x02 << 26
#define JR_CODE 0x08
#define JAL_CODE 0x03 << 26
#define SYSCALL_CODE 0x0c

#endif
