#include "common.h"
#include "fs.h"
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
//  ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
//  return (uintptr_t)DEFAULT_ENTRY;
    int fd = fs_open(filename,0,0);
    fs_read(fd,DEFAULT_ENTRY,getfile_size(fd));
    fs_close(fd);
    return (uintptr_t)DEFAULT_ENTRY;
}
