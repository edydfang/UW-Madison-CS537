// Test that test dump_physmem(int*, int*, int);

#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_FRAMES  100
int frames[NUM_FRAMES];
int pids[NUM_FRAMES];

int
main(void)
{
  dump_physmem(frames, pids, NUM_FRAMES);
  for(int i=0;i<NUM_FRAMES;i++){
      printf(1, "%x\t%d\n", frames[i], pids[i]);
  }
  exit();
}
