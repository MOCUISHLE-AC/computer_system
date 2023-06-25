#include <am.h>
#include <x86.h>


#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
    unsigned long time_ms=inl(RTC_PORT)-boot_time;
    return time_ms;
  //return 0;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  //直接cv native
  //int temp = w< _screen.width - x?w: _screen.width - x;
  int temp=0;
  if(w< _screen.width - x)
  {
    temp=w;
  }
  else
  {
    temp=_screen.width - x;
  }
  int cp_bytes = sizeof(uint32_t) * temp;
  for (int j = 0; j < h && y + j < _screen.height; j ++) {
    memcpy(&fb[(y + j) * _screen.width + x], pixels, cp_bytes);
    pixels += w;
  }
}

void _draw_sync() {
}

int _read_key() {
  if(inb(0x64))
    return inl(0x60);
  else
    return _KEY_NONE;
}

//API 获取屏幕大小
void getscreen(int* width,int* height)
{
    *width=_screen.width;
    *height=_screen.height;
}
