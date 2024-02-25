#ifndef MMSOUND_H
#define MMSOUND_H

#include <string>

/** defines for the different sound systems */
#define MMSL_NONE         0
#define MMSL_SPEAKER      1
#define MMSL_PORTAUDIO    2

extern bool mmslInitSoundSystem(int system, const std::string &device="default");
extern void mmslCloseSoundSystem();

extern bool mmslSoundSystemAvailable(int system);

extern void mmslSetSmoothening(int smoothen);
extern void mmslSetFrequency(int frequency);

extern void mmslPlayTone(unsigned int duration);
extern void mmslPlayPause(unsigned int duration);
extern void mmslPlayErrorTone(unsigned int dotLength);

#endif

