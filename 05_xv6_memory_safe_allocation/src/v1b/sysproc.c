#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


int sys_dump_physmem(void)
{
  int* frames;
  int* pids;
  int num_frame;
  if(argint(2, (void *)&num_frame)) {
    return -1;
  }
  if(argptr(0, (void*)&frames, num_frame*sizeof(int)) < 0 || 
    argptr(1, (void*)&pids, num_frame*sizeof(int)) < 0 ) {
      return -1;
  }
  if(num_frame < 1 || frames == 0 || pids == 0){
    return -1;
  }
  int i = 0, j = 0;
  // filled all data of used pages
  while(i<num_frame && i<allocated_count) {
    while(pg2pid[j]==-1){
      j++;
    }
    frames[i] = IDX2FN(j);
    pids[i] = pg2pid[j];
    i ++;
    j ++;
  }
  // unused array
  if(i<num_frame){
    for(;i<num_frame;i++){
      frames[i] = -1;
      pids[i] = -1;
    }
  } 
  return 0;
}