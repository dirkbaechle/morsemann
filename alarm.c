/* alarm.c -- seligman 6/92 */

/*
-- Implementation of alarm.h
*/

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

static int alarmPending = 0;  /* Nonzero when the alarm is set. */

static void ualarm();
static void AlarmHandler();

/* Default BSDSIGS for backwards compatibility of makefile */
#ifndef BSDSIGS
#ifndef POSIXSIGS
#ifndef OTHERSIGS
#define BSDSIGS
#endif
#endif
#endif

void AlarmSet(time)
    int time;
{
    alarmPending = 1;
#ifdef BSDSIGS
    signal(SIGALRM, AlarmHandler);
#else
#ifdef POSIXSIGS
    sigset(SIGALRM, AlarmHandler);
#else
    /* You're on your own... */
# endif /*POSIXSIGS*/
#endif /*BSDSIGS*/
    ualarm(1000 * time, 0);
}


/*
-- If an alarm signal is lurking (due to a prior call to SetAlarm), then
-- pause until it arrives.  This procedure could have simply been written:
--   if (alarmPending) pause();
-- but that allows a potential race condition.
*/
void AlarmWait()
{
#ifdef BSDSIGS
    long savemask = sigblock(sigmask(SIGALRM));
    if (alarmPending)
        sigpause(savemask);
    sigsetmask(savemask);
#else
#ifdef POSIXSIGS
    sighold(SIGALRM);
    if (alarmPending)
      sigpause(SIGALRM);
    sigrelse(SIGALRM);
#else
    /* You're on your own */
#endif /*POSIXSIGS*/
#endif /*BSDSIGS*/

}


static void ualarm(us)
    unsigned us;
{
    struct itimerval rttimer, old_rttimer;

    rttimer.it_value.tv_sec  = us / 1000000;
    rttimer.it_value.tv_usec = us % 1000000;
    rttimer.it_interval.tv_sec  = 0;
    rttimer.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &rttimer, &old_rttimer)) {
	perror("ualarm");
	exit(1);
    }
}


#ifdef __hpux

static void AlarmHandler(sig, code, scp)
    int sig;
    int code;
    struct sigcontext *scp;
{
    alarmPending = 0;
    /* Prevent alarm signal from interrupting any pending read. */
    if (scp->sc_syscall == SYS_READ)
	scp->sc_syscall_action = SIG_RESTART;
}

#else

static void AlarmHandler()
{
    alarmPending = 0;
}

#endif __hpux
