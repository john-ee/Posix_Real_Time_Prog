#include <unistd.h>
#include <stdio.h>
//#include <sys/types.h>
//#include <sys/wait.h>
#include <signal.h>
//#include <errno.h>
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
int k_final = 0;


int main(int argc, char const *argv[]) {

  int i = 9;

  struct timespec ref;
  clock_gettime(CLOCK_REALTIME, &ref);
  double t = (double)ref.tv_sec*1e9 + (double)ref.tv_nsec;
  int k_final = 0;
  while (((double)ref.tv_sec*1e9 + (double)ref.tv_nsec)<t+1e6) {
    clock_gettime(CLOCK_REALTIME, &ref);
    k_final++;
  }

  int k = 0;
  while (i>0) {
    struct timespec ref;
    clock_gettime(CLOCK_REALTIME, &ref);
    double t = (double)ref.tv_sec*1e9 + (double)ref.tv_nsec;
    k = 0;
    while (((double)ref.tv_sec*1e9 + (double)ref.tv_nsec)<t+1e6) {
      clock_gettime(CLOCK_REALTIME, &ref);
      k++;
    }
    //printf("%d\n",k);
    if (k<k_final)
      k_final = k;
    i--;
  }

  struct sched_param father_params;
  int ret;
  father_params.sched_priority = PRIORITY_F;
  ret = sched_setscheduler(getpid(), SCHED_FIFO, &father_params);
  if (ret == -1) {
    perror("sched_setscheduler father");
    exit(1);
  }

  printf("k = %d\n", k_final);

  if ( (pid=fork()) == -1 ) {
    perror("fork");
    exit(1);
  }

	if(pid) {
    // FATHER
    struct sched_param child_params;
    int ret;
    child_params.sched_priority = PRIORITY_C;
    ret = sched_setscheduler(pid, SCHED_FIFO, &child_params);
    if (ret == -1) {
      perror("sched_setscheduler child");
      exit(1);
    }
  }

  else {
    // CHILD

  }



  return 0;
}
