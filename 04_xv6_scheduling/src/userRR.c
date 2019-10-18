// user-level toy scheduler

#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXJOB_COUNT 1000
int child_pid[MAXJOB_COUNT];

int
main(int argc, char *argv[])
{
  int time_slice, iterations, job_count;
  char *job_name;
  char **child_argv = malloc(sizeof(char*));
  if(argc != 5){
    printf(2, "usage: userRR <timeslice> <iterations> <job> <jobcount>\n");
    exit();
  }
  job_name = argv[3];
  time_slice = atoi(argv[1]);
  iterations = atoi(argv[2]);
  job_count = atoi(argv[4]);

  // printf(1,"forking new children...\n");
  child_argv[0] = job_name;
  // create child process
  for(int i=0; i<job_count; i++) {
    int pid = fork2(1);
    // printf(1, "[%x] child_pid %x\n", getpid(), pid);
    if(pid == 0) {
        // no argv
        exec(job_name, child_argv);
    }else{
        // parent
        // printf(1,"I'm parent...\n");
        child_pid[i] = pid;
    }
  }
  // printf(1,"start scheduling...\n");
  for(int i=0; i<iterations; i++) {
      for(int j=0; j<job_count; j++){
          setpri(child_pid[j], 2);
          sleep(time_slice);
          setpri(child_pid[j], 1);
      }
  }
  // printf(1,"Killing...\n");
  for(int i=0; i<job_count; i++){
      kill(child_pid[i]);
      wait();
  }
  // exit
  exit();
}