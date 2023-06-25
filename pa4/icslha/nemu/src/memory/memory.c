#include "nemu.h"
#include "device/mmio.h"
//add
#include "memory/mmu.h"
#define PMEM_SIZE (128 * 1024 * 1024)

//pa4 add
//取前20位
#define PTE_ADDR(pte) ((uint32_t)(pte)& ~0xfff)
//页目录
#define PDX(va) (((uint32_t)(va)>>22) & 0x3ff)
//二级页表
#define PTX(va) (((uint32_t)(va)>>12) & 0x3ff)
//offset
#define OFF(va) ((uint32_t)(va) & 0xfff)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

paddr_t page_translate(vaddr_t addr,bool flag)
{
    //CR0转为mmu中定义的结构体
    CR0 cr0=(CR0)cpu.CR0;
    if(!(cr0.paging&&cr0.protect_enable))
        return addr;
    //CR3转为mmu中定义的结构体
    CR3 cr3=(CR3)cpu.CR3;
    //页目录
    //CR3中base，页目录表基址
    PDE* base=(PDE*) PTE_ADDR(cr3.val);//
    PDE pde=(PDE)paddr_read((uint32_t)(base+PDX(addr)),4);
    assert(pde.present);

    //二级页表
    PTE* ptab=(PTE*)PTE_ADDR(pde.val);
    PTE pte=(PTE)paddr_read((uint32_t)(ptab+PTX(addr)),4);
    assert(pte.present);

    //access、dirty
    pde.accessed=1;
    pte.accessed=1;
    if(flag)
        pte.dirty=1;

    //物理地址
    paddr_t real=PTE_ADDR(pte.val)|OFF(addr);
    //Log("virtual addr=%x\t real address:%x",addr,real);
    return real;
}


uint32_t paddr_read(paddr_t addr, int len) {
    if(is_mmio(addr)==-1)
    {
        return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
    }
    else
    {
        return mmio_read(addr,len,is_mmio(addr));
    }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  //memcpy(guest_to_host(addr), &data, len);
   if(is_mmio(addr)==-1)
   {
        memcpy(guest_to_host(addr), &data, len);
   }
   else
   {
        mmio_write(addr,len,data,is_mmio(addr));
   }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  //return paddr_read(addr, len);
  if(PTE_ADDR(addr)!=PTE_ADDR(addr+len-1))
  {
    //Log("read跨页了，需要拼接\n");
    int len1=0x1000-OFF(addr);
    int len2=len-len1;
    //分别映射
    paddr_t paddr1=page_translate(addr,false);
    paddr_t paddr2=page_translate(addr+len1,false);

    //组合返回值
    uint32_t low=paddr_read(paddr1,len1);
    uint32_t high=paddr_read(paddr2,len2);
    uint32_t result=high<<(len1*8)|low;
    //Log("high:%x\t low:%x\t result:%x\n",high,low,result);
    return result;
  }
  else
  {
    paddr_t paddr=page_translate(addr,false);
    return paddr_read(paddr,len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  //paddr_write(addr, len, data);
    if(PTE_ADDR(addr)!=PTE_ADDR(addr+len-1))
    {
      Log("write跨页了，需要拼接\n");
      int len1=0x1000-OFF(addr);
      int len2=len-len1;
      //分别映射
      paddr_t paddr1=page_translate(addr,true);
      paddr_t paddr2=page_translate(addr+len1,true);
      //组合
      uint32_t low=data&(~0u>>((4-len1)<<3));
      uint32_t high=data>>((4-len2)<<3);

      paddr_write(paddr1,len1,low);
      paddr_write(paddr2,len2,high);
    }
    else
    {
      paddr_t paddr=page_translate(addr,true);
      paddr_write(paddr,len,data);
    }
}
