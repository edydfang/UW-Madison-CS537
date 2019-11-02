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
    r->next = prev->next;
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
  
  // we only care about page after kinit1 finished
  if ( ((uint) V2P(v)>>12) >=  ((uint)PHYSTOP>>12)-MAX_FRAME_NUM) {
    // cprintf("%x %x\n", VA2IDX(v), (PHYSTOP >> 12)-1);
    pg2pid[VA2IDX(v)] = -1;
    if (initflag) {
      allocated_count --;
    }
  }
  insert_freelist(r);

}



// Called by select_page
// return 1 if the current page is safe
int
safe_guard(int index, int pid) {
    // flag is used to determine whether this is safe, 0 is unsafe
    int flag = 1;
    // if this is the first page and doesn't have the previous one
    if (index == 0) {
        if (pid == pg2pid[index + 1] || pg2pid[index + 1] == -1 || pg2pid[index + 1] == -2)
            return 1;
        else return 0;
    } else {
        // compare with the lower page
        if (pg2pid[index + 1] != pid && pg2pid[index + 1] != -1 && pg2pid[index + 1] != -2) {
            flag = 0;
        }
        // compare with the upper page
        if (pg2pid[index - 1] != pid && pg2pid[index - 1] != -1 && pg2pid[index - 1] != -2) {
            flag = 0;
        }
    }
    return flag;
}

// select a va for the current pid process
// compare with the pid of previous and the next page
// if they are all different, return the address
// else, find next address
// @See: safe_guard
struct run *
select_page(int pid) {
    // 1 if we select the kmem.freelist, see line 187
    int if_freelist = 1;
    struct run *r;
    struct run *prev;
    r = kmem.freelist;

    int index = VA2IDX(r);
//    if (pid > 0) {
//        cprintf("===========185\n");
//        cprintf("current page number = [%x]\n", r);
//    }
    while (!safe_guard(index, pid)) {
        if_freelist = 0;
        prev = r;
        r = r->next;
//        if (pid > 0)
//            cprintf("current page number = [%x]\n", r);
        index = VA2IDX(r);
    }
    // fix the freelist linkedlist
    if (if_freelist == 1) {
        kmem.freelist = r->next;
    } else {
        prev->next = r->next;
    }
    return r;
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
    // Select the first safe page
    // @see select_page
    if(pid == -2){
        // TODO: To prevent from that attack, UNKNOWN process should also be 
        // viewed as a process
        r = kmem.freelist;
        if(r)
            kmem.freelist = r->next;
    } else {
        r = select_page(pid);
    }
  if(kmem.use_lock)
    release(&kmem.lock);

  // we only care about page after kinit1 finished
  if ( ((uint) V2P(r)>>12) >=  ((uint)PHYSTOP>>12)-MAX_FRAME_NUM) {
    pg2pid[VA2IDX(r)] = pid;
    allocated_count ++;
    // cprintf("A: %d\tB: %x PID: %d %d\n", VA2IDX(r), V2P(r) >> 12, pid, allocated_count);
  }
  return (char*)r;
}

