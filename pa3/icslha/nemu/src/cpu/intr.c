#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);//赋值给t0
  rtl_push(&t0);//eflags
  rtl_push(&cpu.cs);//cs
  rtl_li(&t0,ret_addr);//返回地址
  rtl_push(&t0);//eip

  //门描述符地址
  vaddr_t read_begin=cpu.idtr.base+NO*sizeof(GateDesc);
  //Log("%x",cpu.idtr.base);
  //读取
  uint32_t offset_0to15 = vaddr_read(read_begin,2);
  //Log("%x",offset_0to15);
  //16-32 btye 7 8
  uint32_t offset_16to32 =vaddr_read(read_begin+sizeof(GateDesc)-2,2);
  //Log("%x",offset_16to32);
  //跳转地址
  uint32_t target_addr=(offset_16to32<<16)+offset_0to15;
  decoding.is_jmp=1;
  decoding.jmp_eip=target_addr;
  //Log("target_addr %x",target_addr);
}

void dev_raise_intr() {
}
