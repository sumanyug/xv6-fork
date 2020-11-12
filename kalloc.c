// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int num_phys_page[PHYSTOP/PGSIZE];
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE){
      kfree(p);
      kmem.num_phys_page[get_index_pte(p)] = 0;
  }
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  uint pte_num;
  pte_num = get_index_pte(v);
  if(kmem.use_lock)
    acquire(&kmem.lock);
  // //cprintf("Subtracting: %d\n",pte_num);
  if(pte_num  == 0){
    panic("kfree: pte number could not be obtained");
  }
  kmem.num_phys_page[pte_num]--;
  if(kmem.num_phys_page[pte_num] < 0){
    kmem.num_phys_page[pte_num] = 0;
  }
  if(kmem.num_phys_page[pte_num] == 0){
    memset(v, 1, PGSIZE);
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    uint index = get_index_pte((char*)r);
    kmem.num_phys_page[index] = 1;
    kmem.freelist = r->next;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

int
getNumFreePages(void)
{
  struct run *r;
  int count = 0;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    count ++;
    r = r->next;
  }
  if(kmem.use_lock)
    release(&kmem.lock);

  return count;

}

uint increment_page_num(char* virt_addr){
  uint pte_num = get_index_pte(virt_addr);
  uint answer = 0;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  kmem.num_phys_page[pte_num] ++;
  answer = kmem.num_phys_page[pte_num];
  if(kmem.use_lock)
    release(&kmem.lock);
  return answer;
}

uint get_phys_count(char* virt_addr){
  uint pte_num = get_index_pte(virt_addr);
  uint answer = 0;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  answer = kmem.num_phys_page[pte_num];
  if(kmem.use_lock)
    release(&kmem.lock);
  return answer;

}
uint decrement_page_num(char* virt_addr){
  uint pte_num = get_index_pte(virt_addr);

  uint answer = 0;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  kmem.num_phys_page[pte_num] --;
  if (kmem.num_phys_page[pte_num] < 0)
    kmem.num_phys_page[pte_num] = 0;
  answer = kmem.num_phys_page[pte_num];
  if(kmem.use_lock)
    release(&kmem.lock);
  return answer;
}