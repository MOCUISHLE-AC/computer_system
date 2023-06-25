#include "common.h"
#include "fs.h"
//pa4
#include "memory.h"
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
/* PA3*/
//  TODO();
//    ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
//    return (uintptr_t)DEFAULT_ENTRY//
//    int fd = fs_open(filename,0,0);
//    fs_read(fd,DEFAULT_ENTRY,getfile_size(fd));
//    fs_close(fd);
//    return (uintptr_t)DEFAULT_ENTRY;

/* PA4*/
    int fd = fs_open(filename,0,0);
    int size= getfile_size(fd);
    int page_num=size/PGSIZE;//页数量
    if(size%PGSIZE!=0)
        page_num++;
    void *pa=NULL;
    void *va=DEFAULT_ENTRY;
    for(int i=0;i<page_num;i++)
    {
        //申请空闲页
        pa=new_page();
        //va->pa
        _map(as,va,pa);
        fs_read(fd,pa,PGSIZE);
        va+=PGSIZE;
    }
    fs_close(fd);
    return (uintptr_t)DEFAULT_ENTRY;
}
