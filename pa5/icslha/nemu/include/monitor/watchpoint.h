#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char wpexpr[32];//表达式
  uint32_t value;//wp值

} WP;

bool isChanged();
WP* new_wp(char* expr);
void free_wp(WP *wp);
#endif
