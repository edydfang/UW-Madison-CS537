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

// convert from virtual address to array index
// range from 0 to MAX_FRAME_NUM (top to down)
#define VA2IDX(va) (( (uint) PHYSTOP >> 12) - ( (uint) V2P(va) >> 12) - 1)


void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld


void* start_page;

// high_pid: if higher page allocated, pid set to that pid; if not -1
// low_pid: if lower page allocated, pid set to that pid; if not -1
// when free page: check left and right sequentialy until meet one free page
struct run {
  struct run *next;
  int high_pid;
  int low_pid;
};

int pg2pid[MAX_FRAME_NUM]; // mapping from page to pids
int allocated_count = 0; // num of allocated pages
int initflag=0;

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
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
  initflag = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
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
  
  // we only care about page after kinit1 finished
  if (!initflag && ((uint) V2P(v)>>12) >=  ((uint)PHYSTOP>>12)-MAX_FRAME_NUM) {
    pg2pid[VA2IDX(v)] = -1;
  }
  if(!initflag){   
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
  }
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
  kmem.freelist = r->next;

  // we only care about page after kinit1 finished
  if ( ((uint) V2P(r)>>12) >=  ((uint)PHYSTOP>>12)-MAX_FRAME_NUM) {
    pg2pid[VA2IDX(r)] = pid;
    allocated_count ++;
    kmem.freelist = kmem.freelist->next;
    // cprintf("A: %d\tB: %x PID: %d %d\n", VA2IDX(r), V2P(r) >> 12, pid, allocated_count);
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

