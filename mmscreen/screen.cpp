
/* Benutzt conio.h usw. statt der */
/* `ncurses' Funktionen, falls gesetzt. */
/* #define DOS */

/* Gibt die Umlaute in Textausgaben ASCII-codiert */
/* aus, falls gesetzt. */
/* #define ASCII */

/* Benutzt keine Farben, sondern nur das */
/* (hoffentlich) vom Terminal unterst"utzte ``Highlighting'' */
/* #define NO_COLORS */

/*-------------------------------------------------------- Includes */

#include <stdio.h>
#include <stdlib.h>

#ifdef DOS
#include <dos.h>
#include <conio.h>
#include <string.h>
#else
#include <string.h>
#include <unistd.h>
#include <curses.h>
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

