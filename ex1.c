#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#define SIG SIGRTMIN
#define TIMER 30
#define PRIORITY_F 17
#define COUNT_LIMIT 10

int compteur;

void notify() {
  compteur++;
	printf("Period nb %d just finished\n", compteur);
}

void alarm_handler() {
	exit(0);
}

int main(int argc, char const *argv[]) {

  struct sched_param scheduling_parameters;
  int ret;
  scheduling_parameters.sched_priority = PRIORITY_F;
  ret = sched_setscheduler(getpid(), SCHED_FIFO, &scheduling_parameters);
  if (ret == -1) {
    perror("sched_setscheduler");
    exit(1);
  }

  struct sigaction sa;
  sa.sa_handler = alarm_handler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGALRM, &sa, NULL)) {
    perror("alarm");
    exit(1);
  }

  struct sigaction sa1;
	sa1.sa_sigaction = notify;
	sigemptyset (&sa1.sa_mask);
	sa1.sa_flags = 0;
	if (sigaction(SIG, &sa1, NULL)) {
		perror("SIGRTMIN");
		exit(1);
	}

  struct sigevent sev;
  timer_t created_timer;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG;
  sev.sigev_value.sival_ptr = &created_timer;
  /* ID du timer créé */
  ret = timer_create(CLOCK_REALTIME, NULL, &created_timer);
  if (ret == -1) {
    perror("timer_create");
    exit(1);
  }

  struct itimerspec new_setting, old_setting;
  new_setting.it_value.tv_sec = 1;
  /* démarage après 1 sec. */
  new_setting.it_value.tv_nsec = 0;
  new_setting.it_interval.tv_sec = 0;
  new_setting.it_interval.tv_nsec = 1000000;
  /* expiration toutes les
  1 000 000 nanosec. */
  ret = timer_settime(created_timer, 0, &new_setting, &old_setting);
  if (ret == -1) {
    perror("timer_settime");
    exit(1);
  }
  //alarm(TIMER);
  while(compteur < COUNT_LIMIT) {

  }
  return 0;
}
