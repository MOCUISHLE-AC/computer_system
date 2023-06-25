#ifndef __FS_H__
#define __FS_H__

#include "common.h"

enum {SEEK_SET, SEEK_CUR, SEEK_END};
//fs.c中函数
int fs_open(const char* pathname,int flag,int mode);
ssize_t fs_read(int fd,void* buf,size_t len);
ssize_t fs_write(int fd,void* buf,size_t len);
int fs_close(int fd);
size_t getfile_size(int fd);//获取文件大小，为了别的文件可以访问
off_t fs_lseek(int fd,off_t offset,int whence);
#endif
