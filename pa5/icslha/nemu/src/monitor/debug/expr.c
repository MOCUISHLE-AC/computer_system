#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
#include <assert.h>
enum {
  TK_NOTYPE = 256,
  DECIMIAL,//十进制
  HEXADECIMAL,//八进制
  leftparen,//(
  rightparen,//)
  times,//*
  divide,// 除
  add, //+
  minus,//-
  TK_EQ,// ==
  TK_NOTEQ,//!=
  TK_AND, // &&
  TK_OR, // ||
  TK_NOT, // !
  Regname,//取寄存器中的值 $
  DEREF,// * 引用
  RightMinus,// - 负号
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0[xX][0-9A-Fa-f]+",HEXADECIMAL}, //八进制
  {"[1-9][0-9]*|0",DECIMIAL},   //16进制
  {"\\$[a-z]{2,3}",Regname},
  {"\\(",leftparen},
  {"\\)",rightparen},
  {"\\*",times},           //乘
  {"\\/",divide} ,          //除
  {"\\+", add},         // plus
  {"\\-",minus},     //减
  {"==", TK_EQ},       // equal
  {"!=",TK_NOTEQ},      //not equal
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!",TK_NOT},

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
            case TK_NOTYPE:
                //do nothing
                break;
            case Regname:
                //需要去除$
                tokens[nr_token].type=rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
                nr_token++;
                break;
          default:
                tokens[nr_token].type=rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                nr_token++;
                ;
        }

        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

//()检查
int check_parentheses(int p,int q)
{
    int counter=0;
    for(int i=p;i<q+1;i++)
    {
        if(tokens[i].type==leftparen)
            counter++;
        else if(tokens[i].type==rightparen)
            counter--;
    }
    if(counter!=0)
    {
        panic("( ),不匹配\n");
        return -1;
    }
    else
    {
        if(tokens[p].type==leftparen&&tokens[q].type==rightparen)
        {
             return 1;//()
        }
        else
        {
            return 0;//非()
        }
    }
}

//find dominant
int dominant(int p,int q)
{
    int counter=0;
    int queue[100]={0};//queue[]dominant位置
    int index=0;//index 索引queue
    for(int i=p;i<q+1;i++)
    {
        if(tokens[i].type==leftparen)
            counter++;
        else if(tokens[i].type==rightparen)
            counter--;
        else if(counter==0)
        {
            //不是括号中的
            if(tokens[i].type>=times&&tokens[i].type<=RightMinus)
            {
                queue[index++]=i;
            }
        }
    }
    int prior=10;//最低优先
    int myindex=0;
    //定义优先级《-
    for(int i=index-1;i>=0;i--)
    {
        if(tokens[queue[i]].type==TK_AND||tokens[queue[i]].type==TK_OR)//1
        {
                if(1<prior&&tokens[queue[i]].type==TK_AND)
                {
                    myindex=i;
                    prior=1;
                }
                if(0<prior&&tokens[queue[i]].type==TK_OR)
                {
                    myindex=i;
                    prior=0;
                }
        }
        if(tokens[queue[i]].type==TK_EQ||tokens[queue[i]].type==TK_NOTEQ)//2
        {
            if(2<prior)
            {
                myindex=i;
                prior=2;
            }
        }
        if(tokens[queue[i]].type==add||tokens[queue[i]].type==minus)//3
        {
            if(3<prior)
            {
                myindex=i;
                prior=3;
            }
        }
       if(tokens[queue[i]].type==times||tokens[queue[i]].type==divide)//4
       {
            if(4<prior)
            {
                myindex=i;
                prior=4;
            }
       }
       if(tokens[queue[i]].type==TK_NOT||tokens[queue[i]].type==RightMinus||tokens[queue[i]].type==DEREF||tokens[queue[i]].type==Regname)//5
        {
            if(5<prior)//所有单目运算符
            {
                myindex=i;
                prior=5;
            }
        }
    }
    //单目运算符重新遍历-》
    if(prior==5)//!
    {
        prior=10;
        myindex=0;
        for(int i=0;i<index;i++)
        {
            if(tokens[queue[i]].type==TK_AND||tokens[queue[i]].type==TK_OR)//1
            {
                if(1<prior&&tokens[queue[i]].type==TK_AND)
                {
                    myindex=i;
                    prior=1;
                }
                if(0<prior&&tokens[queue[i]].type==TK_OR)
                {
                    myindex=i;
                    prior=0;
                }
            }
            if(tokens[queue[i]].type==TK_EQ||tokens[queue[i]].type==TK_NOTEQ)//2
            {
                if(2<prior)
                {
                    myindex=i;
                    prior=2;
                }
            }
            if(tokens[queue[i]].type==add||tokens[queue[i]].type==minus)//3
            {
                if(3<prior)
                {
                    myindex=i;
                    prior=3;
                }
            }
           if(tokens[queue[i]].type==times||tokens[queue[i]].type==divide)//4
           {
                if(4<prior)
                {
                    myindex=i;
                    prior=4;
                }
           }
           if(tokens[queue[i]].type==TK_NOT||tokens[queue[i]].type==RightMinus||tokens[queue[i]].type==DEREF||tokens[queue[i]].type==Regname)//5
            {
                if(5<prior)
                {
                    myindex=i;
                    prior=5;
                }
            }
        }
    }
    return queue[myindex];
}

//eval 计算
int eval(int p,int q)
{
    printf("Debug:p=%d \t q=%d\n",p,q);
    if(p>q)
    {
        panic("eval()时，p>q\n");
        return -1;
    }
    else if(p==q)
    {
        if(tokens[p].type==DECIMIAL)
            return atoi(tokens[p].str);
        else if(tokens[p].type==HEXADECIMAL)
        {
            uint32_t num = 0;
            sscanf(tokens[p].str, "%x", &num);
            return num;
        }
        //处理读取寄存器
        else if(tokens[p].type==Regname)
        {
            printf("Debug:%s\n",tokens[p].str);
            for(int i=0;i<8;i++)
            {
                if(strcmp(tokens[p].str,regsl[i])==0)
                    return reg_l(i);
            }
            if(strcmp(tokens[p].str,"eip")==0)
                return cpu.eip;
            if(strcmp(tokens[p].str,"cr")==0)
                return cpu.CR3;
        }

    }
    else if(check_parentheses(p,q)==1)
    {
        return eval(p+1,q-1);
    }
    else
    {
        int op=dominant(p,q);
        printf("Debug:op=%d\n",op);
        //单目运算符
        int val1=0;
        if(p<op)
        {
            val1=eval(p,op-1);
        }
        int val2=eval(op+1,q);
        switch(tokens[op].type)
        {
            case add:
                return val1 +val2;
            case minus:
                return val1-val2;
            case times:
                return val1*val2;
            case divide:
                return val1/val2;
            case TK_EQ:
                if(val1==val2)
                    return true;
                else
                    return false;
            case TK_NOTEQ:
                if(val1!=val2)
                    return true;
                else
                    return false;
            case TK_AND:
                if(val1&&val2)
                    return true;
                else
                    return false;
            case TK_OR:
                if(val1||val2)
                    return true;
                else
                    return false;
            case TK_NOT:
                if(val2!=0)
                    return false;
                else
                    return true;
            case RightMinus:
                return -1*val2;
            case DEREF:
                return vaddr_read(val2,4);
        }
    }
    panic("eval不可能执行到这里\n");
    return -1;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  //处理 - * 单目运算符
  for(int i=0;i<nr_token;i++)
  {
    if(tokens[i].type==times&&(i==0||(tokens[i-1].type!=DECIMIAL&&tokens[i-1].type!=HEXADECIMAL&&tokens[i-1].type!=rightparen)))
    {
        tokens[i].type=DEREF;//引用
    }
    else if(tokens[i].type==minus&&(i==0||(tokens[i-1].type!=DECIMIAL&&tokens[i-1].type!=HEXADECIMAL&&tokens[i-1].type!=rightparen)))
    {
        tokens[i].type=RightMinus;//-
    }
  }
  printf("Debug:nr_token=%d\n",nr_token);
  /* TODO: Insert codes to evaluate the expression. */
  return eval(0,nr_token-1);
}

