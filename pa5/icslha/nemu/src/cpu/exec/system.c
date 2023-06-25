#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();
extern void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(lidt) {
  //TODO();
  //eax 32位操作数，base 32 limit 16
  t1=id_dest->val;
  rtl_lm(&t0,&t1,2);
  cpu.idtr.limit=t0;
  //Log("cpu.idtr.limit=%x",cpu.idtr.limit);
  t2=id_dest->val+2;
  rtl_lm(&t0,&t2,4);
  cpu.idtr.base=t0;
  //Log("cpu.idtr.base=%x",cpu.idtr.base);
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  //TODO();idest = imm
  uint8_t NO= (uint8_t) id_dest->val;
  //Log("int NO=%d, seq_eip=0x%x",NO,decoding.seq_eip);
  raise_intr(NO,decoding.seq_eip);//eip下条指令
  print_asm("int %s ", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //TODO();
  //它将栈顶的三个元素来依次解释成 EIP,CS,EFLAGS,并恢复它们.
  rtl_pop(&cpu.eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&t0);
  memcpy(&cpu.eflags,&t0,sizeof(cpu.eflags));
  decoding.jmp_eip=1;//
  decoding.seq_eip=cpu.eip;
  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //TODO();
  rtl_li(&t0,pio_read(id_src->val,id_dest->width));
  operand_write(id_dest,&t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();
  pio_write(id_dest->val,id_src->width,id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
