#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  ;}

#define DDEBUG 1

#ifdef DDEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

//char buf[10000]; // ~10KB
// work n then sleep t then work n
int workload(int n, int t) {
  int i, j = 0;
  for (i = 0; i < n; i++) {
    j += i * j + 1;
  }

  if (t > 0) sleep(t);
  for (i = 0; i < n; i++) {
    j += i * j + 1;
  }
  return j;
}

int
main(int argc, char *argv[])
{
  struct pstat st;
  check(getpinfo(&st) == 0, "getpinfo");

  // Push this thread to the bottom
  workload(80000000, 0);

  int i, j, k;
  // Launch the 4 processes, but process 2 will sleep in the middle
  for (i = 0; i < 1; i++) {
    int c_pid = fork2(0);
    int t = 0;
    // Child
    if (c_pid == 0) {
      if (i % 2 == 1) {
          t = 64*5; // for this process, give up CPU for one time-slice
      }
      workload(300000000, t);
      exit();
    } else {
      //setpri(c_pid, 2);
    }
  }

  for (i = 0; i < 12; i++) { 
    sleep(20);
    check(getpinfo(&st) == 0, "getpinfo");
    
    for (j = 0; j < NPROC; j++) {
      if (st.inuse[j] && st.pid[j] >= 3 && st.pid[j] != getpid()) {
        DEBUG_PRINT((1, "XV6_SCHEDULER\t CHILD %d\n", st.pid[j]));
        //DEBUG_PRINT((1, "pid: %d\n", st.pid[j]));
        for (k = 3; k >= 0; k--) {
          DEBUG_PRINT((1, "XV6_SCHEDULER\t \t level %d ticks used %d\n", k, st.ticks[j][k]));
          DEBUG_PRINT((1, "XV6_SCHEDULER\t \t level %d qtail %d\n", k, st.qtail[j][k]));
        }
      } 
    }
  }

  for (i = 0; i < 6; i++) {
    wait();
  }

  //printf(1, "TEST PASSED");

  exit();
}
