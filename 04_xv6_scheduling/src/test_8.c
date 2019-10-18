#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include <stddef.h>

#define check(exp, msg) if(exp) {} else {\
  printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
  ;}

#define DDEBUG 1

#ifdef DDEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif


int
main(int argc, char *argv[])
{
  struct pstat *st = NULL;
  //check(getpinfo(&st) == 0, "getpinfo");
  //*st = NULL;
  
  int gret;

  gret = getpinfo(st);
  
  if(gret == -1){
    printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  }else{
    printf(1, "XV6_SCHEDULER\t getpinfo FAILED to return thr proper error code\n");
  }
  
  exit();
}
