
/*-------------------------------------------------------- Includes */

#include "mmscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <sstream>  

#include <unistd.h>
#include <curses.h>

using std::string;

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

/** Aktuelle Breite des Windows */
int screenX = 80;
/** Aktuelle Höhe des Windows */
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
void writeString(const string& str)
{
  printw("%s", str.c_str());
  refresh();
}

/** Schreibt die Zahl \a number an die aktuelle
Position auf dem Bildschirm.
@param number Zahl
*/
void writeNumber(int number)
{
  printw("%d", number);
  refresh();
}

/** Schreibt den Buchstaben \a b an die aktuelle
Position auf dem Bildschirm.
@param b Buchstabe
*/
void writeChar(char b)
{
  addch(b);
  refresh();
}

/** Bewegt den Cursor an die Position \a xpos, \a ypos.
@param xpos X-Koordinate
@param ypos Y-Koordinate
*/
void gotoxy(int xpos, int ypos)
{
  move(ypos-1, xpos-1);
}

/** Löscht den aktuellen Bildschirm und ermittelt
die neuen Bildschirmgrößen und -mitten.
*/
void clrscr()
{
  clear();
  getmaxyx(stdscr, screenY, screenX);
  centerX = screenX / 2;
  centerY = screenY / 2;
}

/** Prüft ob ein Zeichen im Tastaturpuffer vorliegt.
@return 0 falls kein Zeichen vorliegt, 1 sonst
*/
int kbhit(void)
{   /* Ohne ``Delay'' */
    static struct timeval tv = {0, 0};
    fd_set rdfs;

    /* Wurde eine Taste gedrückt? */
    FD_ZERO (&rdfs);
    FD_SET  (STDERR_FILENO, &rdfs);
    if (select  (STDERR_FILENO + 1, &rdfs, NULL, NULL, &tv) <= 0)
        return 0; /* Nein */
    if (FD_ISSET (STDERR_FILENO, &rdfs))
        return 1; /* Ja */
    return 0;     /* Nein */
}

/** Schreibt den String \a string im Format \a fmt
an die Position \a xpos, \a ypos.
*/
void moveWrite(int ypos, int xpos, const char *fmt, const string& str)
{
  mvprintw(ypos-1, xpos-1, fmt, str.c_str());
}

/** Schaltet in den ``normalen'' Text-Modus, d.h.
weisse Schrift auf schwarzem Grund.
*/
void textModusNormal(void)
{
#ifdef NO_COLORS
  attroff(A_STANDOUT);
#else
  attrset(COLOR_PAIR(7));
#endif
}

/** Schaltet in den Text-Modus für ``Auswahl'', d.h.
weiße Schrift auf blauem Grund.
*/
void textModusSelect(void)
{
#ifdef NO_COLORS
  attron(A_STANDOUT);
#else
  attrset(COLOR_PAIR(5));
#endif
}

/** Schaltet in den Text-Modus für ``Fehler'', d.h.
rote Schrift auf schwarzem Grund.
*/
void textModusError(void)
{
#ifdef NO_COLORS
  attron(A_STANDOUT);
#else
  attrset(COLOR_PAIR(1));
#endif
}

/** Versteckt den Text-Cursor.
*/
void hideCursor(void)
{
  curs_set(0);
}

/** Zeigt den Text-Cursor.
*/
void showCursor(void)
{
  curs_set(1);
}

/** Schreibt einen String an die übergebene Position. Stimmt
die Auswahl mit der ID des Strings überein wird dieser mit
weisser Schrift auf blauem Grund dargestellt.
@param str String
@param xpos Position in x-Richtung
@param ypos Position in y-Richtung
@param stringID ID-Nummer des Strings
@param selected Aktuelle Auswahl
*/
void writeSelection(const string& str, int xpos, int ypos, 
                    int stringID, int selected)
{
  if (stringID == selected)
    textModusSelect();

  moveWrite(ypos, xpos, "%s", str);

  if (stringID == selected)
    textModusNormal();
}

/** Liest einen String an der Position (xpos,ypos) mit maximal
\a max Buchstaben linksbündig ein.
@param xpos x-Koordinate
@param ypos y-Koordinate
@param max Maximale Anzahl der Buchstaben
@param str Aktueller String
@return Neuer String
*/
string readString(int xpos, int ypos, int max, const string& str)
{
  keyChar letter = '0';
  string stringCopy(str);
  int stringLength = stringCopy.size();

  showCursor();

  moveWrite(ypos, xpos, "%s", stringCopy);

  while ((letter != 27) && (letter != 13))
  {
    letter = getch();

      if (letter == KEY_BACKSPACE)
      {
	if (stringLength > 0)
	{
	  stringLength--;
	  stringCopy = stringCopy.substr(0, stringLength);
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
	    stringCopy += letter;
	    stringLength++;
	    moveWrite(ypos, xpos, "%s", stringCopy);
	  }
        }
      }
  }
  if (letter == 27)
  {
    // Abbruch -> Alten String wiederherstellen
    stringCopy = str;
  }

  hideCursor();

  return stringCopy;
}

/** Liest eine Nummer an der Position (xpos,ypos) mit maximal
max Stellen linksbündig ein.
@param xpos x-Koordinate
@param ypos y-Koordinate
@param max Maximale Anzahl der Stellen
@param number Aktuelle Nummer
@return Neue Nummer
*/
int readNumber(int xpos, int ypos, int max, int number)
{
  keyChar letter = '0';
  // Int zu string konvertieren
  std::stringstream stream;
  stream << number;
  string stringCopy;
  stream >> stringCopy;
  int stringLength = stringCopy.size();

  showCursor();

  moveWrite(ypos, xpos, "%s", stringCopy);

  while ((letter != 27) && (letter != 13))
  {
    letter = getch();
      if (letter == KEY_BACKSPACE)
      {
	if (stringLength > 0)
	{
	  stringLength--;
	  stringCopy = stringCopy.substr(0, stringLength);
          moveWrite(ypos, xpos, "%s ", stringCopy);
          gotoxy(xpos+stringLength, ypos);
	}
      }
      else
      {
        if ((letter > 47) && (letter < 58))
        {
	  if (stringLength < max)
	  {
	    stringCopy += letter;
	    stringLength++;
	    moveWrite(ypos, xpos, "%s", stringCopy);
	  }
        }
      }
  }

  if (letter != 27)
  {
    // String zu int konvertieren
    stream.clear();
    stream << stringCopy;
    stream >> number;
  }

  hideCursor();

  return number;
}
