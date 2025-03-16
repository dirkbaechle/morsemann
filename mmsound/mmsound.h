#ifndef MMSOUND_H
#define MMSOUND_H

#include <string>

/** defines for the different sound systems */
#define MMSL_NONE    0
#define MMSL_SPEAKER 1
#define MMSL_ALSA    2

extern bool mmslInitSoundSystem(int system, const std::string &device="default");
extern void mmslPrepareSoundStream();
extern void mmslDrainSoundStream();
extern void mmslCloseSoundSystem();

extern bool mmslSoundSystemAvailable(int system);

extern void mmslSetBpm(unsigned int bpm);
extern unsigned int mmslGetBpm();
extern void mmslSetDelayFactor(unsigned int factor);
extern unsigned int mmslGetDelayFactor();
extern void mmslSetSmoothening(int smoothen);
extern void mmslSetFrequency(int frequency);

extern void mmslPlayTone(unsigned int duration);
extern void mmslPlayPause(unsigned int duration);
extern void mmslPlayToneDits(unsigned int dits);
extern void mmslPlayPauseDits(unsigned int dits);
extern void mmslPlayPauseWord();
extern void mmslPlayErrorTone();
extern int mmslMorseChar(int signID);
extern int mmslMorseWord(const std::string &msg);

#endif
