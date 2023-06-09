#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap
.globl vecself;  vecself:  pushl $0;  pushl $0x81; jmp asm_trap
.globl vectime; vectime: pushl $0; pushl $32; jmp asm_trap 
asm_trap:
  pushal

  pushl %esp
  call irq_handle

  #addl $4, %esp
  #切换进程，irq_handle()将trap frame存在eax
  mov %eax,%esp

  popal
  addl $8, %esp

  iret
