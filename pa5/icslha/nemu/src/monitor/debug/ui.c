#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
extern WP** get_static_head();
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

//单步调试 si
static int cmd_si(char *args) {
  //Debug
  printf("Debug: si %s \n",args);
  //单步调试
  if(args==NULL)
  {
    int si_step=1;
    cpu_exec(si_step);
  }
  //多步调试 si [N]
  else
  {
    int si_step=atoi(args);
    cpu_exec(si_step);
  }
  return 0;
}

//打印寄存器 info r
static int cmd_infoReg(char *args) {
    if(strcmp(args,"r")==0)
    {
        for(int i=R_EAX;i<=R_EDI;i++)
        {
           //寄存器中的值
           uint32_t regValue= reg_l(i);
           //寄存器名在regsl[]中reg.c
           printf("%s:\t OX%08x \n",regsl[i],regValue);
        }
        //pc
        printf("eip:\t OX%08x \n",cpu.eip);

        printf("CR0:\t 0x%x\n",cpu.CR0);
        printf("CR3:\t 0x%x\n",cpu.CR3);
        return 0;
    }
    else if(strcmp(args,"w")==0)
    {
        WP** head = get_static_head();
        WP* index = *head;
        int i=0;
        while(index!=NULL)
        {
            printf("WP[%d]: expr:%s \t value:%d\n",i,index->wpexpr,index->value);
            index = index->next;
            i++;
        }
        return 0;
    }
    else
        return 0;
}

//扫描内存
static int cmd_x(char *args)
{
    if(args==NULL)
    {
        printf("need args x [N] address");
        return 0;
    }
    char* agr_1=strtok(NULL," ");
    int step = atoi(agr_1);
    char* agr_2=strtok(NULL," ");
    uint32_t address = 0;
    sscanf(agr_2, "%x", &address);
    //分割后的指令
    printf("Debug:: x i=%d adress=OX%08x\n",step,address);
    //调用memory中的paddr_read
    for(int i=0;i<step;i++)
    {
        //打印内存
        printf("0x%8x  0x%x\n",address + i*4,vaddr_read(address+i*4,4));
    }
    return 0;
}

//计算表达式
static int cmd_p(char *args) {
   bool success=true;
   printf("expr=%d\n",expr(args,&success));
   return 0;
}
//新建检查点
static int cmd_w(char *args) {
   WP* myindex=new_wp(args);
   printf("new WP expr:%s \t value:%d\n",myindex->wpexpr,myindex->value);
   return 0;
}

//删除检查点
static int cmd_d(char *args) {
    char* agr_1=strtok(NULL," ");
    int delete_num = atoi(agr_1);
    printf("delete WP %d\n",delete_num);
    WP** head = get_static_head();
    WP* index = *head;
    free_wp(index + delete_num);
   return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  //单步调试
  {"si","si [N] for Debug",cmd_si},
  //打印寄存器,list检查点
  {"info","info + r print information about the Regs;info + w list wacth point",cmd_infoReg},
  //扫描内存
  {"x","x is to scan the memory, x [N] address print [address,address+N]",cmd_x},
  //计算表答式
  {"p","caculate the expr",cmd_p},
  //新建检查点
  {"w","new a watchpoint",cmd_w},
  //删除检查点
  {"d","delete a watchpoint",cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
