#include "mmsound.h"
#include "beep.h"
#include "alarm.h"

#ifdef HAVE_PORTAUDIO
#include <cmath>
#include <portaudio.h>
#endif

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

static int mmslFrequency = 800;
static int mmslSmoothen = 0;
static int mmslSystem = MMSL_NONE;

#ifdef HAVE_PORTAUDIO
// Init PortAudio device with ...
#define SAMPLE_RATE         44100
#define FRAMES_PER_BUFFER   1024
#define CHANNELS            2

 /* stereo output buffer */
float buffer[FRAMES_PER_BUFFER][2];
PaStreamParameters outputParameters;
PaStream *stream;
PaError err;

// Length of our complete rendering buffer
#define BUF_LEN 8 * 44100
// Rendering buffer for output to PortAudio
float g_buffer[BUF_LEN];

bool initPortAudio(const std::string& device)
{
    err = Pa_Initialize();
    if (err != paNoError)
    {
      cerr << "Error: Couldn't initialize PortAudio.\n" << endl;
      return false;
    }

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice)
    {
      cerr << "Error: No default output device for PortAudio.\n" << endl;
      return false;
    }
    outputParameters.channelCount = CHANNELS;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = 0.050; // Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL); /* no callback, so no callback userData */
    if (err != paNoError)
    {
        cerr << "Error: Couldn't open stream for PortAudio.\n" << endl;
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        cerr << "Error: Couldn't start the stream for PortAudio.\n" << endl;
    }

    return true;
}

void playBufferPortAudio(unsigned int duration)
{
  unsigned long int nbSamples = SAMPLE_RATE * ((float) duration / 1000.0);
  if (nbSamples > 0)
  {
    // Sending the sound
    unsigned long int sample = 0;
    unsigned long int bufferCount = nbSamples / FRAMES_PER_BUFFER;
    unsigned long int i = 0;
    unsigned long int j = 0;

    for(; i < bufferCount; ++i)
    {
      for(j=0; j < FRAMES_PER_BUFFER; ++j)
      {
          buffer[j][0] = g_buffer[sample];  /* left */
          buffer[j][1] = g_buffer[sample];  /* right */
          ++sample;
      }

      Pa_WriteStream( stream, buffer, FRAMES_PER_BUFFER );
    }

    // Send remainder of samples
    unsigned long int remaining = nbSamples - sample;
    if (remaining > 0)
    {
      for(j=0; j < remaining; ++j)
      {
          buffer[j][0] = g_buffer[sample];  /* left */
          buffer[j][1] = g_buffer[sample];  /* right */
          ++sample;
      }
      Pa_WriteStream(stream, buffer, remaining);
    }
  }
}

void renderFrequencyToBufferPortAudio(int frequency)
{
    float t = 2*M_PI*frequency/(SAMPLE_RATE * CHANNELS);
    for (int i=0; i < BUF_LEN; ++i) {
        g_buffer[i] = sin(t*i);
    }
}

//
// Smoothstep functions (generalized form), derived from
// https://en.wikipedia.org/wiki/Smoothstep
//

// Returns binomial coefficient without explicit use of factorials,
// which can't be used with negative integers
unsigned long int pascalTriangle(unsigned int a, unsigned int b)
{
  unsigned long int result = 1; 
  for (unsigned long int i = 0; i < b; ++i)
    result *= (a - i) / (i + 1);
  return result;
}

// Generalized smoothstep
float generalSmoothStep(unsigned int N, float x) 
{
  if (x < 0.0)
    return 0.0;
  if (x >= 1.0)
    return 1.0;
  float result = 0;
  for (unsigned int n = 0; n <= N; ++n)
    result += pascalTriangle(-N - 1, n) *
              pascalTriangle(2 * N + 1, N - n) *
              pow(x, N + n + 1);
  return result;
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      if (initPortAudio(device))
      {
        mmslSystem = MMSL_PORTAUDIO;
        return true;  
      }
#endif
      cerr << "mmslInitSoundSystem: PortAudio support is not available!" << endl;
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      Pa_StopStream(stream);
      Pa_CloseStream(stream);
      Pa_Terminate();
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
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

void mmslSetSmoothening(int smoothen)
{
  mmslSmoothen = smoothen;
}

void mmslSetFrequency(int frequency)
{
  switch (mmslSystem)
  {
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      renderFrequencyToBufferPortAudio(frequency);
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      playBufferPortAudio(duration);
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
    case MMSL_PORTAUDIO:
      AlarmWait();
      AlarmSet(duration);
      AlarmWait();
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
#endif
      break;
  }
}

