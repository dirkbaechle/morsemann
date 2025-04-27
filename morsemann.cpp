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

/*-------------------------------------------------------- Includes */

#include "mmsound.h"
#include "mmscreen.h"
#include "mmword.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

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

/** Die Endnachricht.
*/
void byeMessage(void)
{
  clrscr();
  gotoxy(centerX - 6, centerY);
  mmslSetFrequency((mmRandom(5) + 6) * 100);
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

    /* Cursor hoch */
    if (b == KEY_UP)
    {
      bpmUp();
    }

    /* Cursor runter */
    if (b == KEY_DOWN)
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
      writeSelection(charSet.c_str(), centerX-(charSetLength/2), centerY+6, 1, 2);
    else
      writeSelection(charSet.c_str(), 0, centerY+6, 1, 2);
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

int handleKeyPress(int b)
{
  int error = MM_CONTINUE;
  switch (b)
  {
    case KEY_BACKSPACE:
    case KEY_ESCAPE: error = MM_ESCAPE;
                      break;
    case KEY_UP: // Tone up
                    toneUp();
                    break;
    case KEY_DOWN: // Tone down
                    toneDown();
                    break;
    case KEY_LEFT: // Slower
                    bpmDown();
                    break;
    case KEY_RIGHT: // Faster
                    bpmUp();
                    break;
  }

  return error;
}

int handleConfirmInput(WINDOW *confirmwin, const string &lastWord, string &userWord, int &action)
{
  unsigned int b = 0;
  int error = MM_CONTINUE;
  if (kbhit() != 0)
  {
    b = wgetch(confirmwin);
    error = handleKeyPress(b);
    while (kbhit() != 0) wgetch(confirmwin);
  }
  if (error == MM_CONTINUE)
  {
    action = confirmString(confirmwin, 1, 1, lastWord.size()+1, userWord);
    if (action == MM_REPEAT)
      mmslPrepareSoundStream();
  }

  return error;
}

/** Ausgabe der Morsezeichen, entsprechend der Optionen.
*/
void outputMorseCode(void)
{
  int currentLength = 0;
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

  clrscr();

  // Windows erzeugen
  WINDOW *mainwin = nullptr;
  WINDOW *confirmwin = nullptr;
  if (confirmChars == MM_TRUE)
  {
    mainwin = newwin(screenY - 3, screenX, 0, 0);
    confirmwin = newwin(3, screenX, screenY - 3, 0);
    keypad(confirmwin, TRUE);  /* enable keyboard mapping */
    box(confirmwin, 0, 0);
    wrefresh(confirmwin);
  }
  else
  {
    mainwin = newwin(screenY, screenX, 0, 0);
  }
  keypad(mainwin, TRUE);  /* enable keyboard mapping */
  wrefresh(mainwin);

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
      //
      // Wort in Morsezeichen geben, mit Abfrage bzw.
      // Eingabe der Zeichen über die Tastatur
      //
      int action = MM_ACCEPT;
      userWord = "";
      getyx(mainwin, posY, posX);
      do {
        mmslMorseWord(lastWord);
        intent = handleConfirmInput(confirmwin, lastWord, userWord, action);
      } while ((action == MM_REPEAT) && (intent == MM_CONTINUE));
      wmove(mainwin, posY, posX);
      errors = compareStrings(userWord, lastWord);
      if (errors > 0)
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
        intent = handleKeyPress(b);
        while (kbhit() != 0) wgetch(mainwin);
      }
      mmslPrepareSoundStream();

      if (intent == MM_CONTINUE)
      {
        mmslPlayPauseWord();
      }
    }

    lastWord = getNextWord();
    currentLength += lastWord.size();

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
      if (confirmChars == MM_TRUE)
        lineStop = screenY - 4;
      else
        lineStop = screenY - 1;
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

  } while ((currentLength <= totalLength) && (intent == MM_CONTINUE));

  writeStringW(mainwin, "\n\r+");
  mmslMorseWord("+");
  if (confirmChars == MM_TRUE)
  {
    writeStringW(mainwin, "   (");
    writeNumberW(mainwin, errorCount);
    writeStringW(mainwin, " Fehler)");
  }
  if (confirmChars == MM_TRUE) 
    wmove(mainwin, screenY - 5, centerX - 15);
  else
    wmove(mainwin, screenY - 3, centerX - 15);
  wrefresh(mainwin);
  writeStringW(mainwin, "<< Bitte eine Taste drücken >>");

  b = wgetch(mainwin);
  if (kbhit() != 0)
  {
    while (kbhit() != 0) wgetch(mainwin);
  }

  delwin(mainwin);
  if (confirmChars == MM_TRUE)
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
