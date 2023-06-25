#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t2); //临时寄存器保存值
  operand_write(id_dest,&t2);//使用operand_write执行写操作

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //TODO();
  //Push EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI
  t1=cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t1);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  //TODO();
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t1);//skip esp
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {
  //TODO();将栈指针指向帧指针，然后 POP 备份的原帧指针到%EBP。
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);
  print_asm("leave");
}

make_EHelper(cltd) {
  //把 eax 的 32 位整数扩展 为 64 位，高 32 位用 eax 的符号位填充保存到edx，
  //或 ax 的 16 位整数扩展为 32 位，高 16 位用 ax 的符号位填充保存到 dx。
  if (decoding.is_operand_size_16) {
    //TODO();
    rtl_lr_w(&t0,R_AX);
    rtl_sext(&t0,&t0,2);//符号扩展
    rtl_sari(&t0,&t0,31);
    rtl_sr_w(R_DX,&t0);
  }
  else {
    //TODO();
    rtl_sari(&cpu.edx,&cpu.eax,31);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {

  if (decoding.is_operand_size_16) {
    //TODO();
    rtl_lr_b(&t0, R_AX);
    rtl_sext(&t0, &t0, 1);
    rtl_sr_w(R_AX, &t0);
  }
  else {
    //TODO();
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t0, &t0, 2);
    rtl_sr_l(R_EAX, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

//pa 4 add
make_EHelper(mov_store_cr)
{
    rtl_store_cr(id_dest->reg,&id_src->val);
    print_asm_template2(mov);
}
