#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32
uint32_t expr(char *e, bool *success);

static WP wp_pool[NR_WP];
static WP *head, *free_;
WP** get_static_head(){
    return &head;
}
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char* expr1)
{
    if(free_==NULL)
    {
        panic("free_ is empty\n");
    }
    else
    {
        //处理free_
        WP* free_temp=free_;
        free_=free_->next;
        free_temp->next=NULL;
        //初始化
        bool success=true;
        strcpy(free_temp->wpexpr,expr1);
        free_temp->value=expr(free_temp->wpexpr,&success);
        //处理head
        if(head==NULL)
        {
            head=free_temp;
        }
        else
        {
            WP* index=head;
            while(index->next!=NULL)
            {
                index=index->next;
            }
            index->next=free_temp;
        }
        return free_temp;
    }
}

void free_wp(WP *wp)
{
    if(wp==head)
    {
        head=head->next;
    }
    else
    {
        WP* index=head;
        while(index!=NULL)
        {
            if(index->next==wp)
            {
                //head
                index->next=wp->next;
                //free_
                wp->next=free_;
                wp=free_;
                break;
            }
            index=index->next;
        }
        if(index==NULL)
        {
            panic("free_wp 找不到 wp\n");
        }
    }
}

bool isChanged()
{
    WP* index=head;
    bool flag=false;
    while(index!=NULL)
    {
        bool success=true;
        uint32_t now_value=expr(index->wpexpr, &success);
        if(now_value!=index->value)
        {
            printf("触发监视点:%s value:%d->%d\n",index->wpexpr,index->value,now_value);
            index->value = now_value;
            flag=true;
        }
        index = index->next;
    }
    return flag;
}
