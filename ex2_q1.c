#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>

#define SIG SIGRTMIN
#define TIMER 4
#define PRIORITY_F 3
#define PRIORITY_C 2
#define CHARGE_F 2
#define CHARGE_C 4
#define PERIODE_F 4
#define PERIODE_C 6
#define UNIT 1000000


int pid;
int k_final = 0;
int compteur = 0;
int charge = 0;
timer_t created_timer;
struct timespec start_time;
struct timespec end_time;
struct timespec exec;


static void alarm_handler() {

  if (pid) {
   wait(NULL);
   printf("[T1] %d éxécutions\n", compteur);
 }
 else {
   printf("[T2] %d éxécutions\n", compteur);
 }

 if (timer_delete(created_timer) == -1) {
   perror("timer_delete");
   exit(1);
 }
  exit(0);
}

static void handler()
{
  int i = 0;
  clock_gettime(TIMER_ABSTIME, &start_time);
  if (pid) {
    printf(" * Start T1 %ld s + %ld ns\n", start_time.tv_sec, start_time.tv_nsec);
  }
  else {
    printf("\t * Start T2 %ld s + %ld ns\n", start_time.tv_sec, start_time.tv_nsec);
  }
  while (i<k_final*charge) {
    clock_gettime(TIMER_ABSTIME, &exec);
    i++;
  }
  clock_gettime(TIMER_ABSTIME, &end_time);

  if (pid) {
    printf("   End T1 %ld s + %ld ns\n", end_time.tv_sec, end_time.tv_nsec);
    printf("   Duration T1 %ld s + %ld ns\n",
      end_time.tv_sec - start_time.tv_sec,
      end_time.tv_nsec - start_time.tv_nsec);
  }

  else {
    printf("\t   End T2 %ld s + %ld ns\n", end_time.tv_sec, end_time.tv_nsec);
    printf("\t   Duration T2 %ld s + %ld ns\n",
      end_time.tv_sec - start_time.tv_sec,
      end_time.tv_nsec - start_time.tv_nsec);
  }
  fflush(stdout);
  compteur++;
}

int calibrage(int tours) {
  int i = tours;
  struct timespec ref;
  clock_gettime(CLOCK_REALTIME, &ref);
  double t = (double)ref.tv_sec*1e9 + (double)ref.tv_nsec;
  int cal = 0;
  while (((double)ref.tv_sec*1e9 + (double)ref.tv_nsec)<t+1e6) {
    clock_gettime(CLOCK_REALTIME, &ref);
    cal++;
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
    if (k<cal)
      cal = k;
    i--;
  }
  return cal;
}


int main(int argc, char const *argv[])
{
  struct sched_param father_params;
  int ret;
  struct itimerspec new_setting;
  int period;
  struct sigaction sa, s_alarm;
  struct sigevent sev;
  k_final = calibrage(0);

  father_params.sched_priority = PRIORITY_F;
  ret = sched_setscheduler(getpid(), SCHED_FIFO, &father_params);
    perror("sched_setscheduler father");
    exit(1);
  }

  // On prepare un handler

	sa.sa_sigaction = handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIG, &sa, NULL)) {
		perror("SIG");
		exit(1);
	}

	s_alarm.sa_sigaction = alarm_handler;
	sigemptyset (&s_alarm.sa_mask);
	s_alarm.sa_flags = 0;
  if (sigaction(SIGALRM, &s_alarm, NULL)) {
		perror("SIGALRM");
		exit(1);
	}

  if ( (pid=fork()) == -1 ) {
    perror("fork");
    exit(1);
  }

	if(pid) {
    // FATHER
    printf("Calibrage %d\n",k_final);
    charge = CHARGE_F;
    period = PERIODE_F;

    struct sched_param child_params;
    child_params.sched_priority = PRIORITY_C;
    ret = sched_setscheduler(pid, SCHED_FIFO, &child_params);
    if (ret == -1) {
      perror("sched_setscheduler child");
      exit(1);
    }

    sev.sigev_signo = SIG;
  }

  else {
    // CHILD
    charge = CHARGE_C;
    period = PERIODE_C;
  }

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_value.sival_ptr = &created_timer;
  sev.sigev_signo = SIG;
  /* ID du timer créé */
  ret = timer_create(TIMER_ABSTIME, &sev, &created_timer);
  if (ret == -1) {
    perror("timer_create");
    exit(1);
  }

  new_setting.it_value.tv_sec = 0;
  new_setting.it_value.tv_nsec = period * UNIT;
  new_setting.it_interval.tv_sec = 0;
  new_setting.it_interval.tv_nsec = new_setting.it_value.tv_nsec;

  alarm(TIMER);
  ret = timer_settime(created_timer, 0, &new_setting, NULL);
  if (ret == -1) {
    perror("timer_settime");
    exit(1);
  }

  while (1) {
    sigsuspend(&sa.sa_mask);
  }

  return 0;
}
