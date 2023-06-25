#include "common.h"
#include "syscall.h"
#include "fs.h"

int mysys_write(int fd,void* buf,size_t len);
int mysys_brk(int addr);
_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);//eax
  a[1] =SYSCALL_ARG2(r);//ebx
  a[2]=SYSCALL_ARG3(r);//ecx
  a[3]=SYSCALL_ARG4(r);//edx
  switch (a[0]) {
    case SYS_none:
        SYSCALL_ARG1(r)=1;//do noting
        break;
    case SYS_exit:
        _halt(a[1]);
        break;
    case SYS_write:
        //a[1]= fd a[2]=buf a[3]=len
        SYSCALL_ARG1(r)=mysys_write(a[1],(void*)a[2],a[3]);
        break;
    case SYS_brk:
        SYSCALL_ARG1(r)=mysys_brk(a[1]);
        break;
    case SYS_read:
        SYSCALL_ARG1(r)=fs_read(a[1],(void*)a[2],a[3]);
        break;
    case SYS_open:
        SYSCALL_ARG1(r)=fs_open((char*)a[1],0,0);
        break;
    case SYS_close:
        SYSCALL_ARG1(r)=fs_close(a[1]);
        break;
    case SYS_lseek:
        SYSCALL_ARG1(r)=fs_lseek(a[1],a[2],a[3]);
        break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

//ssize_t write(int fd, const void *buf, size_t count);
int mysys_write(int fd,void* buf,size_t len)
{
    //stdout 或 stderr
    if(fd ==1 || fd ==2)
    {
        //Log("buffer:%s",(char*)buf);
        for(int i=0;i<len;i++)
        {
            char temp;//用于传参给串口
            //需要使用char* 强制类型转换
            temp=*(char*)(buf+i);
            _putc(temp);
        }
        return len;
    }
    else if(fd>=3)
    {
        return fs_write(fd,buf,len);
    }
    else
    {
        panic("fd = %d is not available\n",fd);
    }
    return -1;
}

extern int mm_brk(uint32_t new_brk);
//sbrk(intptr_t increment)
int mysys_brk(int addr)
{
    //单任务总是返回0,
    //return 0;
    return mm_brk(addr);
}
