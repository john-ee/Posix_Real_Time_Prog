#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#define SIG_F SIGRTMIN+1
#define SIG_C SIGRTMIN+2
#define TIMER 30
#define PRIORITY_F 3
#define PRIORITY_C 2
#define COUNT_LIMIT 30

int pid;



int main(int argc, char const *argv[]) {


  if ( (pid=fork()) == -1 ) {
    perror("fork");
    exit(1);
  }

	if(pid) {
    // FATHER
    struct sched_param father_params;
    int ret;
    father_params.sched_priority = PRIORITY_F;
    ret = sched_setscheduler(getpid(), SCHED_FIFO, &father_params);
    if (ret == -1) {
      perror("sched_setscheduler father");
      exit(1);
    }
  }

  else {
    // CHILD
    struct sched_param child_params;
    int ret;
    child_params.sched_priority = PRIORITY_C;
    ret = sched_setscheduler(getpid(), SCHED_FIFO, &child_params);
    if (ret == -1) {
      perror("sched_setscheduler child");
      exit(1);
    }
  }



  return 0;
}
