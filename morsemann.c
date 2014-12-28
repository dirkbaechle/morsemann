/* Morsemann - Ein kleines Programm zum Lernen und Üben des
*              Hörens von Morsezeichen (CW).
*
* Copyright (C) 2003 by Dirk Baechle (dl9obn@darc.de)
*
* http://www.darc.de/distrikte/h/43/programme/morsemann
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
Der ``Morsemann''. Ein kleines Programm zum Lernen und "Uben des H"orens
von Morsezeichen (CW).
\author Dirk B"achle
\version 1.1
\date 2003-03-07
*/

/* Benutzt conio.h usw. statt der */
/* `ncurses' Funktionen, falls gesetzt. */
/* #define DOS */

/* Gibt die Umlaute in Textausgaben ASCII-codiert */
/* aus, falls gesetzt. */
/* #define ASCII */

/* Benutzt keine Farben, sondern nur das */
/* (hoffentlich) vom Terminal unterst"utzte ``Highlighting'' */
/* #define NO_COLORS */

/* Falls gesetzt, wird fortlaufend das Standardwort */
/* `paris' erzeugt. Dient der "Uberpr"ufung der */
/* Geschwindigkeitsangaben. */
/* #define CALIBRATE_MODE */

/*-------------------------------------------------------- Includes */

#include <stdio.h>
#include <stdlib.h>

#include "mmsound.h"

#ifdef DOS
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#else
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <signal.h>
#endif

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

#ifdef DOS
typedef char keyChar;
#else
typedef unsigned int keyChar;
#endif

/*---------------------------------------------------- Const values */

#ifdef DOS
const int KEY_UP = 72;
const int KEY_DOWN = 80;
const int KEY_LEFT = 75;
const int KEY_RIGHT = 77;
const int KEY_BACKSPACE = 8;
#endif

const int MM_TRUE = 1;
const int MM_FALSE = 0;

const int GROUP_WIDTH = 15;
const int OPT_LEFT_WIDTH = 19;
const int OPT_RIGHT_WIDTH = 8;
const int MENU_WIDTH = 4;

/*------------------------------------------------ Global variables */

/** Tonh"ohe */
int tone;
/** L"ange eines Punktes in Millisekunden */
int dotLength = 100;
/** Geschwindigkeit in Buchstaben pro Minute (bpm) */
int bpm = 60;
/** Pausenfaktor */
int delayFactor = 1;
/** Auswahl der Zeichenmenge (1-8) */
int selectedCharGroup = 1;
/** Art der Wortgruppen (fest=MM_FALSE oder variabel=MM_TRUE) */
int variableWords = 0;
/** L"ange der festen Wortgruppen */
int fixedWordLength = 5;
/** Best"atigung jedes einzelnen Zeichens? (0=nein, 1=Buchstabe, 2=Wort) */
int confirmChars = 0;
/** Gesamtzahl der zu gebenden Buchstaben */
int totalLength = 200;
/** Gesamtzahl der gemachten Fehler */
int errorCount = 0;
/** Zeichenmenge als String (Auswahl = 8) */
char charSet[255];
/** L"ange des Zeichenmenge-Strings */
int charSetLength = 0;
/** Array mit den Strings f"ur die Zeichenauswahl */
char groupString[8][50] = {"Alle Zeichen",
			   "Nur Buchstaben",
			   "Nur Zahlen",
			   "Nur Sonderzeichen",
			   "Buchstaben und Zahlen",
			   "Buchstaben und Sonderzeichen",
			   "Zahlen und Sonderzeichen",
			   "Zeichen eingeben"};

/** Aktuelle Breite des Windows */
int screenX = 80;
/** Aktuelle H"ohe des Windows */
int screenY = 25;
/** Aktuelle Mitte des Windows in X-Richtung */
int centerX = 40;
/** Aktuelle Mitte des Windows in Y-Richtung */
int centerY = 12;

/*--------------------------------------------------- Functions */

/** Schreibt den String \a string an die aktuelle
Position auf dem Bildschirm.
@param string String
*/
void writeString(char *string)
{
#ifdef DOS
  cprintf("%s", string);
#else
  printw("%s", string);
  refresh();
#endif
}

/** Schreibt die Zahl \a number an die aktuelle
Position auf dem Bildschirm.
@param number Zahl
*/
void writeNumber(int number)
{
#ifdef DOS
  cprintf("%d", number);
#else
  printw("%d", number);
  refresh();
#endif
}

/** Schreibt den Buchstaben \a b an die aktuelle
Position auf dem Bildschirm.
@param b Buchstabe
*/
void writeChar(char b)
{
#ifdef DOS
  cprintf("%c", b);
#else
  addch(b);
  refresh();
#endif
}

#ifndef DOS
/** Bewegt den Cursor an die Position \a xpos, \a ypos.
@param xpos X-Koordinate
@param ypos Y-Koordinate
*/
void gotoxy(int xpos, int ypos)
{
  move(ypos-1, xpos-1);
}

/** L"oscht den aktuellen Bildschirm und ermittelt
die neuen Bildschirmgr"o"sen und -mitten.
*/
void clrscr()
{
  clear();
  getmaxyx(stdscr, screenY, screenX);
  centerX = screenX / 2;
  centerY = screenY / 2;
}

/** Pr"uft ob ein Zeichen im Tastaturpuffer vorliegt.
@return 0 falls kein Zeichen vorliegt, 1 sonst
*/
int kbhit(void)
{   /* Ohne ``Delay'' */
    static struct timeval tv = {0, 0};
    fd_set rdfs;

    /* Wurde eine Taste gedr"uckt? */
    FD_ZERO (&rdfs);
    FD_SET  (STDERR_FILENO, &rdfs);
    if (select  (STDERR_FILENO + 1, &rdfs, NULL, NULL, &tv) <= 0)
        return 0; /* Nein */
    if (FD_ISSET (STDERR_FILENO, &rdfs))
        return 1; /* Ja */
    return 0;     /* Nein */
}
#endif

/** Schreibt den String \a string im Format \a fmt
an die Position \a xpos, \a ypos.
*/
void moveWrite(int ypos, int xpos, char *fmt, char *string)
{
#ifdef DOS
  gotoxy(xpos, ypos);
  cprintf(fmt, string);
#else
  mvprintw(ypos-1, xpos-1, fmt, string);
#endif
}

/** Erzeugt einen Ton von der L"ange eines Punktes.
*/
void dit(void)
{
  mmslPlayTone(dotLength);
  mmslPlayPause(dotLength);
}


/** Erzeugt einen Ton von der L"ange eines Striches
(= Drei Punkte).
*/
void dah(void)
{
  mmslPlayTone(dotLength*3);
  mmslPlayPause(dotLength);
}

/** Gibt einen Fehlerton aus.
*/
void errorTone(void)
{
  mmslPlayErrorTone(dotLength);
}

/** Erzeugt eine Zufallszahl im Bereich von 0 bis \a maxNumber minus
1.
@param maxNumber Anzahl der m"oglichen Zufallszahlen
@return Die Zufallszahl
*/
int mmRandom(int maxNumber)
{
#ifdef DOS
  return ((int) (maxNumber*1.0*rand()/(RAND_MAX+1.0)));
#else
  return ((int) (maxNumber*1.0*random()/(RAND_MAX+1.0)));
#endif
}

/** Schaltet in den ``normalen'' Text-Modus, d.h.
weisse Schrift auf schwarzem Grund.
*/
void textModusNormal(void)
{
#ifdef NO_COLORS
#ifdef DOS
  textcolor(WHITE);
  textbackground(BLACK);
#else
  attroff(A_STANDOUT);
#endif
#else
#ifdef DOS
  textcolor(WHITE);
  textbackground(BLACK);
#else
  attrset(COLOR_PAIR(7));
#endif
#endif
}

/** Schaltet in den Text-Modus f"ur ``Auswahl'', d.h.
weisse Schrift auf blauem Grund.
*/
void textModusSelect(void)
{
#ifdef NO_COLORS
#ifdef DOS
  textcolor(WHITE);
  textbackground(BLACK);
#else
  attron(A_STANDOUT);
#endif
#else
#ifdef DOS
  textcolor(WHITE);
  textbackground(BLUE);
#else
  attrset(COLOR_PAIR(5));
#endif
#endif
}

/** Schaltet in den Text-Modus f"ur ``Fehler'', d.h.
rote Schrift auf schwarzem Grund.
*/
void textModusError(void)
{
#ifdef NO_COLORS
#ifdef DOS
  textcolor(WHITE);
  textbackground(BLACK);
#else
  attron(A_STANDOUT);
#endif
#else
#ifdef DOS
  textcolor(RED);
  textbackground(BLACK);
#else
  attrset(COLOR_PAIR(1));
#endif
#endif
}

/** Gibt das Zeichen aus und fragt es vorher eventuell ab.
@param b Das zu schreibende Zeichen
@return 1 wenn durch ESC abgebrochen, 0 sonst
*/
int writeSign(char b)
{
  keyChar letter='0';

  if (confirmChars == 1)
  {
    /* Mit Abfrage */
    letter = getch();
    if (letter == b)
    {
      /* Buchstabe richtig */
      textModusNormal();
      writeChar(b);
    }
    else
    {
      if (letter == KEY_BACKSPACE) return(MM_TRUE);
      /* Buchstabe falsch */
      textModusError();
      writeChar(b);
      textModusNormal();
      errorTone();
      errorCount++;
    }
  }
  else
  {
    /* Keine Abfrage? */
    if (confirmChars == 0)
      writeChar(b);
    /* Bei Abfrage von ganzen W"ortern (confirmChars == 2) werden die */
    /* Buchstaben zusammen in `outputMorseCode' ausgegeben... */
  }

  return(MM_FALSE);
}

/** Gibt die Zeichen und den dazugeh"origen Morse-Code aus.
@param signID Das auszugebende Zeichen
@return 1 wenn durch ESC abgebrochen, 0 sonst
*/
int outputSign(int signID)
{
  switch (signID)
  {
    case 97:dit();dah();return(writeSign('a'));
    case 98:dah();dit();dit();dit();return(writeSign('b'));
    case 99:dah();dit();dah();dit();return(writeSign('c'));
    case 100:dah();dit();dit();return(writeSign('d'));
    case 101:dit();return(writeSign('e'));
    case 102:dit();dit();dah();dit();return(writeSign('f'));
    case 103:dah();dah();dit();return(writeSign('g'));
    case 104:dit();dit();dit();dit();return(writeSign('h'));
    case 105:dit();dit();return(writeSign('i'));
    case 106:dit();dah();dah();dah();return(writeSign('j'));
    case 107:dah();dit();dah();return(writeSign('k'));
    case 108:dit();dah();dit();dit();return(writeSign('l'));
    case 109:dah();dah();return(writeSign('m'));
    case 110:dah();dit();return(writeSign('n'));
    case 111:dah();dah();dah();return(writeSign('o'));
    case 112:dit();dah();dah();dit();return(writeSign('p'));
    case 113:dah();dah();dit();dah();return(writeSign('q'));
    case 114:dit();dah();dit();return(writeSign('r'));
    case 115:dit();dit();dit();return(writeSign('s'));
    case 116:dah();return(writeSign('t'));
    case 117:dit();dit();dah();return(writeSign('u'));
    case 118:dit();dit();dit();dah();return(writeSign('v'));
    case 119:dit();dah();dah();return(writeSign('w'));
    case 120:dah();dit();dit();dah();return(writeSign('x'));
    case 121:dah();dit();dah();dah();return(writeSign('y'));
    case 122:dah();dah();dit();dit();return(writeSign('z'));
    case 48:dah();dah();dah();dah();dah();return(writeSign('0'));
    case 49:dit();dah();dah();dah();dah();return(writeSign('1'));
    case 50:dit();dit();dah();dah();dah();return(writeSign('2'));
    case 51:dit();dit();dit();dah();dah();return(writeSign('3'));
    case 52:dit();dit();dit();dit();dah();return(writeSign('4'));
    case 53:dit();dit();dit();dit();dit();return(writeSign('5'));
    case 54:dah();dit();dit();dit();dit();return(writeSign('6'));
    case 55:dah();dah();dit();dit();dit();return(writeSign('7'));
    case 56:dah();dah();dah();dit();dit();return(writeSign('8'));
    case 57:dah();dah();dah();dah();dit();return(writeSign('9'));
    case 44:dah();dah();dit();dit();dah();dah();return(writeSign(','));
    case 46:dit();dah();dit();dah();dit();dah();return(writeSign('.'));
    case 63:dit();dit();dah();dah();dit();dit();return(writeSign('?'));
    case 47:dah();dit();dit();dah();dit();return(writeSign('/'));
    case 61:dah();dit();dit();dit();dah();return(writeSign('='));
  }
  return(MM_FALSE);
}

/** Versteckt den Text-Cursor.
*/
void hideCursor(void)
{
#ifdef DOS
  union REGS inregs;

  inregs.h.ah=0x01;
  inregs.h.ch=0xF0;
  inregs.h.cl=0x02;

  int86(0x10,&inregs,&inregs);
#else
  curs_set(0);
#endif
}

/** Zeigt den Text-Cursor.
*/
void showCursor(void)
{
#ifdef DOS
  union REGS inregs;

  inregs.h.ah=0x01;
  inregs.h.ch=0x06;
  inregs.h.cl=0x07;

  int86(0x10,&inregs,&inregs);
#else
  curs_set(1);
#endif
}

/** Die Endnachricht.
*/
void byeMessage(void)
{
  clrscr();
  gotoxy(centerX - 6, centerY);
  tone = (mmRandom(7)+6)*100;
  mmslSetFrequency(tone);
  /* Setze Geschwindigkeit auf 120 bpm */
  dotLength = 50;
  outputSign('7');
  mmslPlayPause(dotLength*2);
  outputSign('3');
  mmslPlayPause(dotLength*6);
  writeChar(' ');
  outputSign('d');
  mmslPlayPause(dotLength*2);
  outputSign('e');
  mmslPlayPause(dotLength*6);
  writeChar(' ');
  outputSign('d');
  mmslPlayPause(dotLength*2);
  outputSign('l');
  mmslPlayPause(dotLength*2);
  outputSign('9');
  mmslPlayPause(dotLength*2);
  outputSign('o');
  mmslPlayPause(dotLength*2);
  outputSign('b');
  mmslPlayPause(dotLength*2);
  outputSign('n');
  mmslPlayPause(500);
}

/** Ermittelt den zugeh"origen ANSI/ASCII-Kode des Zeichens mit
der ID \a letterID.
@param letterID ID des Zeichens
@return ANSI/ASCII-Kode des Zeichens
*/
char mapToChar(int letterID)
{
  if ((letterID > 0) && (letterID < 27))
    return ((char) letterID+96);
  if ((letterID > 26) && (letterID < 37))
    return ((char) letterID+21);
  if (letterID == 37)
    return ((char) 44);
  if (letterID == 38)
    return ((char) 46);
  if (letterID == 39)
    return ((char) 63);
  if (letterID == 40)
    return ((char) 47);
  if (letterID == 41)
    return ((char) 61);

  return(0);
}

/** Bestimmt durch Zufall das n"achste Zeichen aus dem eingegebenen String.
@return Zeichen
*/
char charSetRandom(void)
{
  return charSet[mmRandom(charSetLength)];
}

/** Bestimmt durch Zufall das n"achste Zeichen.
@return Zeichen
*/
char signRandom(void)
{
  switch (selectedCharGroup)
  {
     case 1: return(mapToChar(mmRandom(41)+1));
     case 2: return(mapToChar(mmRandom(26)+1));
     case 3: return(mapToChar(mmRandom(10)+27));
     case 4: return(mapToChar(mmRandom(5)+37));
     case 5: return(mapToChar(mmRandom(36)+1));
     case 6: if (mmRandom(2) == 0)
	     {
	       return(mapToChar(mmRandom(26)+1));
	     }
	     else
	     {
	       return(mapToChar(mmRandom(5)+37));
	     }
     case 7: return(mmRandom(15)+27);
     case 8: return(charSetRandom());
  }

  return(0);
}

/** Schreibt einen String an die "ubergebene Position. Stimmt
die Auswahl mit der ID des Strings "uberein wird dieser mit
weisser Schrift auf blauem Grund dargestellt.
@param string String
@param xpos Position in x-Richtung
@param ypos Position in y-Richtung
@param stringID ID-Nummer des Strings
@param selected Aktuelle Auswahl
*/
void writeSelection(char *string, int xpos, int ypos, 
                    int stringID, int selected)
{
  if (stringID == selected)
    textModusSelect();

  moveWrite(ypos, xpos, "%s", string);

  if (stringID == selected)
    textModusNormal();
}

/** Liest einen String an der Position (xpos,ypos) mit maximal
\a max Buchstaben linksb"undig ein.
@param xpos x-Koordinate
@param ypos y-Koordinate
@param max Maximale Anzahl der Buchstaben
@param string Zeiger auf den String
*/
void readString(int xpos, int ypos, int max, char *string)
{
  keyChar letter = '0';
  char stringCopy[255];
  int stringLength = strlen(string);

  showCursor();

  strcpy(stringCopy, string);

  moveWrite(ypos, xpos, "%s", stringCopy);

  while ((letter != 27) && (letter != 13))
  {
    letter = getch();
#ifdef DOS
    if (letter == 0)
    {
      letter = getch();
    }
    else
    {
#endif

      if (letter == KEY_BACKSPACE)
      {
	if (stringLength > 0)
	{
	  stringLength--;
	  stringCopy[stringLength] = 0;
          moveWrite(ypos, xpos, "%s ", stringCopy);
          gotoxy(xpos+stringLength, ypos);
	}
      }
      else
      {
        if (letter > 31)
        {
	  if (stringLength < max)
	  {
	    stringCopy[stringLength] = letter;
	    stringLength++;
	    stringCopy[stringLength] = 0;
	    moveWrite(ypos, xpos, "%s", stringCopy);
	  }
        }
      }
#ifdef DOS
    }
#endif
  }
  if (letter != 27)
  {
    strcpy(string, stringCopy);
  }

  hideCursor();

}

/** Liest eine Nummer an der Position (xpos,ypos) mit maximal
max Stellen linksb"undig ein.
@param xpos x-Koordinate
@param ypos y-Koordinate
@param max Maximale Anzahl der Stellen
@param number Zeiger auf die Nummer
*/
void readNumber(int xpos, int ypos, int max, int *number)
{
  keyChar letter = '0';
  char copyString[50];
  int stringLength = 0;

  showCursor();

  sprintf(copyString, "%d", *number);
  stringLength = strlen(copyString);

  moveWrite(ypos, xpos, "%s", copyString);

  while ((letter != 27) && (letter != 13))
  {
    letter = getch();
#ifdef DOS
    if (letter == 0)
    {
      letter = getch();
    }
    else
    {
#endif
      if (letter == KEY_BACKSPACE)
      {
	if (stringLength > 0)
	{
	  stringLength--;
	  copyString[stringLength] = 0;
	  moveWrite(ypos, xpos, "%s ", copyString);
          gotoxy(xpos+stringLength, ypos);
	}
      }
      else
      {
        if ((letter > 47) && (letter < 58))
        {
	  if (stringLength < max)
	  {
	    copyString[stringLength] = letter;
	    stringLength++;
	    copyString[stringLength] = 0;
	    moveWrite(ypos, xpos, "%s", copyString);
	  }
        }
      }
#ifdef DOS
    }
#endif
  }

  if (letter != 27)
  {
    sscanf(copyString, "%ld", number);
  }

  hideCursor();

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
	errorTone();
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
  keyChar b='0';

  clrscr();
  writeSelection("Auswahl der Zeichen",centerX-10,centerY-5,1,2);

  while ((b != KEY_BACKSPACE) && (b != 13))
  {
    charGroupMenu();
    b = getch();

#ifdef DOS
    if (b == 0)
    {
      b = getch();
#endif
      /* Cursor hoch */
      if (b == KEY_UP)
      {
	if (selectedCharGroup == 1) selectedCharGroup = 8;
	else selectedCharGroup--;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
	if (selectedCharGroup == 8) selectedCharGroup = 1;
	else selectedCharGroup++;
      }
#ifdef DOS
    }
#endif
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
  keyChar b='0';

  clrscr();
  writeSelection("Auswahl der Geschwindigkeit", centerX-15, centerY-1, 1, 2);

  textModusSelect();

  while ((b != KEY_BACKSPACE) && (b != 13))
  {
    gotoxy(centerX-3, centerY+1);
    if (bpm < 100)
      writeChar(' ');

    writeNumber(bpm);
    writeString(" BpM");

    b = getch();

#ifdef DOS
    if (b == 0)
    {
      b = getch();
#endif
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
#ifdef DOS
    }
#endif
  }

  textModusNormal();

  dotLength = (int) (6000/bpm);

}

/** Auswahl des Pausenfaktors.
*/
void delaySelection(void)
{
  keyChar b='0';

  clrscr();
  writeSelection("Auswahl des Pausenfaktors", centerX-11, centerY-1, 1, 2);

  textModusSelect();

  while ((b != KEY_BACKSPACE) && (b != 13))
  {
    gotoxy(centerX-5, centerY+1);
    writeNumber(delayFactor);
    writeString(" x Pause");

    b = getch();

#ifdef DOS
    if (b == 0)
    {
      b = getch();
#endif
      /* Cursor hoch */
      if (b == KEY_UP)
      {
	if (delayFactor == 9) delayFactor = 1;
	else delayFactor++;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
	if (delayFactor == 1) delayFactor = 9;
	else delayFactor--;
      }
#ifdef DOS
    }
#endif
  }

  textModusNormal();

}

/** Eingabe der Gesamtanzahl der auszugebenden Buchstaben.
*/
void lengthSelection(void)
{
  clrscr();
  writeSelection("Gesamtanzahl der Buchstaben (mindestens 5!)",centerX-22,centerY-1,1,2);
  do
  {
    readNumber(centerX-2, centerY+1, 4, &totalLength);
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
#ifdef ASCII
  writeSelection("Zeichen best„tigen:", centerX-OPT_LEFT_WIDTH, centerY+3, 6, akt);
#else
  writeSelection("Zeichen bestätigen:", centerX-OPT_LEFT_WIDTH, centerY+3, 6, akt);
#endif
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-2);
  writeString(groupString[selectedCharGroup - 1]);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY-1);
  writeNumber(bpm);
  gotoxy(centerX+OPT_RIGHT_WIDTH, centerY);
  writeNumber(delayFactor);
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
    case 1: writeString("Buchstaben");
            break;
    case 2: writeString("Worte");
            break;
  }

  /* Zeichenmenge anzeigen, falls Option "Zeichen eingeben" gew„hlt */
  if (selectedCharGroup == 8)
  {
    writeSelection("Zeichenmenge:", centerX-6, centerY+5, 1, 2);
    if ((centerX-(charSetLength/2)) > 0)
      writeSelection(charSet, centerX-(charSetLength/2), centerY+6, 1, 2);
    else
      writeSelection(charSet, 0, centerY+6, 1, 2);
  }

}


/** Auswahl f"ur die Optionen.
*/
void optionsSelection(void)
{
  keyChar b='0';
  int currentOption = 1;

  while (b != KEY_BACKSPACE)
  {
    optionsMenu(currentOption);
    b = getch();

#ifdef DOS
    if (b == 0)
    {
      b = getch();
#endif
      /* Cursor hoch */
      if (b == KEY_UP)
      {
	if (currentOption == 1) currentOption = 6;
	else currentOption--;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
	if (currentOption == 6) currentOption = 1;
	else currentOption++;
      }

      /* Cursor rechts */
      if (b == KEY_RIGHT)
      {
	if ((currentOption == 5) &&
             (variableWords == MM_FALSE))
	{
          if (fixedWordLength < 8) 
            fixedWordLength++;
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
            fixedWordLength--;
          else
            fixedWordLength = 8;
        } 
      }

#ifdef DOS
    }
#endif
    if (b == 13)
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
	case 6: if (confirmChars < 2)
		  confirmChars++;
                else
		  confirmChars = 0;
		break;
      }
    }
  }
}

/** Vergleicht die beiden Strings, gibt das richtige Wort
\a lastWord aus und liefert die Anzahl der gemachten Fehler
zur"uck.
@param userWord Eingabe des Benutzers
@param lastWord Das richtige Wort
@return Anzahl der gemachten Fehler
*/
int compareStrings(char *userWord, char *lastWord)
{
  int pos = 0;
  int errors = 0;

  while (userWord[pos] != 0)
  {
    if (userWord[pos] != lastWord[pos])
    {
      textModusError();
      writeChar(lastWord[pos]);
      errors++;
      textModusNormal();
    }
    else
    {
      writeChar(lastWord[pos]);
    }
    pos++;
  }

  /* Sind noch Zeichen "ubrig? */
  if (lastWord[pos] != 0)
  {
    textModusError();
    while (lastWord[pos] != 0)
    {
      writeChar(lastWord[pos]);
      pos++;
      errors++;
    }
    textModusNormal();

  }

  if (errors > 0)
    errorTone();

  return errors;
}

/** Ausgabe der Morsezeichen, entsprechend der Optionen.
*/
void outputMorseCode(void)
{
  int currentLength = 0;
  int wordLength = fixedWordLength;
  int charCount = 0;
  int lineCount = 0;
  int lineNumber = 0;
  int posX = 1;
  int posY = 1;
  char lastSign = 0;
  char lastWord[10];
  char userWord[10];
  int error = MM_FALSE;
  keyChar b = '0';

  clrscr();
#ifndef DOS
  refresh();
#endif
  mmslPlayPause(1000);

  errorCount = 0;

  if (variableWords == MM_TRUE)
    wordLength = mmRandom(6) + 3;

  lastWord[0] = 0;
  userWord[0] = 0;
  do
  {
    do
    {
      charCount++;
      currentLength++;
      lineCount++;
#ifdef CALIBRATE_MODE
      switch (lastSign)
      {
	case 0:error = outputSign('p');
	       break;
	case 1:error = outputSign('a');
	       break;
	case 2:error = outputSign('r');
	       break;
	case 3:error = outputSign('i');
	       break;
	case 4:error = outputSign('s');
	       break;
      }

      lastSign++;
      if (lastSign == 5) lastSign = 0;
#else
      lastSign = signRandom();
      error = outputSign(lastSign);

      if (confirmChars == 2)
      {
	lastWord[charCount-1] = lastSign;
      }
#endif
      mmslPlayPause(dotLength*2+(delayFactor-1)*3);
      if (kbhit() != 0)
      {
	b = getch();
#ifdef DOS
	if (b == 0) b = getch();
#endif
	if (b == KEY_BACKSPACE) error = MM_TRUE;
      }
    } while ((charCount < wordLength) && (error == MM_FALSE));

    if ((confirmChars == 2) && (error == MM_FALSE))
    {
      lastWord[charCount] = 0;
      userWord[0] = 0;
#ifdef DOS
      posX = wherex();
      posY = wherey();
#else
      getyx(stdscr, posY, posX);
      posX++;
      posY++;
#endif
      readString(posX, posY, wordLength, userWord);
      gotoxy(posX, posY);
      errorCount += compareStrings(userWord, lastWord);
    }

    charCount = 0;

    if (variableWords == MM_TRUE)
      wordLength = mmRandom(6) + 3;

    if (wordLength+lineCount < (screenX-1))
    {
      writeChar(' ');
      lineCount++;
    }
    else
    {
      lineCount = 0;
      lineNumber++;
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

    mmslPlayPause(dotLength*4*delayFactor);
  } while ((currentLength < totalLength) && (error == MM_FALSE));

  if (kbhit() != 0)
  {
    while (kbhit() != 0) getch();
  }

  writeString("\n\r");
  dit();dah();dit();dah();dit();writeChar('+');
  mmslPlayPause(dotLength*2);
  writeChar(' ');dit();dit();dit();dah();dit();dah();writeString("sk");
  if (confirmChars != 0)
  {
    writeString("   (");
    writeNumber(errorCount);
    writeString(" Fehler)");
  }
#ifdef ASCII
  writeSelection("<< Bitte eine Taste drcken >>", centerX-15, screenY, 1, 2);
#else
  writeSelection("<< Bitte eine Taste drücken >>", centerX-15, screenY, 1, 2);
#endif


  b = getch();
#ifdef DOS
  if (b == 0) getch();
#endif
}

/** Zeigt das Men"u an.
@param current Aktuelle Auswahl
*/
void mainMenu(int current)
{
  clrscr(); 
  writeSelection("*** Der Morsemann v1.1 ***",centerX-13, centerY-3, 1, 2);
#ifdef ASCII
  writeSelection("by Dirk B„chle (dl9obn@darc.de), 07.03.2003",centerX-23, screenY, 1, 2);
#else
  writeSelection("by Dirk Bächle (dl9obn@darc.de), 07.03.2003",centerX-23, screenY, 1, 2);
#endif

  writeSelection("Start", centerX-MENU_WIDTH, centerY-1, 1, current);
  writeSelection("Optionen", centerX-MENU_WIDTH, centerY, 2, current);
  writeSelection("Beenden", centerX-MENU_WIDTH, centerY+1, 3, current);
}

/** Das Hauptmen"u
*/
void mainSelection(void)
{
  keyChar b='0';
  int currentSelection = 1;

  while (1)
  {
    mainMenu(currentSelection);
    b = getch();

#ifdef DOS
    if (b == 0)
    {
      b = getch();
#endif
      /* Cursor hoch */
      if (b == KEY_UP)
      {
	if (currentSelection == 1) currentSelection = 3;
	else currentSelection--;
      }

      /* Cursor runter */
      if (b == KEY_DOWN)
      {
	if (currentSelection == 3) currentSelection = 1;
	else currentSelection++;
      }
#ifdef DOS
    }
#endif
    if (b == 13)
    {
      switch (currentSelection)
      {
	case 1: tone = (mmRandom(7) + 6) * 100;
                mmslSetFrequency(tone);
		outputMorseCode();
		break;
	case 2: optionsSelection();
		break;
	case 3: return;
      }
    }
  }

}

#ifndef DOS
/** Beendet den ``ncurses''-Modus.
*/
static void finish(int sig)
{
  endwin();

  /* ``Non-curses'' Funktionen aufr"aumen */
  mmslCloseSoundSystem();

  exit(0);
}
#endif

/** Das Hauptprogramm.
*/
int main(void)
{
  if (0 != mmslInitSoundSystem())
    return 1;

#ifdef DOS
  hideCursor();
  textModusNormal();

  clrscr();
  randomize();
  mainSelection();
  confirmChars = 0;
  textModusNormal();
  byeMessage();
  showCursor();

  clrscr();
#else
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
#endif /* of #ifdef DOS ... #else ... */

  return(0);
}

