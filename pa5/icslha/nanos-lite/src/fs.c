#include "fs.h"

//使用 ramdisk_read()和 ramdisk_write()来进行文件的真正读写
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
//am ioe.c 获取屏幕大小
extern void getscreen(int* width,int* height);
//device.c fb_write
extern void fb_write(const void *buf, off_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;    //文件被打开之后的读写指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int width=0,height=0;
  getscreen(&width,&height);
  file_table[FD_FB].size=width*height*sizeof(uint32_t);//一个像素4b
  Log("Init_fs\tfd=FD_FB: screan size is %d",file_table[FD_FB].size);

}

//accessory
size_t getfile_size(int fd)
{
    return file_table[fd].size;
}

//fs_open
int fs_open(const char* pathname,int flag,int mode)
{
    //忽略flags、mode
    for(int i=0;i<NR_FILES;i++)
    {
        if(strcmp(pathname,file_table[i].name)==0)
        {
            file_table[i].open_offset=0;
            return i;
        }
    }
    panic("%s is not in file_table",pathname);
    return -1;
}

extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len) ;
//fs_read
ssize_t fs_read(int fd,void* buf,size_t len)
{
    //Log("Debug: cd fs_read");
    assert(fd>0 && fd<NR_FILES);
    if(fd<3)
    {
        Log("fd < 3 when use fs_read/n");
        return 0;
    }
    //注意偏移量不要越过 文件的边界
    int maxbyte=file_table[fd].size-file_table[fd].open_offset;
    if(len>maxbyte)
    {
        //FD_DISPINFO 调用dispinfo_read;
        if(fd==FD_DISPINFO)
        {
            //Log("len>maxbyte fd==FD_DISPINFO");
            dispinfo_read(buf,file_table[fd].open_offset,maxbyte);
        }
        else if(fd==FD_EVENTS)
        {
            //不需要考虑大小和偏移
            return events_read(buf,len);
        }
        else//其他文件
        {
            ramdisk_read(buf,
                file_table[fd].disk_offset+file_table[fd].open_offset,
                maxbyte);
        }
        //更新偏移量
        file_table[fd].open_offset+=maxbyte;
        return maxbyte;
    }
    else
    {
        if(fd==FD_DISPINFO)
        {
            //Log("len< maxbyte fd==FD_DISPINFO");
            dispinfo_read(buf,file_table[fd].open_offset,len);
        }
        else if(fd==FD_EVENTS)
        {
            return events_read(buf,len);
        }
        else
        {
             ramdisk_read(buf,
             file_table[fd].disk_offset+file_table[fd].open_offset,
             len);
        }
        //更新偏移量
        file_table[fd].open_offset+=len;
        return len;
    }
}

//fs_write
ssize_t fs_write(int fd,void *buf, size_t len)
{
    //Log("Debug: cd fs_write");
    assert(fd>0 && fd<NR_FILES);
    if(fd<3)
    {
        Log("fd < 3 when use fs_write/n");
        return 0;
    }
    int maxbyte=file_table[fd].size-file_table[fd].open_offset;
    if(len>maxbyte)
    {
        if(fd==FD_FB)//VGA
        {
            fb_write(buf,file_table[fd].open_offset,maxbyte);
        }
        else
        {
            ramdisk_write(buf,
                        file_table[fd].disk_offset+file_table[fd].open_offset,
                        maxbyte);
        }
        //更新偏移量
        file_table[fd].open_offset+=maxbyte;
        return maxbyte;
    }
    else
    {
        if(fd==FD_FB)//VGA
        {
            fb_write(buf,file_table[fd].open_offset,len);
        }
        else
        {
            ramdisk_write(buf,
                            file_table[fd].disk_offset+file_table[fd].open_offset,
                            len);
        }
        //更新偏移量
        file_table[fd].open_offset+=len;
        return len;
    }
}

//fs_close
int fs_close(int fd)
{
    //fs_close()可以直接返回 0,表示总是关闭成功.
    assert(fd>0 && fd<NR_FILES);
    return 0;
}

//fs_lseek man 2 lseek
off_t fs_lseek(int fd,off_t offset,int whence)
{
    if(whence==SEEK_SET)
    {
        file_table[fd].open_offset=offset;
        return file_table[fd].open_offset;
    }
    else if(whence==SEEK_CUR)
    {
        file_table[fd].open_offset+=offset;
        return file_table[fd].open_offset;
    }
    else if(whence==SEEK_END)
    {
        file_table[fd].open_offset=getfile_size(fd)+offset;
        return file_table[fd].open_offset;
    }
    else{
        panic("whence=%d is not declared",whence);
        return 0;
    }
}
