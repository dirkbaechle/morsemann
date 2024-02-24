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
// Length of our complete buffer
#define BUF_LEN 8 * 48000
// Buffer for output to Alsa
float g_buffer[BUF_LEN];
// Our output device
snd_pcm_t *pcm_handle;
int channels =1;
snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT;
int rate = 48000;

bool initAlsa(const std::string& device)
{
      string pcm_device(device);
      if (pcm_device.empty())
      {
        pcm_device = "default";
      }

   int err;

    if ((err = snd_pcm_open(&pcm_handle, pcm_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
      cerr << "Playback open error: " <<  snd_strerror(err) << endl;
      return false;
    }

    if ((err = snd_pcm_set_params(pcm_handle,
        format,
        // SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        channels,
        // BUF_LEN,
        rate,
        1, /* period */
        500000)) < 0) {	 /* latency: 0.5sec */ 
      cerr << "Playback open error: " << snd_strerror(err) << endl;
      return false;
    }
    return true;
}

void playBufferAlsa(unsigned int duration)
{
    int nbSamples = rate * channels * ((float) duration / 1000.0);
    if (nbSamples >0) {
      // Sending the sound
      snd_pcm_writei(pcm_handle, g_buffer, nbSamples);
      snd_pcm_drain(pcm_handle);
    }
}

void renderFrequencyToBuffer(int frequency)
{
    float t = 2*M_PI*frequency/(rate*channels);
    for (int i=0; i< BUF_LEN; ++i) {
        g_buffer[i] = sin(t*i);
    }
}

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
      if (BeepInit () != 0)
      {
        cerr << "BeepInit: Can't access speaker!" << endl;
        return false;
      }
      mmslSystem = MMSL_SPEAKER;

      Beep(100, 0, 800);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      if (initAlsa(device))
      {
        mmslSystem = MMSL_ALSA;
        return true;  
      }
#endif
      cerr << "mmslInitSoundSystem: ALSA support is not available!" << endl;
      return false;
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
        BeepCleanup();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      if (pcm_handle)
      {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
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
      renderFrequencyToBuffer(frequency);
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
      Beep((int) duration, 10, mmslFrequency);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      playBufferAlsa(duration);
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
      Beep((int) duration, 0, mmslFrequency);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      Beep((int) duration, 0, mmslFrequency);
      BeepWait();
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
      Beep(dotLength, 10, 700);
      BeepWait();
      Beep(dotLength, 10, 600);
      BeepWait();
      Beep(dotLength, 10, 500);
      BeepWait();
      Beep(dotLength*3, 0, 500);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
#endif
      break;
  }
}

