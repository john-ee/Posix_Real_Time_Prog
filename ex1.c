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
#define COUNT_LIMIT 30

int compteur = 0;
timer_t created_timer;
struct timespec current_time;
struct timespec start_time;

void notify() {
  clock_gettime(CLOCK_REALTIME, &current_time);
  //char * readable = ctime(&current_time.tv_sec);

	printf("[Period %d]\tT = %ld s + %ld ns\n", compteur,
    current_time.tv_sec - start_time.tv_sec,
    current_time.tv_nsec - start_time.tv_nsec);
  compteur++;
}


int main(int argc, char const *argv[]) {

  sigset_t mask;
  printf("Blocking signal %d\n", SIG);
  sigemptyset(&mask);
  sigaddset(&mask, SIG);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  struct sched_param scheduling_parameters;
  int ret;
  scheduling_parameters.sched_priority = PRIORITY_F;
  ret = sched_setscheduler(getpid(), SCHED_FIFO, &scheduling_parameters);
  if (ret == -1) {
    perror("sched_setscheduler");
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
  ret = timer_create(CLOCK_REALTIME, &sev, &created_timer);
  if (ret == -1) {
    perror("timer_create");
    exit(1);
  }

  struct itimerspec new_setting, old_setting;
  new_setting.it_value.tv_sec = 1;
  /* démarage après 1 sec. */
  new_setting.it_value.tv_nsec = 0;
  new_setting.it_interval.tv_sec = 1;
  // expiration toutes les s
  new_setting.it_interval.tv_nsec = 0;
  ret = timer_settime(created_timer, 0, &new_setting, &old_setting);
  if (ret == -1) {
    perror("timer_settime");
    exit(1);
  }

  clock_gettime(CLOCK_REALTIME, &start_time);
  while(compteur < COUNT_LIMIT) {
    sigsuspend(&sa1.sa_mask);
  }

  if (timer_delete(created_timer) == -1) {
    perror("timer_delete");
    exit(1);
  }
  return 0;
}
