#ifndef MMSOUND_H
#define MMSOUND_H

#include <string>

/** defines for the different sound systems */
#define MMSL_NONE    0
#define MMSL_SPEAKER 1
#define MMSL_ALSA    2
#define MMSL_OSS     3

extern bool mmslInitSoundSystem(int system, const std::string &device="default");
extern void mmslCloseSoundSystem();

extern bool mmslSoundSystemAvailable(int system);

extern void mmslSetAttack(int attack);
extern void mmslSetFrequency(int frequency);

extern void mmslPlayTone(unsigned int duration);
extern void mmslPlayPause(unsigned int duration);
extern void mmslPlayErrorTone(unsigned int dotLength);

#endif

