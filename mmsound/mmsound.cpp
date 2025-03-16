#include "mmsound.h"
#include "beep.h"
#include "alarm.h"
#include "global.h"
#include <map>

#ifdef HAVE_ALSA
#include <cmath>
#include <alsa/asoundlib.h>
#endif

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
static int mmslFrequency = 800;
static int mmslSmoothen = 0;
static int mmslSystem = MMSL_NONE;

#ifdef HAVE_ALSA
// Length of our complete render buffer: 8 * 48000 = 384000
#define BUF_LEN 384000
// Render buffer for output to Alsa
unsigned short g_buffer[BUF_LEN];
// Length of our audio buffer towards ALSA: 16*1024 = 16384
#define AUDIO_BUFFER_SIZE	16384
unsigned short audio_buffer[AUDIO_BUFFER_SIZE];

// Our output device
snd_pcm_t *pcm_handle = NULL;
int channels = 1;
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
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
    cerr << "MMSound ALSA open error: " << snd_strerror(err) << endl;
    return false;
  }

	if ((err = snd_pcm_set_params(pcm_handle,
				format,
				SND_PCM_ACCESS_RW_INTERLEAVED,
				channels,
				rate,
				1, /* period */
				500000)) < 0) {  /* latency: 0.5s */
		cerr << "MMSound ALSA error in snd_pcm_set_params: " << snd_strerror(err) << endl;
		return false;
	}

  // Ensure that all audio buffer are written before returning
  // from e.g. a drain (snd_pcm_drain()).
  snd_pcm_nonblock(pcm_handle, 0);
  return true;
}

void playBufferAlsa(unsigned int duration, bool sound = true)
{
  unsigned long int nbSamples = rate * channels * (((float) duration) / 1000.0);
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

void renderFrequencyToBuffer(int frequency)
{
  int amp = 100;
  int amplitude = (int)((double) amp * 327.67);

  float t = ((float) 2 * M_PI * frequency) / (rate * channels);
  for (int i = 0; i < BUF_LEN; ++i) {
      g_buffer[i] = (int) (sin(t*i) * amplitude);
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
    default:
      break;
  }
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

void mmslSetSmoothening(int smoothen)
{
  mmslSmoothen = smoothen;
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

void mmslPlayToneDits(unsigned int dits)
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
      Beep((int) dits * mmslDotLength, 10, mmslFrequency);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
      playBufferAlsa(dits * mmslDotLength);
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
      AlarmSet(duration);
      AlarmWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA    
      playBufferAlsa(duration, false);
#endif
      break;
    default:
      break;
  }
}

/** Erzeugt eine Pause in der Länge von \a dits Punkten.
@param dits Anzahl der Punkte 
*/
void mmslPlayPauseDits(unsigned int dits)
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
      AlarmSet(dits * mmslDotLength);
      AlarmWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA    
      playBufferAlsa(dits * mmslDotLength, false);
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
    case MMSL_SPEAKER:
      AlarmSet(4 * mmslDelayFactor * mmslDotLength);
      AlarmWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA    
      playBufferAlsa(4 * mmslDelayFactor * mmslDotLength, false);
#endif
      break;
    default:
      break;
  }
}


/** Erzeugt einen Ton von der Länge eines Punktes.
*/
void dit(void)
{
  mmslPlayToneDits(1);
  mmslPlayPauseDits(1);
}

/** Erzeugt einen Ton von der Länge eines Striches
(= Drei Punkte).
*/
void dah(void)
{
  mmslPlayToneDits(3);
  mmslPlayPauseDits(1);
}

/** Gibt einen Fehlerton aus.
*/
void mmslPlayErrorTone()
{
  switch (mmslSystem)
  {
    case MMSL_SPEAKER:
      Beep(mmslDotLength, 10, 700);
      BeepWait();
      Beep(mmslDotLength, 10, 600);
      BeepWait();
      Beep(mmslDotLength, 10, 500);
      BeepWait();
      Beep(mmslDotLength*3, 0, 500);
      BeepWait();
      break;
    case MMSL_ALSA:
#ifdef HAVE_ALSA
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
{47, "-..-."},   // !
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

/** Gibt die Zeichen und den dazugehörigen Morse-Code aus.
@param signID Das auszugebende Zeichen
@return 1 wenn das Zeichen unbekannt ist (Fehler), 0 sonst
*/
int mmslMorseChar(int signID)
{
  map<int, string>::const_iterator c_it = cwCode.find(signID);
  if (c_it == cwCode.end())
    return MM_TRUE;
  for (unsigned int i = 0; i < c_it->second.size(); ++i)
  {
    if (c_it->second[i] == '.')
      dit();
    else
      dah();
    mmslPlayPauseDits(1);
  }

  return MM_FALSE;
}

/** Gibt alle bekannten Zeichen des Strings in Morse-Code aus.
@param msg Der zu gebende Text
@return 1 wenn unbekannte Zeichen enthalten waren (Fehler), 0 sonst
*/
int mmslMorseWord(const string &msg)
{
  int res = MM_FALSE;
  if (mmslBpm > 200)
  {
    // loop over x times buffer length
    //  renderFullWord
    ;
  }
  else
  {
    // Einzelne Zeichen ausgeben
    unsigned int slen = msg.size();
    for (unsigned int scnt = 0; scnt < slen; ++scnt)
    {
      map<int, string>::const_iterator c_it = cwCode.find(msg[scnt]);
      if (c_it == cwCode.end())
        continue;
      for (unsigned int i = 0; i < c_it->second.size(); ++i)
      {
        if (c_it->second[i] == '.')
          dit();
        else
          dah();
        mmslPlayPauseDits(1);
      }
      mmslPlayPauseDits(2);
      mmslPlayPause((mmslDelayFactor-1) * 3);
    }
  }
  
  return res;
}