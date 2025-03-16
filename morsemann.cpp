/* Morsemann - Ein kleines Programm zum Lernen und Üben des
*              Hörens von Morsezeichen (CW).
*
* Copyright (C) 2003-2024 by Dirk Baechle (dl9obn@darc.de)
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

/* Falls gesetzt, wird fortlaufend das Standardwort */
/* `paris' erzeugt. Dient der Überprüfung der */
/* Geschwindigkeitsangaben. */
/* #define CALIBRATE_MODE */

/*-------------------------------------------------------- Includes */

#include "mmsound.h"
#include "mmscreen.h"
#include "mmword.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

#include <cstring>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <signal.h>

using std::map;
using std::string;

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

const int GROUP_WIDTH = 15;
const int OPT_LEFT_WIDTH = 19;
const int OPT_RIGHT_WIDTH = 8;
const int MENU_WIDTH = 4;

/*------------------------------------------------ Global variables */

/** Gesamtzahl der zu gebenden Buchstaben */
int totalLength = 200;
/** Gesamtzahl der gemachten Fehler */
int errorCount = 0;

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

/** Gibt die Zeichen und den dazugehörigen Morse-Code aus.
@param signID Das auszugebende Zeichen
@return 1 wenn das Zeichen unbekannt ist (Fehler), 0 sonst
*/
int outputSign(int signID)
{
  if (MM_TRUE == mmslMorseChar(signID))
    return MM_TRUE;
  mmslPlayPauseDits(2);
  map<int, string>::const_iterator c_it = cwChar.find(signID);
  if (c_it == cwChar.end())
      return MM_TRUE;

  writeString(c_it->second);
  return MM_FALSE;
}

/** Die Endnachricht.
*/
void byeMessage(void)
{
  clrscr();
  gotoxy(centerX - 6, centerY);
  mmslSetFrequency((mmRandom(5) + 6) * 100);
  mmslSetBpm(160);
  mmslPrepareSoundStream();
  // TODO implement playMorseWord
  outputSign('7');
  mmslPlayPauseDits(2);
  outputSign('3');
  mmslPlayPauseDits(6);
  writeChar(' ');
  outputSign('d');
  mmslPlayPauseDits(2);
  outputSign('e');
  mmslPlayPauseDits(6);
  writeChar(' ');
  outputSign('d');
  mmslPlayPauseDits(2);
  outputSign('l');
  mmslPlayPauseDits(2);
  outputSign('9');
  mmslPlayPauseDits(2);
  outputSign('o');
  mmslPlayPauseDits(2);
  outputSign('b');
  mmslPlayPauseDits(2);
  outputSign('n');
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

    readString(1, centerY+1, 75, charSet);

    charSetLength = strlen(charSet);

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
      if (selectedCharGroup == 1) selectedCharGroup = 8;
      else --selectedCharGroup;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (selectedCharGroup == 8) selectedCharGroup = 1;
      else ++selectedCharGroup;
    }
  }

  if (selectedCharGroup == 8)
  {
    readCharSet();
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
  unsigned int bpm = mmslGetBpm();

  while ((b != KEY_BACKSPACE) && (b != ENTER_CHAR))
  {
    gotoxy(centerX-3, centerY+1);
    if (bpm < 100)
      writeChar(' ');

    writeNumber(bpm);
    writeString(" BpM");

    b = getch();

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (bpm == 250)
        bpm = 10;
      else
      {
        if (bpm >= 180)
          bpm += 10;
        else
          bpm += 5;
      }
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (bpm == 10)
        bpm = 250;
      else
      {
        if (bpm > 180)
          bpm -= 10;
        else
          bpm -= 5;
      }
    }
  }

  textModusNormal();

  mmslSetBpm(bpm);
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

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (delayFactor == 9) delayFactor = 1;
      else ++delayFactor;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
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
    totalLength = readNumber(centerX-2, centerY+1, 4, totalLength);
  } while (totalLength < 5);
}

/** Zeigt die aktuell eingestellten Optionen an.
@param akt Aktuelle Auswahl
*/
void optionsMenu(int akt)
{
  clrscr();
  writeSelection("*** Optionen ***", centerX-8, centerY-4, 1, 2);

  writeSelection("Zeichen:", centerX-OPT_LEFT_WIDTH, centerY-2, 1, akt);
  writeSelection("Geschwindigkeit (in BpM):", centerX-OPT_LEFT_WIDTH, centerY-1, 2, akt);
  writeSelection("Pausenfaktor:", centerX-OPT_LEFT_WIDTH, centerY, 3, akt);
  writeSelection("Zeichenanzahl:", centerX-OPT_LEFT_WIDTH, centerY+1, 4, akt);
  writeSelection("Feste Wortgruppen:", centerX-OPT_LEFT_WIDTH, centerY+2, 5, akt);
  writeSelection("Zeichen bestätigen:", centerX-OPT_LEFT_WIDTH, centerY+3, 6, akt);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-2);
  writeString(groupString[selectedCharGroup - 1]);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-1);
  writeNumber(mmslGetBpm());
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY);
  writeNumber(mmslGetDelayFactor());
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+1);
  writeNumber(totalLength);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+2);
  if (variableWords == MM_FALSE)
  {
    writeString("Ja (");
    writeNumber(fixedWordLength);
    writeString(")");
  }
  else
    writeString("Nein (3-8)");
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY+3);
  switch (confirmChars)
  {
    case 0: writeString("Nein");
            break;
    case 1: writeString("Worte");
            break;
  }

  /* Zeichenmenge anzeigen, falls Option "Zeichen eingeben" gewählt */
  if (selectedCharGroup == 8)
  {
    writeSelection("Zeichenmenge:", centerX-6, centerY+5, 1, 2);
    if ((centerX-(charSetLength/2)) > 0)
      writeSelection(charSet, centerX-(charSetLength/2), centerY+6, 1, 2);
    else
      writeSelection(charSet, 0, centerY+6, 1, 2);
  }

}


/** Auswahl für die Optionen.
*/
void optionsSelection(void)
{
  keyChar b = '0';
  int currentOption = 1;

  while (b != KEY_BACKSPACE)
  {
    optionsMenu(currentOption);
    b = getch();

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      if (currentOption == 1) currentOption = 6;
      else --currentOption;
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
    {
      if (currentOption == 6) currentOption = 1;
      else ++currentOption;
    }

    /* Cursor rechts */
    if (b == KEY_RIGHT)
    {
      if ((currentOption == 5) &&
          (variableWords == MM_FALSE))
      {
        if (fixedWordLength < 8) 
          ++fixedWordLength;
        else
          fixedWordLength = 2;
      } 
    }

    /* Cursor links */
    if (b == KEY_LEFT)
    {
      if (currentOption == 5)
      {
        if (fixedWordLength > 2) 
          --fixedWordLength;
        else
          fixedWordLength = 8;
      } 
    }

    if (b == ENTER_CHAR)
    {
      switch (currentOption)
      {
        case 1: charGroupSelection();
          break;
        case 2: speedSelection();
          break;
        case 3: delaySelection();
          break;
        case 4: lengthSelection();
          break;
        case 5: variableWords = 1 - variableWords;
          break;
        case 6: confirmChars = 1 - confirmChars;
          break;
      }
    }
  }
}

/** Ausgabe der Morsezeichen, entsprechend der Optionen.
*/
void outputMorseCode(void)
{
  int currentLength = 0;
  int lineCount = 0;
  int lineNumber = 0;
  int posX = 1;
  int posY = 1;
  string lastWord;
  string userWord;
  int error = MM_FALSE;
  int errors = 0;
  keyChar b = '0';

  clrscr();
  refresh();
  mmslPrepareSoundStream();
  mmslPlayPause(1000);

  errorCount = 0;

  lastWord = getNextWord();
  currentLength += lastWord.size();
  lineCount = lastWord.size();
  do
  {
    if (confirmChars == MM_TRUE)
    {
      userWord = "";
      getyx(stdscr, posY, posX);
      ++posX;
      ++posY;
      mmslMorseWord(lastWord);
      // TODO add option to repeat word!
      userWord = readString(posX, posY, lastWord.size()+1, userWord);
      gotoxy(posX, posY);
      errors = compareStrings(userWord, lastWord);
      if (errors > 0)
      {
        errorCount += errors;
        textModusError();
        writeString(lastWord);
        textModusNormal();
        mmslPrepareSoundStream();
        mmslPlayErrorTone();
      }
      else
      {
        writeString(lastWord);
        mmslPrepareSoundStream();
      }
 
    }
    else
    {
      mmslMorseWord(lastWord);
      mmslDrainSoundStream();
      writeString(lastWord);
      if (kbhit() != 0)
      {
        b = getch();
        if ((b == KEY_BACKSPACE) || (b == KEY_ESCAPE))
        {
          error = MM_TRUE;
        }
        while (kbhit() != 0) getch();
        mmslPrepareSoundStream();
      }
      else
      {
        mmslPrepareSoundStream();
        mmslPlayPauseWord();
      }
    }

    lastWord = getNextWord();
    currentLength += lastWord.size();

    // Passt das neue Wort noch in die aktuelle Zeile?
    if (((int) lastWord.size())+lineCount < (screenX-1))
    {
      // Ja
      writeChar(' ');
      lineCount += lastWord.size() + 1;
    }
    else
    {
      lineCount = lastWord.size();
      ++lineNumber;
      if (lineNumber >= (screenY-1))
      {
        clrscr();
        lineNumber = 0;
      }
      else
      {
      	writeString("\n\r");
      }
    }

  } while ((currentLength < totalLength) && (error == MM_FALSE));

  if (kbhit() != 0)
  {
    while (kbhit() != 0) getch();
  }

  writeString("\n\r");
  outputSign('+');
  if (confirmChars != 0)
  {
    writeString("   (");
    writeNumber(errorCount);
    writeString(" Fehler)");
  }
  writeSelection("<< Bitte eine Taste drücken >>", centerX-15, screenY, 1, 2);

  b = getch();
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
  writeSelection("Optionen", centerX-MENU_WIDTH, centerY, 2, current);
  writeSelection("Beenden", centerX-MENU_WIDTH, centerY+1, 3, current);
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
        if (currentSelection == 1) currentSelection = 3;
        else --currentSelection;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
        if (currentSelection == 3) currentSelection = 1;
        else ++currentSelection;
      }
    if (b == ENTER_CHAR)
    {
      switch (currentSelection)
      {
        case 1: mmslSetFrequency((mmRandom(5) + 6) * 100);
          outputMorseCode();
          break;
        case 2: optionsSelection();
          break;
        case 3: return;
      }
    }
  }
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
int main(void)
{
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
  confirmChars = 0;
  textModusNormal();
  byeMessage();
  showCursor();

  finish(0);               /* we're done */

  return(0);
}
