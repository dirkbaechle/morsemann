#include "mmsound.h"
#include "beep.h"
#ifdef HAVE_ALSA
#include <cmath>
#include <alsa/asoundlib.h>
#endif

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

static int mmslFrequency = 800;
static int mmslAttack = 0;
static int mmslSystem = MMSL_NONE;

#ifdef HAVE_ALSA
/// Requested sample rate
static int pcm_rate = 44100;
/// Sample rate actually used/supported
static unsigned int pcm_exact_rate;
/// Number of channels (1=mono, 2=stereo)
static int pcm_channels = 2;
/// Handle to the ALSA sound device
static snd_pcm_t *pcm_handle = NULL;
/// Pointer to the hardware param struct
static snd_pcm_hw_params_t *pcm_params = NULL;
/// Number of frames for the sound device
static snd_pcm_uframes_t pcm_frames = 0;
/// Number of periods
static int pcm_periods = 2;
/// Size of the period
static snd_pcm_uframes_t pcm_periodsize = 4096;

/// Size of the replay window that we move over our sine buffer
static int pcm_replay_buffersize = 2;
/// Size of the PCM buffer, where the actual sine/pause tone is stored
static long int pcm_buffersize = 32000;

/// Pointer to the sine sound array
unsigned char *pcm_sound;
/// Pointer to the pause sound array
unsigned char *pcm_pause;

#endif

bool mmslInitSoundSystem(int system, const std::string &device)
{
  if (mmslSystem != MMSL_NONE)
  {
    mmslCloseSoundSystem();
  }

  switch (system)
  {
    case MMSL_SPEAKER:
#ifndef DOS
      if (BeepInit () != 0)
      {
        cerr << "BeepInit: Can't access speaker!" << endl;
        return false;
      }

      Beep(100, 0, 800);
      BeepWait();
#endif
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      string pcm_device(device);
      if (pcm_device.empty())
      {
        pcm_device = "default";
      }
      /* Open the PCM device in playback mode */
      if (pcm = snd_pcm_open(&pcm_handle, pcm_device.c_str(),
              SND_PCM_STREAM_PLAYBACK, 0) < 0)
      {
        cerr << "ERROR: Can't open '" << pcm_device << "' PCM device. ";
        cerr << snd_strerror(pcm) << endl;
        return false;
      }
      /* Allocate parameters object and fill it with default values*/
      snd_pcm_hw_params_alloca(&pcm_params);

      snd_pcm_hw_params_any(pcm_handle, pcm_params);

      /* Set parameters */
      if (pcm = snd_pcm_hw_params_set_access(pcm_handle, pcm_params,
              SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
        cerr << "ERROR: Can't set interleaved mode. " << snd_strerror(pcm) << endl;

      if (pcm = snd_pcm_hw_params_set_format(pcm_handle, pcm_params,
                SND_PCM_FORMAT_S16_LE) < 0)
        cerr << "ERROR: Can't set format. " << snd_strerror(pcm) << endl;

      if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, pcm_params, pcm_channels) < 0)
        cerr << "ERROR: Can't set channels number." << snd_strerror(pcm) << endl;

      if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, pcm_params, &pcm_rate, 0) < 0)
        cerr << "ERROR: Can't set rate. " << snd_strerror(pcm) << endl;

      /* Write parameters */
      if (pcm = snd_pcm_hw_params(pcm_handle, pcm_params) < 0)
        cerr << "ERROR: Can't set hardware parameters. " << snd_strerror(pcm) << endl;

      /* Read out parameters */
      snd_pcm_hw_params_get_channels(pcm_params, &pcm_channels);
      snd_pcm_hw_params_get_rate(pcm_params, &pcm_rate, 0);

      /* Allocate buffer to hold single period */
      snd_pcm_hw_params_get_period_size(pcm_params, &pcm_frames, 0);
      snd_pcm_hw_params_get_period_size_min(pcm_params, &pcm_min_frames, NULL);

      /// TODO: optimize frame number in range [pcm_min_frames; pcm_frames]
      pcm_replay_buffersize = pcm_frames * pcm_channels * 2 /* 2 -> sample size */;
      pcm_sound = (unsigned char *) malloc(pcm_buffersize);
      pcm_pause = (unsigned char *) malloc(pcm_buffersize);

      snd_pcm_hw_params_get_period_time(params, &pcm_period_time, NULL);

      for (loops = (dotLength * 1000) / pcm_period_time; loops > 0; loops--) {

        if (pcm = read(0, buff, pcm_replay_buffersize) == 0) {
          printf("Early end of file.\n");
          return 0;
        }

        if (pcm = snd_pcm_writei(pcm_handle, buff, frames) == -EPIPE) {
          printf("XRUN.\n");
          snd_pcm_prepare(pcm_handle);
        } else if (pcm < 0) {
          printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
        }

      }

      snd_pcm_drain(pcm_handle);
      snd_pcm_close(pcm_handle);
      free(buff);




#else
      cerr << "mmslInitSoundSystem: ALSA support is not available!" << endl;
      return false;
#endif
      break;
    default:
      break;
  }

  return true;
}

void mmslCloseSoundSystem()
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
#ifndef DOS
        BeepCleanup();
#endif
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      if (pcm_handle)
      {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
      }
      if (pcm_sound)
      {
        free(pcm_sound);
        pcm_sound = NULL;
      }
      if (pcm_pause)
      {
        free(pcm_pause);
        pcm_pause = NULL;
      }
#endif
      break;
    default:
      break;
  }
}

bool mmslSoundSystemAvailable(int system)
{
  switch (system)
  {
    case MMSL_SPEAKER:
      return true;
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      return true;
#else
      return false;
#endif
      break;
    default:
      break;
  }
  return false;
}

void mmslSetAttack(int attack)
{
  mmslAttack = attack;
}

void mmslSetFrequency(int frequency)
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
#ifdef HAVE_ALSA
#endif
      break;
    default:
      break;
  }
  mmslFrequency = frequency;
}

void mmslPlayTone(unsigned int duration)
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
#ifdef DOS
      sound(mmslFrequency);
      delay((int) duration);
#else
      Beep((int) duration, 10, mmslFrequency);
      BeepWait();
#endif
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
#endif
      break;
    default:
      break;
  }
}

/** Erzeugt eine Pause von \a duration Millisekunden.
@param duration Anzahl der Millisekunden
*/
void mmslPlayPause(unsigned int duration)
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
#ifdef DOS
      nosound();
      delay((int) duration);
#else
      Beep((int) duration, 0, mmslFrequency);
      BeepWait();
#endif
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
#endif
      break;
    default:
      break;
  }
}

void mmslPlayErrorTone(unsigned int dotLength)
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
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
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
#endif
      break;
  }
}

