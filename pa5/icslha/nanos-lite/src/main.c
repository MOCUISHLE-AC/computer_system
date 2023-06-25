#include "common.h"

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

//MM 是指存储管理器(Memory Manager)模块,它专门负责分页相关的存储管理.
void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);
//pa4 add
extern void load_prog(const char *filename);
int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_ramdisk();

  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif

  init_fs();

  //uint32_t entry = loader(NULL, "/bin/pal");
  //((void (*)(void))entry)();
  load_prog("/bin/pal");
  load_prog("/bin/hello");
  load_prog("/bin/videotest");

  _trap();//内核自陷
  panic("Should not reach here");
}
