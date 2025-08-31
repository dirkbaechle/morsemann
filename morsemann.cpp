/* Morsemann - Ein kleines Programm zum Lernen und Üben des
*              Hörens von Morsezeichen (CW).
*
* Copyright (C) 2003-2025 by Dirk Baechle (dl9obn@darc.de)
*
* https://github.com/dirkbaechle/morsemann
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the 
*
* Free Software Foundation, Inc.
* 675 Mass Ave
* Cambridge
* MA 02139
* USA
*
*/

/** \file morsemann.c
Der ``Morsemann''. Ein kleines Programm zum Lernen und Üben des Hörens
von Morsezeichen (CW).
\author Dirk Bächle
\version 1.3
\date 2003-03-07
*/

/* Benutzt keine Farben, sondern nur das */
/* (hoffentlich) vom Terminal unterstützte ``Highlighting'' */
/* #define NO_COLORS */

/*-------------------------------------------------------- Includes */

#include "mmsound.h"
#include "mmscreen.h"
#include "mmword.h"
#include "utf8file.h"
#include "mmconfig.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include "sys/stat.h"

using std::map;
using std::string;
using std::ostringstream;
using std::cout;
using std::endl;

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

const int GROUP_WIDTH = 15;
const int OPT_LEFT_WIDTH = 19;
const int OPT_RIGHT_WIDTH = 8;
const int MENU_WIDTH = 6;
const int INFO_WIDTH = 20;

const int MAX_FILENAME_LENGTH = 12;

/*------------------------------------------------ Global variables */

/** Gesamtzahl der zu gebenden Buchstaben */
unsigned long int totalLength = 200;
/** Gesamtzahl der gemachten Fehler */
int errorCount = 0;
/** Soll die Tonhöhe zufällig gewählt werden? (0=nein, 1=ja) */
int selectMorseFrequencyRandomly = MM_TRUE;
/** Tonhöhe in Hz falls die Tonhöhe fest gesetzt ist. */
int fixedMorseFrequency = 800;
/** Wird im Datei-Modus die gewählte Länge an Zeichen beachtet? (0=nein, 1=ja) */
int countCharsInFileMode = MM_TRUE;
/** Sollen die Optionen/Einstellungen in der INI Datei gespeichert
 * werden? (0=nein, 1=ja)
 */
int saveOptionsToIniFile = MM_TRUE;

MMConfig config;
MMConfig initialConfig;
string configPath;

/*--------------------------------------------------- Functions */

const map<int, string> cwChar = {
{97, "a"},
{98, "b"},
{99, "c"},
{100, "d"},
{101, "e"},
{102, "f"},
{103, "g"},
{104, "h"},
{105, "i"},
{106, "j"},
{107, "k"},
{108, "l"},
{109, "m"},
{110, "n"},
{111, "o"},
{112, "p"},
{113, "q"},
{114, "r"},
{115, "s"},
{116, "t"},
{117, "u"},
{118, "v"},
{119, "w"},
{120, "x"},
{121, "y"},
{122, "z"},
{48, "0"},
{49, "1"},
{50, "2"},
{51, "3"},
{52, "4"},
{53, "5"},
{54, "6"},
{55, "7"},
{56, "8"},
{57, "9"},
// Satzzeichen
{44, ","},
{46, "."},
{63, "?"},
{47, "/"},
{61, "="},
// Start der Zeichen die wir normalerweise nicht im Morsetext ausgeben
{33, "!"},
{34, "\""},
{36, "$"},
{39, "'"},
{40, "("},
{41, ")"},
{43, "+"},
{45, "-"},
{58, ":"},
{59, ";"},
{64, "@"},
{96, "`"}};

/** Die Endnachricht.
*/
void byeMessage(void)
{
  clrscr();
  gotoxy(centerX - 6, centerY);
  if (selectMorseFrequencyRandomly)
    mmslSetFrequency((mmRandom(5) + 6) * 100);
  else
    mmslSetFrequency(fixedMorseFrequency);
  mmslSetDelayFactor(1);
  mmslSetBpm(140);
  mmslPrepareSoundStream();
  writeString("73 ");
  mmslMorseWord("73");
  mmslPlayPauseWord();
  mmslDrainSoundStream();
  writeString("de ");
  mmslPrepareSoundStream();
  mmslMorseWord("de");
  mmslPlayPauseWord();
  mmslDrainSoundStream();
  writeString("dl9obn");
  mmslPrepareSoundStream();
  mmslMorseWord("dl9obn");
  mmslPlayPause(500);
}

/** Liest einen String als Zeichenmenge ein.
*/
void readCharSet(void)
{
  int error, i;
  char b = '0';

  clrscr();
  writeSelection("Eingabe der Zeichenmenge", centerX-12, centerY-1, 1, 2);

  do
  {
    error = MM_FALSE;
    i = 0;

    charSet = readString(1, centerY+1, 75, charSet);

    charSetLength = charSet.size();

    while ((i < charSetLength) && (error == MM_FALSE))
    {
      b = charSet[i++];
      if (((b < 97) || (b > 122)) && ((b < 48) || (b > 57)) && (b != 44) && (b != 46) && (b != 47) && (b != 61) && (b != 63))
      {
        error = MM_TRUE;
        mmslPlayErrorTone();
      }
    }
  } while ((charSetLength == 0) || (error == MM_TRUE));
}

/** Zeigt die aktuelle Auswahl der Zeichenmenge an.
*/
void charGroupMenu(void)
{
  writeSelection(groupString[0], centerX-GROUP_WIDTH, centerY-3, 1, selectedCharGroup);
  writeSelection(groupString[1], centerX-GROUP_WIDTH, centerY-2, 2, selectedCharGroup);
  writeSelection(groupString[2], centerX-GROUP_WIDTH, centerY-1, 3, selectedCharGroup);
  writeSelection(groupString[3], centerX-GROUP_WIDTH, centerY,   4, selectedCharGroup);
  writeSelection(groupString[4], centerX-GROUP_WIDTH, centerY+1, 5, selectedCharGroup);
  writeSelection(groupString[5], centerX-GROUP_WIDTH, centerY+2, 6, selectedCharGroup);
  writeSelection(groupString[6], centerX-GROUP_WIDTH, centerY+3, 7, selectedCharGroup);
  writeSelection(groupString[7], centerX-GROUP_WIDTH, centerY+4, 8, selectedCharGroup);
}

/** Auswahl der auszugebenden Zeichen.
*/
void charGroupSelection(void)
{
  keyChar b = '0';

  clrscr();
  writeSelection("Auswahl der Zeichen", centerX-10, centerY-5, 1, 2);

  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {
    charGroupMenu();
    b = getch();

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (selectedCharGroup == CG_ALL_CHARS) selectedCharGroup = 8;
      else --selectedCharGroup;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (selectedCharGroup == CG_ENTERED_CHAR_SET) selectedCharGroup = 1;
      else ++selectedCharGroup;
    }
  }

  if (selectedCharGroup == CG_ENTERED_CHAR_SET)
  {
    readCharSet();
  }
}

/** Erhöht die aktuelle Morsegeschwindigkeit.
 */
void bpmUp(void)
{
  unsigned int bpm = mmslGetBpm();

  if (bpm == 250)
    bpm = 10;
  else
  {
    if (bpm >= 180)
      bpm += 10;
    else
      bpm += 5;
  }

  mmslSetBpm(bpm);
}

/** Verringert die aktuelle Morsegeschwindigkeit.
 */
void bpmDown(void)
{
  unsigned int bpm = mmslGetBpm();

  if (bpm == 10)
    bpm = 250;
  else
  {
    if (bpm > 180)
      bpm -= 10;
    else
      bpm -= 5;
  }

  mmslSetBpm(bpm);
}

/** Erhöht die aktuelle Tonhöhe.
 */
void toneUp()
{
  unsigned int tone = mmslGetFrequency();

  if (tone < 1200)
  {
    tone += 100;

    mmslSetFrequency(tone); 
  }
}

/** Verringert die aktuelle Tonhöhe.
 */
void toneDown()
{
  unsigned int tone = mmslGetFrequency();

  if (tone > 600)
  {
    tone -= 100;

    mmslSetFrequency(tone); 
  }
}

/** Auswahl der Geschwindigkeit.
*/
void speedSelection(void)
{
  keyChar b = '0';

  clrscr();
  writeSelection("Auswahl der Geschwindigkeit", centerX-15, centerY-1, 1, 2);

  textModusSelect();
  unsigned int bpm;

  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {
    gotoxy(centerX-3, centerY+1);
    bpm = mmslGetBpm();
    if (bpm < 100)
      writeChar(' ');

    writeNumber(bpm);
    writeString(" BpM");

    b = getch();

    /* Cursor hoch/rechts */
    if ((b == KEY_UP) || (b == KEY_RIGHT))
    {
      bpmUp();
    }

    /* Cursor runter/links */
    if ((b == KEY_DOWN) || (b == KEY_LEFT))
    {
      bpmDown();
    }
  }

  textModusNormal();
}

/** Auswahl des Pausenfaktors.
*/
void delaySelection(void)
{
  keyChar b = '0';

  clrscr();
  writeSelection("Auswahl des Pausenfaktors", centerX-11, centerY-1, 1, 2);

  textModusSelect();

  unsigned int delayFactor = mmslGetDelayFactor();
  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {
    gotoxy(centerX-5, centerY+1);
    writeNumber(delayFactor);
    writeString(" x Pause");

    b = getch();

    /* Cursor hoch/rechts */
    if ((b == KEY_UP) || (b == KEY_RIGHT))
    {
      if (delayFactor == 9) delayFactor = 1;
      else ++delayFactor;
    }

    /* Cursor runter/links */
    if ((b == KEY_DOWN) || (b == KEY_LEFT))
    {
      if (delayFactor == 1) delayFactor = 9;
      else --delayFactor;
    }
  }

  textModusNormal();

  mmslSetDelayFactor(delayFactor);
}

/** Eingabe der Gesamtanzahl der auszugebenden Buchstaben.
*/
void lengthSelection(void)
{
  clrscr();
  writeSelection("Gesamtanzahl der Buchstaben (mindestens 5!)", centerX-22, centerY-1, 1, 2);
  do
  {
    totalLength = readNumber(centerX-2, centerY+1, 9, totalLength);
  } while (totalLength < 5);
}


/** Liest einen Dateinamen ein.
*/
void fileNameSelection(void)
{
  clrscr();
  writeSelection("Dateiname", centerX-6, centerY-1, 1, 2);

  fileName = readString(1, centerY+1, 75, fileName);
}

/** Zeigt die aktuell eingestellten Optionen für die Morsezeichen an.
@param akt Aktuelle Auswahl
*/
void morseOptionsMenu(int akt)
{
  clrscr();
  writeSelection("*** Optionen ***", centerX-8, centerY-6, 1, 2);

  writeSelection("Geschwindigkeit (in BpM):", centerX-OPT_LEFT_WIDTH, centerY-4, 1, akt);
  writeSelection("Pausenfaktor:", centerX-OPT_LEFT_WIDTH, centerY-3, 2, akt);
  writeSelection("Zeichenanzahl:", centerX-OPT_LEFT_WIDTH, centerY-2, 3, akt);
  writeSelection("Zeichen:", centerX-OPT_LEFT_WIDTH, centerY-1, 4, akt);
  writeSelection("Zeichen bestätigen:", centerX-OPT_LEFT_WIDTH, centerY, 5, akt);
  writeSelection("Worte erzeugen:", centerX-OPT_LEFT_WIDTH, centerY+1, 6, akt);

  if (wordMode == MM_WM_RANDOM) {
    writeSelection("Feste Wortgruppen:", centerX-OPT_LEFT_WIDTH, centerY+2, 7, akt);
  } else if (wordMode == MM_WM_FILE) {
    writeSelection("Dateimodus:", centerX-OPT_LEFT_WIDTH, centerY+2, 7, akt);
    writeSelection("Dateiname:", centerX-OPT_LEFT_WIDTH, centerY+3, 8, akt);
    writeSelection("Alle Zeichen geben:", centerX-OPT_LEFT_WIDTH, centerY+4, 9, akt);
  }

  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-4);
  writeNumber(mmslGetBpm());
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-3);
  writeNumber(mmslGetDelayFactor());
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-2);
  writeNumberULong(totalLength);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-1);
  writeString(groupString[selectedCharGroup - 1]);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY);
  switch (confirmWords)
  {
    case 0: writeString("Nein");
            break;
    case 1: writeString("Worte");
            break;
  }

  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+1);
  if (wordMode == MM_WM_RANDOM) {
    writeString("Zufällig");
    gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+2);
    if (variableWords == MM_FALSE)
    {
      writeString("Ja (");
      writeNumber(fixedWordLength);
      writeString(")");
    }
    else
      writeString("Nein (3-8)");
  } else if (wordMode == MM_WM_FILE) {
    writeString("Aus Datei");
    gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+2);
    if (fileWordsRandom) {
      writeString("Worte");
    } else {
      writeString("Text");
    }
    gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+3);
    if (!fileName.empty()) {
      if (fileName.size() < MAX_FILENAME_LENGTH)
      {
        writeString(fileName);
      } else {
        writeString("Gesetzt");
      }
      if (!utf8FileExists)
      {
        gotoxy(centerX+OPT_RIGHT_WIDTH-2, centerY+3);
        writeString("!");
      }
      else
      {
        if (utf8FileContainsNoWords)
        {
          gotoxy(centerX+OPT_RIGHT_WIDTH-2, centerY+3);
          writeString("~");
        }
      }
    } else {
      writeString("Nein");
    }
    gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+4);
    if (fileWordsExtendedCharset) {
      writeString("Ja");
    } else {
      writeString("Nein");
    }
  } else {
    writeString("PARIS");
  }

  /* Zeichenmenge anzeigen, falls Option "Zeichen eingeben" gewählt */
  if (selectedCharGroup == CG_ENTERED_CHAR_SET)
  {
    writeSelection("Zeichenmenge:", centerX-6, centerY+7, 1, 2);
    if ((centerX-(charSetLength/2)) > 0)
      writeSelection(charSet.c_str(), centerX-(charSetLength/2), centerY+8, 1, 2);
    else
      writeSelection(charSet.c_str(), 0, centerY+8, 1, 2);
  }

}


/** Auswahl für die Morsezeichen-Optionen.
*/
void morseOptionsSelection(void)
{
  keyChar b = '0';
  int currentOption = 1;
  string oldFileName = fileName;
  int oldCharGroup = selectedCharGroup;
  string oldCharSet = charSet;

  if (wordMode == MM_WM_FILE)
    rescanUtf8File();
  while (b != KEY_BACKSPACE)
  {
    morseOptionsMenu(currentOption);
    b = getch();

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (currentOption == 1) {
        if (wordMode == MM_WM_RANDOM)
          currentOption = 7;
        else if (wordMode == MM_WM_FILE)
          currentOption = 9;
        else
          currentOption = 6;
      }
      else --currentOption;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (wordMode == MM_WM_RANDOM) {
        if (currentOption == 7) currentOption = 1;
        else ++currentOption;
      } else if (wordMode == MM_WM_FILE) {
        if (currentOption == 9) currentOption = 1;
        else ++currentOption;
      } else {
        if (currentOption == 6) currentOption = 1;
        else ++currentOption;
      }
    }

    /* Cursor rechts */
    if (b == KEY_RIGHT)
    {
      if (wordMode == MM_WM_RANDOM) {
        if ((currentOption == 7) &&
            (variableWords == MM_FALSE))
        {
          if (fixedWordLength < 8) 
            ++fixedWordLength;
          else
            fixedWordLength = 2;
        } 
      }
    }

    /* Cursor links */
    if (b == KEY_LEFT)
    {
      if (wordMode == MM_WM_RANDOM) {
        if ((currentOption == 7) &&
            (variableWords == MM_FALSE))
        {
          if (fixedWordLength > 2) 
            --fixedWordLength;
          else
            fixedWordLength = 8;
        }
      } 
    }

    if (b == ENTER_CHAR)
    {
      switch (currentOption)
      {
        case 1: speedSelection();
          break;
        case 2: delaySelection();
          break;
        case 3: lengthSelection();
          break;
        case 4: charGroupSelection();
                if ((selectedCharGroup != oldCharGroup) ||
                    ((selectedCharGroup == CG_ENTERED_CHAR_SET) &&
                     (oldCharSet != charSet)))
                {
                  oldCharSet = charSet;
                  oldCharGroup = selectedCharGroup;
                  if (MM_WM_FILE == wordMode)
                    rescanUtf8File();
                }
          break;
        case 5: confirmWords = 1 - confirmWords;
          break;
        case 6: if (wordMode == MM_WM_PARIS)
                  wordMode = MM_WM_RANDOM;
                else
                  wordMode = 1 - wordMode;
                if (MM_WM_FILE == wordMode)
                  rescanUtf8File();
          break;
        case 7: if (wordMode == MM_WM_RANDOM)
                  variableWords = 1 - variableWords;
                else if (wordMode == MM_WM_FILE)
                  fileWordsRandom = 1 - fileWordsRandom;
          break;
        case 8: if (wordMode == MM_WM_FILE)
                {
                  fileNameSelection();
                  // Hat sich der Name der Datei geändert?
                  if (fileName != oldFileName)
                  {
                    oldFileName = fileName;
                    // Ja, also Wortzähler zurücksetzen
                    filePosition = 0;
                    rescanUtf8File();
                  }
                }
          break;
        default: fileWordsExtendedCharset = 1 - fileWordsExtendedCharset;
                 rescanUtf8File();
          break;
      }

    }
  }
}

/** Auswahl der festen Tonhöhe.
*/
void frequencySelection(void)
{
  keyChar b = '0';

  clrscr();
  writeSelection("Auswahl der Tonhöhe", centerX-10, centerY-1, 1, 2);

  textModusSelect();

  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {
    gotoxy(centerX-3, centerY+1);

    if (fixedMorseFrequency < 1000)
      writeString(" ");
    writeNumber(fixedMorseFrequency);
    writeString(" Hz");

    b = getch();

    /* Cursor hoch/rechts */
    if ((b == KEY_UP) || (b == KEY_RIGHT))
    {
      if (fixedMorseFrequency < 1000)
        fixedMorseFrequency += 100;
      else
        fixedMorseFrequency = 600;
    }

    /* Cursor runter/links */
    if ((b == KEY_DOWN) || (b == KEY_LEFT))
    {
      if (fixedMorseFrequency > 600)
        fixedMorseFrequency -= 100;
      else
        fixedMorseFrequency = 1000;
    }
  }

  textModusNormal();
}

/** Auswahl der Sound-Shaping Methode.
*/
void soundShapingSelection(void)
{
  keyChar b = '0';

  int selectedShaping;
  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {

    clrscr();
    textModusNormal();
    writeSelection("Auswahl des Zeichen-Shapings", centerX-14, centerY-1, 1, 2);

    textModusSelect();
    gotoxy(centerX-11, centerY+1);

    selectedShaping = mmslGetSmoothening();
    switch (selectedShaping) {
      case 1:  writeString("-2x^3 + 3x^2");
        break;
      case 2:  writeString("6x^5 - 15x^4 + 10x^3");
        break;
      case 3:  writeString("sin(PI/2*x)^2");
        break;
      default: writeString("Aus");
        break;
    }

    b = getch();

    /* Cursor hoch/rechts */
    if ((b == KEY_UP) || (b == KEY_RIGHT))
    {
      if (selectedShaping < 3)
        selectedShaping += 1;
      else
        selectedShaping = 0;
      mmslSetSmoothening(selectedShaping);
    }

    /* Cursor runter/links */
    if ((b == KEY_DOWN) || (b == KEY_LEFT))
    {
      if (selectedShaping > 0)
        selectedShaping -= 1;
      else
        selectedShaping = 3;
      mmslSetSmoothening(selectedShaping);
    }
  }

  textModusNormal();
}

/** Zeigt die aktuell eingestellten allgemeinen Optionen an.
@param akt Aktuelle Auswahl
*/
void commonOptionsMenu(int akt)
{
  clrscr();
  writeSelection("*** Einstellungen ***", centerX-8, centerY-5, 1, 2);

  writeSelection("Wahl der Tonhöhe", centerX-OPT_LEFT_WIDTH, centerY-3, 1, akt);
  writeSelection("Feste Tonhöhe (in Hz):", centerX-OPT_LEFT_WIDTH, centerY-2, 2, akt);
  writeSelection("Zeichen-Shaping:", centerX-OPT_LEFT_WIDTH, centerY-1, 3, akt);
  writeSelection("Zähle Fehler pro:", centerX-OPT_LEFT_WIDTH, centerY, 4, akt);
  writeSelection("Zähle Zeichen (Datei):", centerX-OPT_LEFT_WIDTH, centerY+1, 5, akt);
  writeSelection("Wortzähler (Datei):", centerX-OPT_LEFT_WIDTH, centerY+2, 6, akt);
  writeSelection("Optionen speichern:", centerX-OPT_LEFT_WIDTH, centerY+3, 7, akt);

  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-3);
  if (selectMorseFrequencyRandomly == MM_TRUE)
    writeString("zufällig");
  else
    writeString("fest");
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-2);
  writeNumber(fixedMorseFrequency);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-1);
  switch (mmslGetSmoothening()) {
    case 1:  writeString("x^3");
      break;
    case 2:  writeString("x^5");
      break;
    case 3:  writeString("sin");
      break;
    default: writeString("Aus");
      break;
  }
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY);
  if (mmwlGetCountErrorsPerWord())
    writeString("Wort");
  else
    writeString("Buchstabe");
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+1);
  if (countCharsInFileMode == MM_FALSE)
    writeString("Nein");
  else
    writeString("Ja");
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+2);
  writeNumber(filePosition);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+3);
  if (saveOptionsToIniFile == MM_FALSE)
    writeString("Nein");
  else
    writeString("Ja");
}

/** Auswahl für die allgemeinen Einstellungen.
*/
void commonOptionsSelection(void)
{
  keyChar b = '0';
  int currentOption = 1;
  // Wir prüfen hier ob sich die Einstellung zum
  // "Speichern in Config-Datei" ändert. Falls ja,
  // schreiben wir die Datei sofort neu (s. unten).
  // Dies ist besonders wichtig wenn man die Option
  // auf "aus" stellt, weil wir ja trotzdem die neue
  // Einstellung für den nächsten Programmstart
  // behalten möchten.
  int oldSaveOptions = saveOptionsToIniFile;

  while (b != KEY_BACKSPACE)
  {
    commonOptionsMenu(currentOption);
    b = getch();

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (currentOption == 1) currentOption = 7;
      else --currentOption;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (currentOption == 7) currentOption = 1;
      else ++currentOption;
    }

    if (b == ENTER_CHAR)
    {
      switch (currentOption)
      {
        case 1: selectMorseFrequencyRandomly = 1 - selectMorseFrequencyRandomly;
          break;
        case 2: frequencySelection();
          break;
        case 3: soundShapingSelection();
          break;
        case 4: mmwlSetCountErrorsPerWord(1 - mmwlGetCountErrorsPerWord());
          break;
        case 5: countCharsInFileMode = 1 - countCharsInFileMode;
          break;
        case 6: filePosition = 0;
          break;
        case 7: saveOptionsToIniFile = 1 - saveOptionsToIniFile;
          break;
      }
    }
  }

  // Hat sich "Speichern in Config-Datei" geändert?
  if (oldSaveOptions != saveOptionsToIniFile)
  {
    // Ja, also sofort die neue Config schreiben
    MMConfig savedConfig;
    savedConfig.readFromFile(configPath);
    savedConfig.saveOptions = saveOptionsToIniFile;
    savedConfig.writeFile(configPath);
    initialConfig = savedConfig;
  }
}

void updateInfos(WINDOW *infowin)
{
  ostringstream info;
  info << mmslGetBpm() << "bpm, " << mmslGetFrequency() << "Hz";
  string infoText = info.str();
  string fill(INFO_WIDTH - infoText.size(), ' ');
  
  wmove(infowin, 0, screenX - INFO_WIDTH);
  wprintw(infowin, "%s%s", fill.c_str(), infoText.c_str());
  wrefresh(infowin);
}

int handleKeyPress(int b, WINDOW *infowin)
{
  int error = MM_CONTINUE;
  switch (b)
  {
    case KEY_BACKSPACE:
    case KEY_ESCAPE: error = MM_ESCAPE;
                      break;
    case KEY_UP: // Tone up
                    toneUp();
                    updateInfos(infowin);
                    break;
    case KEY_DOWN: // Tone down
                    toneDown();
                    updateInfos(infowin);
                    break;
    case KEY_LEFT: // Slower
                    bpmDown();
                    updateInfos(infowin);
                    break;
    case KEY_RIGHT: // Faster
                    bpmUp();
                    updateInfos(infowin);
                    break;
  }

  return error;
}

int handleConfirmInput(WINDOW *confirmwin, WINDOW *infowin, const string &lastWord, string &userWord, int &action)
{
  unsigned int b = 0;
  int error = MM_CONTINUE;
  if (kbhit() != 0)
  {
    b = wgetch(confirmwin);
    error = handleKeyPress(b, infowin);
    while (kbhit() != 0) wgetch(confirmwin);
  }
  if (error == MM_CONTINUE)
  {
    wmove(confirmwin, 0, 1);
    wprintw(confirmwin, " Eingabe ('#' wiederholt) ");
    wrefresh(confirmwin);
    action = confirmString(confirmwin, 1, 1, lastWord.size()+1, userWord);
    if (action == MM_REPEAT)
      mmslPrepareSoundStream();
    box(confirmwin, 0, 0);
    wrefresh(confirmwin);
  }

  return error;
}

/** Ausgabe der Morsezeichen, entsprechend der Optionen.
*/
void outputMorseCode(void)
{
  unsigned long int currentLength = 0;
  int lineCount = 0;
  int lineStop = 0;
  int lineNumber = 0;
  int posX = 1;
  int posY = 1;
  string lastWord;
  string userWord;
  int intent = MM_CONTINUE; // do we want to stop, or continue going?
  int errors = 0;
  keyChar b = '0';
  int wordError = MM_UTF8_WORD;

  clrscr();

  if (MM_WM_FILE == wordMode)
  {
    rescanUtf8File();
    if (utf8FileExists == MM_FALSE)
    {
      writeSelection("Angegebene Datei nicht gefunden!",centerX-16, centerY-3, 1, 2);
      writeSelection(fileName, centerX-fileName.size()/2, centerY-1, 1, 2);
      writeSelection("<< Bitte eine Taste drücken >>", centerX-15, screenY-4, 1, 2);
      getch();
      return;
    }
    if (MM_TRUE == utf8FileContainsNoWords)
    {
      writeSelection("Die Datei enthält keine Wörter!",centerX-15, centerY-3, 1, 2);
      writeSelection(fileName, centerX-fileName.size()/2, centerY-1, 1, 2);
      writeSelection("<< Bitte eine Taste drücken >>", centerX-15, screenY-4, 1, 2);
      getch();
      return;
    }
  }

  // Windows erzeugen
  WINDOW *mainwin = nullptr;
  WINDOW *infowin = nullptr;
  WINDOW *confirmwin = nullptr;
  if (confirmWords == MM_TRUE)
  {
    mainwin = newwin(screenY - 4, screenX, 0, 0);
    infowin = newwin(1, screenX, screenY - 4, 0);
    confirmwin = newwin(3, screenX, screenY - 3, 0);
    keypad(confirmwin, TRUE);  /* enable keyboard mapping */
    box(confirmwin, 0, 0);
    wrefresh(confirmwin);
  }
  else
  {
    mainwin = newwin(screenY - 1, screenX, 0, 0);
    infowin = newwin(1, screenX, screenY - 1, 0);
  }
  keypad(mainwin, TRUE);  /* enable keyboard mapping */
  wrefresh(mainwin);
  updateInfos(infowin);

  prepareWordFile();
  errorCount = 0;

  lastWord = getNextWord(wordError);
  currentLength += lastWord.size();
  lineCount = lastWord.size();

  mmslPrepareSoundStream();
  mmslPlayPause(1000);

  do
  {
    if (confirmWords == MM_TRUE)
    {
      //
      // Wort in Morsezeichen geben, mit Abfrage bzw.
      // Eingabe der Zeichen über die Tastatur
      //
      int action = MM_ACCEPT;
      userWord = "";
      getyx(mainwin, posY, posX);
      do {
        mmslMorseWord(lastWord);
        intent = handleConfirmInput(confirmwin, infowin, lastWord, userWord, action);
      } while ((action == MM_REPEAT) && (intent == MM_CONTINUE));
      wmove(mainwin, posY, posX);
      errors = compareStrings(userWord, lastWord);
      if ((errors > 0) && (intent == MM_CONTINUE))
      {
        errorCount += errors;
        textModusErrorW(mainwin);
        writeStringW(mainwin, lastWord);
        textModusNormalW(mainwin);
        mmslPrepareSoundStream();
        mmslPlayErrorTone();
      }
      else
      {
        writeStringW(mainwin, lastWord);
        mmslPrepareSoundStream();
      }
 
    }
    else
    {
      //
      // Wort in Morsezeichen geben, ohne Abfrage
      //
      mmslMorseWord(lastWord);
      mmslDrainSoundStream();
      writeStringW(mainwin, lastWord);
      if (kbhit() != 0)
      {
        b = wgetch(mainwin);
        intent = handleKeyPress(b, infowin);
        while (kbhit() != 0) wgetch(mainwin);
      }
      mmslPrepareSoundStream();

      if (intent == MM_CONTINUE)
      {
        mmslPlayPauseWord();
      }
    }

    lastWord = getNextWord(wordError);
    currentLength += lastWord.size();
    if ((MM_UTF8_EOF == wordError) &&
        (MM_WM_FILE == wordMode) &&
        (MM_FALSE == fileWordsRandom))
    {
      // Wortzähler zurücksetzen
      filePosition = 0;
    }

    // Passt das neue Wort noch in die aktuelle Zeile?
    if (((int) lastWord.size())+lineCount < (screenX-1))
    {
      // Ja
      writeCharW(mainwin, ' ');
      lineCount += lastWord.size() + 1;
    }
    else
    {
      lineCount = lastWord.size();
      ++lineNumber;
      if (confirmWords == MM_TRUE)
        lineStop = screenY - 5;
      else
        lineStop = screenY - 2;
      if (lineNumber >= lineStop)
      {
        wclear(mainwin);
        wmove(mainwin, 0, 0);
        wrefresh(mainwin);
        lineNumber = 0;
      }
      else
      {
      	writeStringW(mainwin, "\n\r");
      }
    }

  } while (((currentLength <= totalLength) ||
            ((MM_WM_FILE == wordMode) &&
             (MM_FALSE == countCharsInFileMode))) && 
           (intent == MM_CONTINUE) && 
           (wordError != MM_UTF8_EOF));

  if ((currentLength > totalLength) &&
      (MM_WM_FILE == wordMode) &&
      (MM_TRUE == countCharsInFileMode) &&
      (filePosition > 0))
  {
    // Wortzähler (Datei) korrigieren, das Wort das zum
    // Abbruch geführt hat weil es 'nicht mehr reinpasst' soll
    // beim nächsten Start wieder ausgegeben werden.
    --filePosition;
  }

  writeStringW(mainwin, "\n\r+");
  mmslMorseWord("+");
  releaseWordFile();
  if (confirmWords == MM_TRUE)
  {
    writeStringW(mainwin, "   (");
    writeNumberW(mainwin, errorCount);
    writeStringW(mainwin, " Fehler)");
  }
  if (confirmWords == MM_TRUE) 
    wmove(mainwin, screenY - 6, centerX - 15);
  else
    wmove(mainwin, screenY - 4, centerX - 15);
  wrefresh(mainwin);
  writeStringW(mainwin, "<< Bitte eine Taste drücken >>");

  b = wgetch(mainwin);
  if (kbhit() != 0)
  {
    while (kbhit() != 0) wgetch(mainwin);
  }

  delwin(mainwin);
  delwin(infowin);
  if (confirmWords == MM_TRUE)
    delwin(confirmwin);

}

/** Zeigt das Menü an.
@param current Aktuelle Auswahl
*/
void mainMenu(int current)
{
  clrscr(); 
  writeSelection("*** Der Morsemann v1.4 ***",centerX-13, centerY-3, 1, 2);
  writeSelection("Dirk Bächle (dl9obn@darc.de), 2003-03-07",centerX-20, screenY, 1, 2);

  writeSelection("Start", centerX-MENU_WIDTH, centerY-1, 1, current);
  writeSelection("Morsezeichen", centerX-MENU_WIDTH, centerY, 2, current);
  writeSelection("Einstellungen", centerX-MENU_WIDTH, centerY+1, 3, current);
  writeSelection("Beenden", centerX-MENU_WIDTH, centerY+2, 4, current);
}

/** Das Hauptmenü
*/
void mainSelection(void)
{
  keyChar b = '0';
  int currentSelection = 1;

  while (1)
  {
    mainMenu(currentSelection);
    b = getch();

      /* Cursor hoch */
      if (b == KEY_UP)
      {
        if (currentSelection == 1) currentSelection = 4;
        else --currentSelection;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
        if (currentSelection == 4) currentSelection = 1;
        else ++currentSelection;
      }
    if (b == ENTER_CHAR)
    {
      switch (currentSelection)
      {
        case 1: if (selectMorseFrequencyRandomly == MM_TRUE)
                  mmslSetFrequency((mmRandom(5) + 6) * 100);
                else
                  mmslSetFrequency(fixedMorseFrequency);
                outputMorseCode();
          break;
        case 2: morseOptionsSelection();
          break;
        case 3: commonOptionsSelection();
          break;
        case 4: return;
      }
    }
  }
}

void setConfigValuesToSystem(const MMConfig &config)
{
  mmslSetBpm(config.speed);
  mmslSetDelayFactor(config.farnsworthFactor);
  totalLength = config.totalLength;
  confirmWords = config.confirmWords;
  wordMode = config.wordMode; 
  // random
  selectedCharGroup = config.charGroup;
  charSet = config.charSet;
  charSetLength = charSet.size();
  variableWords = config.variableWordLength;
  fixedWordLength = config.fixedWordLength;
  // file
  fileName = config.fileName;
  fileWordsRandom = config.fileModeWords;
  filePosition = config.filePosition;
  fileWordsExtendedCharset = config.fileUseAllChars;

  //
  // Common
  //
  selectMorseFrequencyRandomly = config.randomFrequency;
  fixedMorseFrequency = config.fixedMorseFrequency;
  mmslSetSmoothening(config.soundShaping);
  mmwlSetCountErrorsPerWord(config.errorsPerWord);
  countCharsInFileMode = config.countCharsInFileMode;
  saveOptionsToIniFile = config.saveOptions;
}

void setConfigValuesFromSystem(MMConfig &config)
{
  //
  // Morse
  //
  config.speed = mmslGetBpm();
  config.farnsworthFactor = mmslGetDelayFactor();
  config.totalLength = totalLength;
  config.confirmWords =  confirmWords;
  config.wordMode = wordMode;
  // random
  config.charGroup = selectedCharGroup;
  config.charSet = charSet;
  config.variableWordLength = variableWords;
  config.fixedWordLength = fixedWordLength;
  // file
  config.fileName = fileName;
  config.fileModeWords = fileWordsRandom;
  config.filePosition = filePosition;
  config.fileUseAllChars = fileWordsExtendedCharset;

  //
  // Common
  //
  config.randomFrequency = selectMorseFrequencyRandomly;
  config.fixedMorseFrequency = fixedMorseFrequency;
  config.soundShaping = mmslGetSmoothening();
  config.errorsPerWord = mmwlGetCountErrorsPerWord();
  config.countCharsInFileMode = countCharsInFileMode;
  config.saveOptions = saveOptionsToIniFile;
}

void printHelp()
{
    std::cout <<
            "Morsemann, " << programVersion << ", Dirk Baechle <dl9obn@darc.de>\n"
            "\n"
            "-c, --config <Dateipfad>:       Pfad auf Konfig-Datei setzen\n"
            "-d, --dump-config:              Default-Konfiguration ausgeben\n"
            "-p, --parse-file <Dateipfad>:   Datei parsen und auf Bildschirm ausgeben\n"
            "-h, --help:                     Diese Hilfe zu den Optionen anzeigen\n";
    exit(1);
}

/** Beendet den ``ncurses''-Modus.
*/
static void finish(int /* sig */)
{
  endwin();

  /* ``Non-curses'' Funktionen aufräumen */
  mmslCloseSoundSystem();

  exit(0);
}

/** Das Hauptprogramm.
*/
int main(int argc, char *argv[])
{
  // define available command line options
  const char* const short_opts = "c:dp:h";
  const option long_opts[] = {
          {"config", required_argument, nullptr, 'c'},
          {"dump-config", no_argument, nullptr, 'd'},
          {"parse-file", required_argument, nullptr, 'p'},
          {"help", no_argument, nullptr, 'h'},
          {nullptr, no_argument, nullptr, 0}
  };

  // process CLOs
  string cliConfigPath;
  string cliParseFile;
  MMConfig cliConfig;
  while (true)
  {
    const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

    if (-1 == opt)
      break;

    switch (opt)
    {
      case 'c':
        cliConfigPath = std::string(optarg);
        break;
      case 'd':
        cout << cliConfig.toString() << endl;
        return 0;
      case 'p':
        cliParseFile = std::string(optarg);
        break;
      case 'h': // -h or --help
      case '?': // Unrecognized option
      default:
        printHelp();
        break;
    }
  }

  const char *c = getenv("MORSEMANN_CONFIG");
  if( c != NULL ) 
    configPath = string( c );
  if (configPath.empty())
  { 
    string homePath;
    const char *v = getenv("HOME");
    if( v != NULL ) 
      homePath = string( v );
    if (!homePath.empty())
    {
      configPath = homePath + MM_CONFIG_FOLDER;
      // Does this path actually exist?
      struct stat sb;
      if (stat(configPath.c_str(), &sb) != 0)
      {
        // Else set config path to empty again
        configPath = "";
      }
    }
    configPath += MM_CONFIG_FILE;
  }

  if (!cliConfigPath.empty())
  {
    configPath = cliConfigPath;
  }

  config.readFromFile(configPath);
  initialConfig = config;
  setConfigValuesToSystem(config);
 
  if (!cliParseFile.empty())
  {
    parseUtf8FileToStdout(cliParseFile);
    return 0;
  }

  if (!mmslInitSoundSystem(MMSL_ALSA))
    return 1;

  (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

  (void) initscr();      /* initialize the curses library */
  keypad(stdscr, TRUE);  /* enable keyboard mapping */
  (void) nonl();         /* tell curses not to do NL->CR/NL on output */
  (void) cbreak();       /* take input chars one at a time, no wait for \n */
  (void) noecho();       /* no echo input */

#ifndef NO_COLORS
  if (has_colors())
  {
    start_color();

    /*
     * Simple color assignment, often all we need.  Color pair 0 cannot
     * be redefined.  This example uses the same value for the color
     * pair as for the foreground color, though of course that is not
     * necessary:
     */
    init_pair(1, COLOR_RED,     COLOR_BLACK);
    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
    init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(4, COLOR_BLUE,    COLOR_BLACK);
    init_pair(5, COLOR_WHITE,   COLOR_BLUE);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE,   COLOR_BLACK);
  }
#endif /* of #ifndef NO_COLORS */

  hideCursor();
  textModusNormal();

  clrscr();
  srandom((unsigned int) time(NULL));
  mainSelection();
  textModusNormal();

  if (config.saveOptions == MM_TRUE)
  {
    setConfigValuesFromSystem(config);
    if (config != initialConfig)
    {
      config.writeFile(configPath);
    }
  }

  confirmWords = 0;
  byeMessage();
  showCursor();

  finish(0);               /* we're done */

  return(0);
}
