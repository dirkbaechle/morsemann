#include "mmsound.h"
#include "global.h"
#include <map>

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif
#ifdef HAVE_PORTAUDIO
#include <portaudio.h>
#endif
#ifdef HAVE_PULSEAUDIO
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

#include <cmath>
#include <functional>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;

/** Geschwindigkeit in Buchstaben pro Minute (bpm) */
static unsigned int mmslBpm = 60;
/** Länge eines Punktes in Millisekunden */
static unsigned int mmslDotLength = 100;
/** Pausenfaktor */
unsigned int mmslDelayFactor = 1;
/** Länge der Rampe für das Formen (Smoothing) der Morsezeichen in ms */
unsigned long int rampLength = 2;
/** Aufteilung/Position der Smoothing-Rampe (0 = hartes Timing, 1 = überhängend/Tiefpass) */
static int rampStrategy = 1;
/** Frequenz für die Ausgabe der Morsezeichen. */
static unsigned int mmslFrequency = 800;
/** Gewählte Funktion für das Formen (Smoothing) der Morsezeichen (0-2) */
static unsigned int mmslSmoothen = 3;
/** Gewähltes Sound-System für die Ausgabe der Morsezeichen (ALSA, PortAudio, Pulseaudio) */
static int mmslSystem = MMSL_NONE;


int channels = 1;
int samplerate = 48000;
// Length of our complete render buffer: 80 * 48000 = 3840000
#define BUF_LEN 3840000
// Render buffer for output to Alsa
unsigned short int g_buffer[BUF_LEN];

unsigned long int durationToSamples(unsigned long int msecDuration)
{
  unsigned long int nbSamples = samplerate * channels * (((float) msecDuration) / 1000.0);

  return nbSamples;
}

#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
#if defined(HAVE_ALSA) || defined(HAVE_PULSEAUDIO)
// Length of our audio buffer towards ALSA: 16*1024 = 16384
#define AUDIO_BUFFER_SIZE	16384
#else
#define AUDIO_BUFFER_SIZE	60
#endif
unsigned short int audio_buffer[AUDIO_BUFFER_SIZE];
#endif

#ifdef HAVE_ALSA
// Our output device
snd_pcm_t *pcm_handle = NULL;
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

bool initAlsa(const std::string& device)
{
  string pcm_device(device);
  if (pcm_device.empty())
  {
    pcm_device = "default";
  }

  int err;
  if ((err = snd_pcm_open(&pcm_handle, pcm_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    cerr << "MMSound ALSA open error: " << snd_strerror(err) << endl;
    return false;
  }

	if ((err = snd_pcm_set_params(pcm_handle,
				format,
				SND_PCM_ACCESS_RW_INTERLEAVED,
				channels,
				samplerate,
				1, /* period */
				500000)) < 0) {  /* latency: 0.5s */
		cerr << "MMSound ALSA error in snd_pcm_set_params: " << snd_strerror(err) << endl;
		return false;
	}

  // Ensure that all audio buffers are written before returning
  // from e.g. a drain (snd_pcm_drain()).
  snd_pcm_nonblock(pcm_handle, 0);
  return true;
}

void playBufferAlsa(unsigned long int nbSamples, bool sound)
{
  unsigned long int sample = 0;
  if (nbSamples > 0)
  {
    unsigned long int loops = nbSamples / AUDIO_BUFFER_SIZE;
    unsigned long int i = 0;
    unsigned long int j;
    for (; i < loops; ++i)
    {
      if (sound)
      {
        for (j = 0; j < AUDIO_BUFFER_SIZE; ++j)
        {
          audio_buffer[j] = g_buffer[sample++];
        }
      }
      else
      {
        for (j = 0; j < AUDIO_BUFFER_SIZE; ++j)
        {
          audio_buffer[j] = 0;
          ++sample;
        }
      }
      // Sending the sound
      snd_pcm_writei(pcm_handle, audio_buffer, AUDIO_BUFFER_SIZE);
    }
    unsigned long int remaining = nbSamples - sample;
    if (remaining > 0)
    {
      if (sound)
      {
        for (j = 0; j < remaining; ++j)
        {
          audio_buffer[j] = g_buffer[sample++];
        }
      }
      else
      {
        for (j = 0; j < remaining; ++j)
        {
          audio_buffer[j] = 0;
        }
      }
      // Sending the sound
      snd_pcm_writei(pcm_handle, audio_buffer, remaining);
    }
  }
}
#else
// Dummy Funktionen bereitstellen
bool initAlsa(const std::string& /*device*/)
{
	cerr << "MMSound ALSA not supported (HAVE_ALSA is not defined)" << endl;
  return false;
}

void playBufferAlsa(unsigned long int /*nbSamples*/, bool /*sound*/)
{
}
#endif

#ifdef HAVE_PORTAUDIO
typedef struct paPlayData
{
    int finished;
    unsigned long int totalFrames;
    unsigned long int position;
    bool playSound;

    void reset(unsigned long int frames, bool sound)
    {
      finished = 0;
      totalFrames = frames;
      position = 0;
      playSound = sound;
    }
}
paPlayData;

PaStreamParameters pa_outputParameters;
PaStream *pa_stream = nullptr;
PaError pa_err;
struct paPlayData playData;

/* Callback, wird von PortAudio aufgerufen um neue Frames zum Abspielen
** in die internen Buffer zu übertragen.
*/
static int paPlayCallback(const void */*inputBuffer*/,
                          void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* /*timeInfo*/,
                          PaStreamCallbackFlags /*statusFlags*/,
                          void *userData)
{
  struct paPlayData *data = (struct paPlayData*) userData;
  if (data->totalFrames == 0)
  {
    return paAbort;
  }

  short *out = (short*) outputBuffer;
  unsigned long i;

  for (i=0; i < framesPerBuffer; ++i)
  {
    if (data->playSound)
      *out++ = g_buffer[data->position + i];
    else
      *out++ = 0;
  }
  data->position += framesPerBuffer;

  if (framesPerBuffer <= data->totalFrames)
  {
    data->totalFrames -= framesPerBuffer;
  }
  else
  {
    data->totalFrames = 0;
  }
  return paContinue;
}

/*
 * Callback-Routine, wird aufgerufen wenn alle Frames abgespielt wurden.
 */
static void paStreamFinished(void* userData)
{
  struct paPlayData *data = (struct paPlayData *) userData;
  data->finished = 1;
}

bool initPortaudio(const std::string& device)
{
  pa_err = Pa_Initialize();
  if (pa_err != paNoError)
  {
    cerr << "MMSound PORTAUDIO open error: " << Pa_GetErrorText(pa_err) << endl;
    return false;
  }

  string pa_device(device);
  if (pa_device.empty() || (pa_device == "default"))
  {
    pa_outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (pa_outputParameters.device == paNoDevice)
    {
      cerr << "MMSound PORTAUDIO open error: no default output device." << endl;
      return false;
    }
  }
  else
  {
    // TODO Implement!!!
  }

  pa_outputParameters.channelCount = channels;
  pa_outputParameters.sampleFormat = paInt16; /* 16 bit integers (short, little endian) output */
  pa_outputParameters.suggestedLatency = 0.050; // Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  pa_outputParameters.hostApiSpecificStreamInfo = NULL;

  pa_err = Pa_OpenStream(
              &pa_stream,
              NULL, /* no input */
              &pa_outputParameters,
              samplerate,
              AUDIO_BUFFER_SIZE,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              paPlayCallback,
              &playData);
  if (pa_err != paNoError)
  {
    cerr << "MMSound PORTAUDIO open stream error: " << Pa_GetErrorText(pa_err) << endl;
    return false;
  }

  pa_err = Pa_SetStreamFinishedCallback(pa_stream, &paStreamFinished);
  if (pa_err != paNoError)
  {
    cerr << "MMSound PORTAUDIO setting stream finished callback error: " << Pa_GetErrorText(pa_err) << endl;
    return false;
  }

  return true;
}

void playBufferPortaudio(unsigned long int nbSamples, bool sound)
{
  if (nbSamples > 0)
  {
    unsigned long int loops = nbSamples / AUDIO_BUFFER_SIZE;
    unsigned long int remainderFrames = nbSamples % AUDIO_BUFFER_SIZE;
    unsigned long int moduloSamples = loops * AUDIO_BUFFER_SIZE;
    if (remainderFrames > AUDIO_BUFFER_SIZE/2)
      moduloSamples = (loops + 1) * AUDIO_BUFFER_SIZE;

    playData.reset(moduloSamples, sound);
    Pa_StartStream(pa_stream);
    while (!playData.finished)
    {
      Pa_Sleep(1);
    }
    Pa_StopStream(pa_stream);
  }
}
#else
// Dummy Funktionen bereitstellen
bool initPortaudio(const std::string& /*device*/)
{
	cerr << "MMSound PORTAUDIO not supported (HAVE_PORTAUDIO is not defined)" << endl;
  return false;
}

void playBufferPortaudio(unsigned long int /*nbSamples*/, bool /*sound*/)
{
}
#endif

#ifdef HAVE_PULSEAUDIO
// Our output device
pa_simple *dsp_fd = nullptr;
/* The Sample format to use */
static pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = (u_int32_t) samplerate,
	.channels = (u_int8_t) channels
};

bool initPulseaudio(const std::string& /*device*/)
{
	int error;

	if (!(dsp_fd = pa_simple_new(NULL, "Morsemann", PA_STREAM_PLAYBACK, NULL, 
				"playback", &ss, NULL, NULL, &error))) {
	        fprintf(stderr, "pa_simple_new() failed: %s\n", 
		pa_strerror(error));
    return false;
	}

  return true;
}

void playBufferPulseaudio(unsigned long int nbSamples, bool sound)
{
  int e;
  unsigned long int sample = 0;
  if (nbSamples > 0)
  {
    unsigned long int loops = nbSamples / AUDIO_BUFFER_SIZE;
    unsigned long int i = 0;
    unsigned long int j;
    for (; i < loops; ++i)
    {
      if (sound)
      {
        for (j = 0; j < AUDIO_BUFFER_SIZE; ++j)
        {
          audio_buffer[j] = g_buffer[sample++];
        }
      }
      else
      {
        for (j = 0; j < AUDIO_BUFFER_SIZE; ++j)
        {
          audio_buffer[j] = 0;
          ++sample;
        }
      }
      // Sending the sound
	    pa_simple_write(dsp_fd, audio_buffer, AUDIO_BUFFER_SIZE * sizeof(short int), &e);
    }
    unsigned long int remaining = nbSamples - sample;
    if (remaining > 0)
    {
      if (sound)
      {
        for (j = 0; j < remaining; ++j)
        {
          audio_buffer[j] = g_buffer[sample++];
        }
      }
      else
      {
        for (j = 0; j < remaining; ++j)
        {
          audio_buffer[j] = 0;
        }
      }
      // Sending the sound
	    pa_simple_write(dsp_fd, audio_buffer, remaining * sizeof(short int), &e);
    }
  }
}
#else
// Dummy Funktionen bereitstellen
bool initPulseaudio(const std::string& /*device*/)
{
	cerr << "MMSound Pulseaudio not supported (HAVE_PULSEAUDIO is not defined)" << endl;
  return false;
}

void playBufferPulseaudio(unsigned long int /*nbSamples*/, bool /*sound*/)
{
}
#endif


#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
void renderFrequencyToBuffer(int frequency)
{
  int amp = 100;
  int amplitude = (int)((double) amp * 327.67);

  float t = ((float) 2 * M_PI * frequency) / (samplerate * channels);
  for (int i = 0; i < BUF_LEN; ++i) {
      g_buffer[i] = (unsigned short int) (sin(t*i) * amplitude);
  }
}

void clearGlobalBuffer()
{
  for (int i = 0; i < BUF_LEN; ++i) {
      g_buffer[i] = (unsigned short int) 0;
  }
}

// Smoothing function, f(x) = sin(PI/2*x)^2 
float smoothSinSquared(float x) 
{
  if (x <= 0.0)
    return 0.0;
  if (x >= 1.0)
    return 1.0;
  return pow(sin(M_PI*x/2.0), 2);
}

//
// Smoothstep functions, see also
// https://en.wikipedia.org/wiki/Smoothstep
//

// Smoothstep, version A, f(x) = -2x^3 + 3x^2
float smoothStep(float x) 
{
  if (x <= 0.0)
    return 0.0;
  if (x >= 1.0)
    return 1.0;
  return x * x * (3.0f - 2.0f * x);
}

// Smoothstep, version B, f(x) = 6x^5 - 15x^4 + 10x^3
float smootherStep(float x) 
{
  if (x <= 0.0)
    return 0.0;
  if (x >= 1.0)
    return 1.0;
  return x * x * x * (x * (6.0f * x - 15.0f) + 10.0f);
}

std::function<float(float)> smoothenAmplitude[4] = {
  nullptr,
  &smoothStep,
  &smootherStep,
  &smoothSinSquared
};


unsigned long int renderMorseElementAt(int frequency,
                                       unsigned long int toGo,
                                       unsigned long int start,
                                       unsigned long int ditSamples)
{
  int amp = 100;
  int amplitude = (int)((double) amp * 327.67);

  float t = ((float) 2 * M_PI * frequency) / (samplerate * channels);
  unsigned long int endOfChar = start;
  unsigned long int currentSample = 0;
  unsigned long int smoothSamples = durationToSamples(rampLength);
  float x = 0.0;

  switch (mmslSmoothen)
  {
    case 1: // Smoothen with f(x) = -2x^3 + 3x^2
    case 2: // Smoothen with f(x) = 6x^5 - 15x^4 + 10x^3
    case 3: // Smoothen with f(x) = sin(PI/2*x)^2
            // Smoothen IN
            for (currentSample = endOfChar; currentSample < (endOfChar + smoothSamples); ++currentSample)
            {
              x = ((float) (currentSample - endOfChar))/((float) smoothSamples);
              g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude * smoothenAmplitude[mmslSmoothen](x));
            }
            if (rampStrategy == 1)
            {
              // Simulation eines Tiefpass-Verhaltens (ausklingendes Smoothing
              // setzt erst nach dem Ende des Zeichens ein...

              // Normal data
              for (; currentSample < (endOfChar + toGo); ++currentSample)
              {
                g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude);
              }
              // Smoothen OUT
              for (; currentSample < (endOfChar + toGo + smoothSamples); ++currentSample)
              {
                x = ((float) (endOfChar + toGo + smoothSamples - currentSample))/((float) smoothSamples);
                g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude * smoothenAmplitude[mmslSmoothen](x));
              }
            }
            else
            {
              // Ein- und ausklingendes Smoothing befinden sich komplett
              // "innerhalb" des Zeichens...

              // Normal data
              for (; currentSample < (endOfChar + toGo - smoothSamples); ++currentSample)
              {
                g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude);
              }
              // Smoothen OUT
              for (; currentSample < (endOfChar + toGo); ++currentSample)
              {
                x = ((float) (endOfChar + toGo - currentSample))/((float) smoothSamples);
                g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude * smoothenAmplitude[mmslSmoothen](x));
              }
            }
            break;
    default: // No smoothing at all
            for (currentSample = endOfChar; currentSample < (endOfChar + toGo); ++currentSample)
            {
              g_buffer[currentSample] = (unsigned short int) (sin(t*currentSample) * amplitude);
            }
            break;
  }
  endOfChar += toGo;

  // add a pause
  if (rampStrategy == 1)
  {
    endOfChar += ditSamples - smoothSamples;
  }
  else
  {
    endOfChar += ditSamples;
  }

  return endOfChar;
}

unsigned long int renderMorseCharAt(const string &cw, unsigned long int start)
{
  unsigned long int ditSamples = durationToSamples(mmslDotLength);
  unsigned long int endOfChar = start;
  unsigned long int toGo = 0;
  for (size_t pos = 0; pos < cw.size(); ++pos)
  {
    if (cw[pos] == '.')
    {
      // render a point
      toGo = ditSamples;
    }
    else
    {
      // render a dash
      toGo = 3 * ditSamples;
    }
    endOfChar = renderMorseElementAt(mmslFrequency, toGo, endOfChar, ditSamples);
  }

  return endOfChar;
}

unsigned long int renderErrorToneAt(unsigned long int start)
{
  unsigned long int ditSamples = durationToSamples(mmslDotLength);
  unsigned long int endOfChar = start;
  unsigned long int toGo = ditSamples;
  for (int pos = 0; pos < 3; ++pos)
  {
    // render a point
    endOfChar = renderMorseElementAt(700 - pos * 100, toGo, endOfChar, ditSamples);
  }
  endOfChar += 2 * ditSamples;

  return endOfChar;
}

std::function<void(unsigned long int, bool)> playBuffer[4] = {
  nullptr,
  &playBufferAlsa,
  &playBufferPortaudio,
  &playBufferPulseaudio
};

#endif

bool mmslInitSoundSystem(int system, const std::string &device)
{
  if (mmslSystem != MMSL_NONE)
  {
    mmslCloseSoundSystem();
  }

  switch (system)
  {
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      if (initPortaudio(device))
      {
        mmslSystem = MMSL_PORTAUDIO;
        return true;  
      }
#endif
      cerr << "mmslInitSoundSystem: PORTAUDIO support is not available!" << endl;
      return false;
      break;
    case MMSL_PULSEAUDIO:
#ifdef HAVE_PULSEAUDIO
      if (initPulseaudio(device))
      {
        mmslSystem = MMSL_PULSEAUDIO;
        return true;  
      }
#endif
      cerr << "mmslInitSoundSystem: Pulseaudio support is not available!" << endl;
      return false;
      break;
    default:
      break;
  }

  return true;
}

void mmslPrepareSoundStream()
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      if (pcm_handle)
      {
        snd_pcm_prepare(pcm_handle);
      }
#endif
      break;
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
#endif
      break;
    case MMSL_PULSEAUDIO:
#ifdef HAVE_PULSEAUDIO
#endif
      break;
    default:
      break;
  }
}

void mmslDrainSoundStream()
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      if (pcm_handle)
      {
        snd_pcm_drain(pcm_handle);
      }
#endif
      break;
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      if (pa_stream)
      {
        Pa_StopStream(pa_stream);
      }
#endif
      break;
    case MMSL_PULSEAUDIO:
#ifdef HAVE_PULSEAUDIO
#endif
      break;
    default:
      break;
  }
}

void mmslCloseSoundSystem()
{
  switch (mmslSystem)
  {
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
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      if (pa_stream)
      {
        Pa_CloseStream(pa_stream);
        Pa_Terminate();
        pa_stream = nullptr;
      }
#endif
      break;
    case MMSL_PULSEAUDIO:
#ifdef HAVE_PULSEAUDIO
      if (dsp_fd)
      {
        pa_simple_free(dsp_fd);
        dsp_fd = nullptr;
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
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      return true;
#else
      return false;
#endif
      break;
    case MMSL_PORTAUDIO:
#ifdef HAVE_PORTAUDIO
      return true;
#else
      return false;
#endif
      break;
    case MMSL_PULSEAUDIO:
#ifdef HAVE_PULSEAUDIO
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

void mmslSetSmoothening(unsigned int smoothen)
{
  mmslSmoothen = smoothen;
}

unsigned int mmslGetSmoothening()
{
  return mmslSmoothen;
}

void mmslSetFrequency(unsigned int frequency)
{
  mmslFrequency = frequency;
}

unsigned int mmslGetFrequency()
{
  return mmslFrequency;
}

/** Erzeugt eine Pause von \a duration Millisekunden.
@param duration Anzahl der Millisekunden
*/
void mmslPlayPause(unsigned long int duration)
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
    case MMSL_PORTAUDIO:
    case MMSL_PULSEAUDIO:
#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
      playBuffer[mmslSystem](durationToSamples(duration), false);
#endif
      break;
    default:
      break;
  }
}

/** Erzeugt eine Pause zwischen zwei Worten.
 */
void mmslPlayPauseWord()
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
    case MMSL_PORTAUDIO:
    case MMSL_PULSEAUDIO:
#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
      playBuffer[mmslSystem](durationToSamples(4 * mmslDelayFactor * mmslDotLength), false);
#endif
      break;
    default:
      break;
  }
}

/** Gibt einen Fehlerton aus.
*/
void mmslPlayErrorTone()
{
  switch (mmslSystem)
  {
    case MMSL_ALSA:
    case MMSL_PORTAUDIO:
    case MMSL_PULSEAUDIO:
#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
      clearGlobalBuffer();
      unsigned long int endOfChar = renderErrorToneAt(0);
      playBuffer[mmslSystem](endOfChar, true);
#endif
      break;
  }
}

void mmslSetBpm(unsigned int bpm)
{
  mmslDotLength = (int) (6000/bpm);
  mmslBpm = bpm;
}

unsigned int mmslGetBpm()
{
  return mmslBpm;
}

void mmslSetDelayFactor(unsigned int factor)
{
  mmslDelayFactor = factor;
}

unsigned int mmslGetDelayFactor()
{
  return mmslDelayFactor;
}

const map<int, string> cwCode = {
{97, ".-"},      // a
{98, "-..."},    // b
{99, "-.-."},    // c
{100, "-.."},    // d
{101, "."},      // e
{102, "..-."},   // f
{103, "--."},    // g
{104, "...."},   // h
{105, ".."},     // i
{106, ".---"},   // j
{107, "-.-"},    // k
{108, ".-.."},   // l
{109, "--"},     // m
{110, "-."},     // n
{111, "---"},    // o
{112, ".--."},   // p
{113, "--.-"},   // q
{114, ".-."},    // r
{115, "..."},    // s
{116, "-"},      // t
{117, "..-"},    // u
{118, "...-"},   // v
{119, ".--"},    // w
{120, "-..-"},   // x
{121, "-.--"},   // y
{122, "--.."},   // z
{48, "-----"},   // 0
{49, ".----"},   // 1
{50, "..---"},   // 2
{51, "...--"},   // 3
{52, "....-"},   // 4
{53, "....."},   // 5
{54, "-...."},   // 6
{55, "--..."},   // 7
{56, "---.."},   // 8
{57, "----."},   // 9
// Satzzeichen
{44, "--..--"},  // ,
{46, ".-.-.-"},  // .
{63, "..--.."},  // ?
{47, "-..-."},   // /
{61, "-...-"},   // =
// Start der Zeichen die wir normalerweise nicht im Morsetext ausgeben
{33, "-.-.--"},  // !
{34, ".-..-."},  // "
{36, "...-..-"}, // $
{39, ".----."},  // '
{40, "-.--."},   // (
{41, "-.--.-"},  // )
{43, ".-.-."},   // +
{45, "-....-"},  // -
{58, "---..."},  // :
{59, "-.-.-."},  // ;
{64, ".--.-."},  // @
{96, ".-----."}};// `

// Maximale Anzahl der Elemente in einem Morsezeichen
#define MAX_CHAR_ELEMENTS 7

/** Gibt alle bekannten Zeichen des Strings in Morse-Code aus.
@param msg Der zu gebende Text
@return 1 wenn unbekannte Zeichen enthalten waren (Fehler), 0 sonst
*/
int mmslMorseWord(const string &msg)
{
  int res = MM_FALSE;
  unsigned int slen = msg.size();

  switch (mmslSystem)
  {
    case MMSL_ALSA:
    case MMSL_PORTAUDIO:
    case MMSL_PULSEAUDIO:
#if defined(HAVE_ALSA) || defined(HAVE_PORTAUDIO) || defined(HAVE_PULSEAUDIO)
      // Maximale Länge des gesamten Wortes bei aktueller BpM Geschwindigkeit in Dots...
      unsigned long int elements = (slen * MAX_CHAR_ELEMENTS * 4) + (slen - 1) * 2 +
                                   (mmslDelayFactor - 1) * 3;
      // ...und in Samples.
      unsigned int durationMs = elements * mmslDotLength;
      if (rampStrategy == 1)
      {
        durationMs += rampLength;
      }
      unsigned long int wordDuration = durationToSamples(durationMs);
      unsigned long int endOfChar = 0;
      if (wordDuration < BUF_LEN)
      {
        // Das gesamte Wort wird in den Buffer gerendert
        clearGlobalBuffer();
        for (unsigned int scnt = 0; scnt < slen; ++scnt)
        {
          map<int, string>::const_iterator c_it = cwCode.find(msg[scnt]);
          if (c_it == cwCode.end())
          {
            res = MM_TRUE;
            continue;
          }

          endOfChar = renderMorseCharAt(c_it->second, endOfChar);
          // 2 Dits Pause ...
          endOfChar += durationToSamples(2 * mmslDotLength);
          // plus ggf. die Verlängerung durch den delay-Faktor.
          if (mmslDelayFactor > 1)
            endOfChar += durationToSamples((mmslDelayFactor - 1) * 3 * mmslDotLength);
        }
        playBuffer[mmslSystem](endOfChar, true);
      }
      else
      {
        // Der Buffer ist bei aktueller Geschwindigkeit zu klein...
        // die Zeichen werden einzeln ausgeben
        for (unsigned int scnt = 0; scnt < slen; ++scnt)
        {
          map<int, string>::const_iterator c_it = cwCode.find(msg[scnt]);
          if (c_it == cwCode.end())
          {
            res = MM_TRUE;
            continue;
          }

          clearGlobalBuffer();
          unsigned long int endOfChar = renderMorseCharAt(c_it->second, (unsigned long int) 0);
          playBuffer[mmslSystem](endOfChar, true);
          // 2 Dits Pause ...
          playBuffer[mmslSystem](durationToSamples(2 * mmslDotLength), false);
          // plus ggf. die Verlängerung durch den delay-Faktor.
          if (mmslDelayFactor > 1)
            playBuffer[mmslSystem](durationToSamples((mmslDelayFactor - 1) * 3 * mmslDotLength), false);
        }
      }
#endif
      break;
  }

  return res;
}
