#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
//  _switch(&pcb[i].as);
//  current = &pcb[i];
//  ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

int current_game=0;
void switch_current_game()
{
    current_game=2-current_game;
    //0 仙剑 2 videotest
    Log("current_game=%d",current_game);
}

_RegSet* schedule(_RegSet *prev) {
  //return NULL;
  if(current!=NULL)
  {
    current->tf=prev;
  }
  else
  {
    current=&pcb[current_game];
  }
  static int num=0;
  static const int frequent=100;
  if(current==&pcb[current_game])
  {
      if(current_game==0)
        {Log("run pal %d\n",num);}
      else if(current_game==2)
        {Log("run videotest %d\n",num);}
      num++;
  }
  else
  {
    current=&pcb[current_game];
  }
  if(num==frequent)
  {
    current=&pcb[1];
    Log("run hello \n");
    num=0;
  }
  _switch(&current->as);//切换地址空间
  //返回上下文
   return current->tf;
  /*
  **********需要加入优先级调度************
  //current=&pcb[0];//切换进程
  //切换到不同的进程
  current=(current==&pcb[0]?&pcb[1]:&pcb[0]);
  Log("as.ptr=0x%x\n",(uint32_t)current->as.ptr);
  _switch(&current->as);//切换地址空间
  */
}
