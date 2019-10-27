// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "./proc.h"
#define V2IDX(v) (V2P(r) >> 12)
#define IDX2V(idx) aa

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

uint count = 0;
void* start_page;

// high_pid: if higher page allocated, pid set to that pid; if not -1
// low_pid: if lower page allocated, pid set to that pid; if not -1
// when free page: check left and right sequentialy until meet one free page
struct run {
  struct run *next;
  int high_pid;
  int low_pid;
};

int pg2pid[MAX_FRAME_NUM]; //first convert index to frame num

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;


int va2idx(void * va) {
  // first part 0-4MB, second part 4MB-PHYSTOP, we should only consider the second part
  int idx = 0;
  if ((int) V2P(va)<4*1024*1024) {
      // 0~1023
      idx = (int) (V2P(va) >> 12);
  } else {
    idx = 1024 + (PGROUNDDOWN((PHYSTOP - V2P(va))) >> 12);
  }
  return idx;
}
int idx2framenum(int idx) {
  int frame_num = 0;
  if (idx < 1024) {
    frame_num = idx;
  } else {
    frame_num = (PHYSTOP >> 12) - (idx-1024) ;
  }
  return frame_num;
}

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  // init pg2pid
  for(int i=0; i<MAX_FRAME_NUM; i++) {
    pg2pid[i] = -1;
  }
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
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

// insert into the freelist and keep it sorted
void
insert_freelist(struct run *r){
  if(kmem.use_lock)
    acquire(&kmem.lock);
  struct run *prev = kmem.freelist;
  if(!prev || prev < r){
    kmem.freelist = r;
    r->next = prev;
  }else{
    while(prev->next!=0 && prev->next > r)
    {
      prev = prev->next;
    }
    r->next = prev->next->next;
    prev->next = r;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return;
}

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

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  r = (struct run*)v;
  insert_freelist(r);

}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(int pid)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);

  // cprintf("count %d\n", count++);
  // pid = myproc()->pid;
  pg2pid[va2idx(r)] = pid;
  cprintf("A: %x\tB: %x\n", idx2framenum(va2idx(r)), V2P(r) >> 12);
  return (char*)r;
}

