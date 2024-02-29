/* alarm.c -- seligman 6/92 */

/*
-- Implementation of alarm.h
*/

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

static int alarmPending = 0;  /* Nonzero when the alarm is set. */

/* Default BSDSIGS for backwards compatibility of makefile */
#ifndef BSDSIGS
#ifndef POSIXSIGS
#ifndef LINUXSIGS
#ifndef OTHERSIGS
#define BSDSIGS
#endif
#endif
#endif
#endif

static int ualarm(unsigned int us)
{
    struct itimerval rttimer, old_rttimer;

    rttimer.it_value.tv_sec  = us / 1000000;
    rttimer.it_value.tv_usec = us % 1000000;
    rttimer.it_interval.tv_sec  = 0;
    rttimer.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &rttimer, &old_rttimer)) 
        return 1;

    return 0;
}


#ifdef __hpux

static void AlarmHandler(int sig, 
                         int code, 
                         struct sigcontext *scp)
{
    alarmPending = 0;
    /* Prevent alarm signal from interrupting any pending read. */
    if (scp->sc_syscall == SYS_READ)
	scp->sc_syscall_action = SIG_RESTART;
}

#else

#ifndef LINUXSIGS
static void AlarmHandler()
{
    alarmPending = 0;
}
#else
#ifdef USE_SIGINFO
static void AlarmHandler(int /* sig */, siginfo_t * /* siginfo */, void * /* context */)
{
    alarmPending = 0;
}
#else
static void AlarmHandler(int /* sig */)
{
    alarmPending = 0;
}
#endif /*USE_SIGINFO*/
#endif /*LINUXSIGS*/

#endif /* __hpux */


int AlarmSet(int time)
{
#ifdef LINUXSIGS
  struct sigaction sigact;
#endif

    alarmPending = 1;
#ifdef BSDSIGS
    signal(SIGALRM, AlarmHandler);
#else
#ifdef POSIXSIGS
    sigset(SIGALRM, AlarmHandler);
#else
#ifdef LINUXSIGS
#ifdef USE_SIGINFO
    sigact.sa_sigaction = AlarmHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGALRM, &sigact, NULL);
#else
    sigact.sa_handler = AlarmHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGALRM, &sigact, NULL);
#endif /*USE_SIGINFO*/
#else
    /* You're on your own... */
#endif /*LINUXSIGS*/
#endif /*POSIXSIGS*/
#endif /*BSDSIGS*/
   return ualarm(1000 * time);
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
#ifdef LINUXSIGS
    sigset_t savemask;
    sigset_t blockmask;
    sigemptyset(&blockmask);
    sigaddset(&blockmask, SIGALRM);
    sigprocmask(SIG_SETMASK, (const sigset_t *) 0, &savemask);
    sigprocmask(SIG_BLOCK, (const sigset_t *) &blockmask, &savemask);
    if (alarmPending)
    {
      sigemptyset(&blockmask);
      sigsuspend(&blockmask);
    }
    sigprocmask(SIG_SETMASK, (const sigset_t *) &savemask, (sigset_t *) 0);
#else
    /* You're on your own */
#endif /*LINUXSIGS*/
#endif /*POSIXSIGS*/
#endif /*BSDSIGS*/

}

