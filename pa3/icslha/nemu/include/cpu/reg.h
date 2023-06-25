#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
  union{
     union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];
    /* Do NOT change the order of the GPRs' definitions. */
    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
     * in PA2 able to directly access these registers.
     */

     //eax~edi与gpr[8]共享内存
     struct{
       rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
     };
  };
  //eip
  vaddr_t eip;
  //实现eflags寄存器
  struct bs{
    unsigned int CF:1;//[0]
    unsigned int one:1;
    //unsigned int :4;
    unsigned int PF:1;//PF-PA2没用上
    unsigned int :1;
    unsigned int AF:1;//AF-PA2没用上
    unsigned int :1;
    unsigned int ZF:1;//[6]
    unsigned int SF:1;//[7]
    unsigned int :1;
    unsigned int IF:1;//[9]
    unsigned int :1;
    unsigned int OF:1;//[11]
    unsigned int :20;
  }eflags;
  //IDTR寄存器
  struct IDTR_register
  {
    uint32_t base;//基地址
    uint16_t limit;//offset
  }idtr;
  //cs寄存器16位
  rtlreg_t cs;//因为使用push指令暂时写为32bit

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

#endif
