#ifndef MMSOUND_H
#define MMSOUND_H

#define MMSL_YES 0
#define MMSL_NO  1

#define MMSL_SUCCESS 0
#define MMSL_ERROR   1

/** defines for the different sound systems */
#define MMSL_SPEAKER 0
#define MMSL_ALSA    1
#define MMSL_OSS     2

extern int mmslInitSoundSystem();
extern void mmslCloseSoundSystem();

extern int mmslSoundSystemAvailable(int system);

extern void mmslSetAttack(int attack);
extern void mmslSetFrequency(int frequency);

extern void mmslPlayTone(unsigned int duration);
extern void mmslPlayPause(unsigned int duration);
extern void mmslPlayErrorTone(unsigned int dotLength);

#endif

