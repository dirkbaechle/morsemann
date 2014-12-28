#include "mmsound.h"
#include "beep.h"

#include <stdio.h>

static int mmslFrequency = 800;
static int mmslAttack = 0;

int mmslInitSoundSystem()
{
#ifdef DOS
  ;
#else
  if (BeepInit () != 0)
  {
    fprintf (stderr, "BeepInit: Can't access speaker!\n");
    return MMSL_ERROR;
  }

  Beep(100, 0, 800);
  BeepWait();

  return MMSL_SUCCESS;
#endif
}

void mmslCloseSoundSystem()
{
#ifdef DOS
  ;
#else
  BeepCleanup();
#endif
}

int mmslSoundSystemAvailable(int system)
{
  return MMSL_YES;
}

void mmslSetAttack(int attack)
{
  mmslAttack = attack;
}

void mmslSetFrequency(int frequency)
{
  mmslFrequency = frequency;
}

void mmslPlayTone(unsigned int duration)
{
#ifdef DOS
  sound(mmslFrequency);
  delay((int) duration);
#else
  Beep((int) duration, 10, mmslFrequency);
  BeepWait();
#endif
}

/** Erzeugt eine Pause von \a duration Millisekunden.
@param duration Anzahl der Millisekunden
*/
void mmslPlayPause(unsigned int duration)
{
#ifdef DOS
  nosound();
  delay((int) duration);
#else
  Beep((int) duration, 0, mmslFrequency);
  BeepWait();
#endif
}

void mmslPlayErrorTone(unsigned int dotLength)
{
#ifdef DOS
  sound(700);
  delay((int) dotLength);
  sound(600);
  delay((int) dotLength);
  sound(500);
  delay((int) dotLength);
  nosound();
  delay((int) 3*dotLength);
#else
  Beep(dotLength, 10, 700);
  BeepWait();
  Beep(dotLength, 10, 600);
  BeepWait();
  Beep(dotLength, 10, 500);
  BeepWait();
  Beep(dotLength*3, 0, 500);
  BeepWait();
#endif
}

