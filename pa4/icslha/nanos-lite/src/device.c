#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

extern int _read_key();//key
extern unsigned long _uptime();//time
size_t events_read(void *buf, size_t len) {
  //return 0;
  int key=_read_key();
  bool down = false;
  if (key & 0x8000) {
        key ^= 0x8000;
        down = true;
  }
  char temp[20];
  if(down && key == _KEY_F12) {
      extern void switch_current_game();
      switch_current_game();
      Log("key down:_KEY_F12, switch current game!");
   }
  if (key != _KEY_NONE)
  {
    if(down){
        sprintf(temp,"kd %s\n",keyname[key]);}
    else{
        sprintf(temp,"ku %s\n",keyname[key]);}
  }
  else{
    sprintf(temp,"t %d\n",_uptime());}

  if(strlen(temp)<=len)
  {
    strncpy((char*)buf,temp,len);
    return strlen(temp);
  }
  Log("strlen(event)=%d>len=%d",strlen(temp),len);
  return 0;
}

//dispinfo存储屏幕信息
static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
    //用于把字符串 dispinfo 中 offset 开始 的 len 字节写到 buf 中
    Log("dispinfo_read: len=%d",len);
    strncpy(buf,dispinfo+offset,len);
}
//am ioe.c 获取屏幕大小
extern void getscreen(int* width,int* height);
void fb_write(const void *buf, off_t offset, size_t len) {
    //Log("cd fb_write");
    if(offset%4!=0||len%4!=0)
    {
        panic("VGA 1像素 should be 4 byte");
    }
    //屏幕上offset处
    int width=0,height=0;
    getscreen(&width,&height);
    int index = offset/sizeof(uint32_t);//第多少个像素
    //x1 ,y1开始的位置
    int screen_y1 = index/width;
    int screen_x1 = index%width;
    //x2,y2结束的位置
    index=(offset+len)/sizeof(uint32_t);
    int screen_y2=index/width;
    int screen_x2 = index%width;

    if(screen_y2==screen_y1)
    {
        _draw_rect(buf,screen_x1,screen_y1,screen_x2-screen_x1,1);
        return;
    }
    else if(screen_y2-screen_y1==1)
    {
        _draw_rect(buf,screen_x1,screen_y1,width-screen_x1,1);
        _draw_rect(buf+4*(width-screen_x1),0,screen_y2,screen_x2,1);
        return;
    }
    else if(screen_y2-screen_y1>1)
    {
        _draw_rect(buf,screen_x1,screen_y1,width-screen_x1,1);
        int draw_index=width-screen_x1;
        _draw_rect(buf+draw_index*4,0,screen_y1+1,width,screen_y2-screen_y1-2);
        draw_index += width*(screen_y2-screen_y1-2);
        _draw_rect(buf+draw_index*4,0,screen_y2,screen_x2,1);
        return;
    }
    panic("screen_y2<screen_y1! erro");
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int width=0,height=0;
  getscreen(&width,&height);
  Log("WIDTH:%d\nHEIGHT:%d\n",width,height);
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d\n",width,height);

}
