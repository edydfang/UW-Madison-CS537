#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"
#include "./proc.h"

struct pstat {
  int inuse[NPROC]; // whether this slot of the process table is in use (1 or 0)
  int pid[NPROC];   // PID of each process
  int priority[NPROC];  // current priority level of each process (0-3)
  enum procstate state[NPROC];  // current state (e.g., SLEEPING or RUNNABLE) of each process
  int ticks[NPROC][NLAYER];  // total num ticks each process has accumulated at each priority
  int qtail[NPROC][4]; // total num times moved to tail of this queue (e.g., setprio, end of timeslice, waking)
};

#endif // _PSTAT_H_