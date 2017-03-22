#include <unistd.h>
#include <stdio.h>
//#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
//#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#define SIG_F SIGRTMIN+1
#define SIG_C SIGRTMIN+2
#define TIMER 4
#define PRIORITY_F 3
#define PRIORITY_C 2
#define CHARGE_F 2
#define CHARGE_C 4


int pid;
int k_final = 0;
int compteur = 0;
int charge = 0;
struct timespec start_time;
struct timespec end_time;

static void handler(int sig, siginfo_t *si, void *uc)
{
  if (sig == SIGALRM && pid != 0) {
    kill(pid,SIGUSR2);
  	wait(NULL);
  	exit(0);
  }
  if (sig == SIGUSR2 && pid == 0) {
    exit(0);
  }

  int i = 0;
  clock_gettime(TIMER_ABSTIME, &start_time);
  if (pid) {
    printf("Start T1 %ld s + %ld ns\n", start_time.tv_sec, start_time.tv_nsec);
  }
  else {
    printf("\tStart T2 %ld s + %ld ns\n", start_time.tv_sec, start_time.tv_nsec);
  }
  while (i<k_final*charge) {
    clock_gettime(TIMER_ABSTIME, NULL);
    i++;
  }
  clock_gettime(TIMER_ABSTIME, &end_time);

  if (pid) {
    printf("End T1 %ld s + %ld ns\n", end_time.tv_sec, end_time.tv_nsec);
    printf("Duration T1 %ld ns\n", end_time.tv_nsec - start_time.tv_nsec);
  }

  else {

    printf("\tEnd T2 %ld s + %ld ns\n", end_time.tv_sec, end_time.tv_nsec);
    printf("\tDuration T2 %ld ns\n", end_time.tv_nsec - start_time.tv_nsec);
  }
}

int calibrage(int tours) {
  int i = tours;
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
    clock_gettime(CLOCK_REALTIME, &ref);
    t = (double)ref.tv_sec*1e9 + (double)ref.tv_nsec;
    k = 0;
    while (((double)ref.tv_sec*1e9 + (double)ref.tv_nsec)<t+1e6) {
      clock_gettime(CLOCK_REALTIME, &ref);
      k++;
    }
    if (k<k_final)
      k_final = k;
    i--;
  }
  return k_final;
}


int main(int argc, char const *argv[])
{
  int k = calibrage(9);
  printf("Calibrage %d\n",k);

  struct sched_param father_params;
  int ret;
  father_params.sched_priority = PRIORITY_F;
  ret = sched_setscheduler(getpid(), SCHED_FIFO, &father_params);
  if (ret == -1) {
    perror("sched_setscheduler father");
    exit(1);
  }

  // On bloque les deux signaux qu'on utilisera
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIG_C);
  sigaddset(&mask, SIG_F);
  if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1) {
    perror("sigprocmask");
    exit(1);
  }

  // On prepare un handler
  struct sigaction sa;
	sa.sa_sigaction = handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIG_F, &sa, NULL)) {
		perror("SIG_F");
		exit(1);
	}
  if (sigaction(SIG_C, &sa, NULL)) {
		perror("SIG_C");
		exit(1);
	}
  if (sigaction(SIGALRM, &sa, NULL)) {
		perror("SIGALRM");
		exit(1);
	}
  if (sigaction(SIGUSR2, &sa, NULL)) {
		perror("SIGUSR2");
		exit(1);
	}

  struct itimerspec new_setting, old_setting;
  new_setting.it_value.tv_sec = 1;
  /* démarage après 1 sec. */
  new_setting.it_value.tv_nsec = 0;
  new_setting.it_interval.tv_sec = 0;
  // expiration toutes les s
  new_setting.it_interval.tv_nsec = 1000;

  if ( (pid=fork()) == -1 ) {
    perror("fork");
    exit(1);
  }

	if(pid) {
    // FATHER
    charge = CHARGE_F;
    struct sched_param child_params;
    int ret;
    child_params.sched_priority = PRIORITY_C;
    ret = sched_setscheduler(pid, SCHED_FIFO, &child_params);
    if (ret == -1) {
      perror("sched_setscheduler child");
      exit(1);
    }

    struct sigevent sev;
    timer_t created_timer;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG_F;
    sev.sigev_value.sival_ptr = &created_timer;
    /* ID du timer créé */
    ret = timer_create(TIMER_ABSTIME, &sev, &created_timer);
    if (ret == -1) {
      perror("timer_create");
      exit(1);
    }

    ret = timer_settime(created_timer, 0, &new_setting, &old_setting);
    if (ret == -1) {
      perror("timer_settime");
      exit(1);
    }

    sigaddset(&sa.sa_mask, SIG_C);
    alarm(TIMER);
    while (1) {
      sigsuspend(&sa.sa_mask);
    }

    if (timer_delete(created_timer) == -1) {
      perror("timer_delete");
      exit(1);
    }

  }

  else {
    // CHILD
    charge = CHARGE_C;
    struct sigevent sev;
    timer_t created_timer;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG_C;
    sev.sigev_value.sival_ptr = &created_timer;
    /* ID du timer créé */
    ret = timer_create(TIMER_ABSTIME, &sev, &created_timer);
    if (ret == -1) {
      perror("timer_create");
      exit(1);
    }


    ret = timer_settime(created_timer, 0, &new_setting, &old_setting);
    if (ret == -1) {
      perror("timer_settime");
      exit(1);
    }

    sigaddset(&sa.sa_mask, SIG_F);
    while (1) {
      sigsuspend(&sa.sa_mask);
    }

    if (timer_delete(created_timer) == -1) {
      perror("timer_delete");
      exit(1);
    }
  }

  return 0;
}
