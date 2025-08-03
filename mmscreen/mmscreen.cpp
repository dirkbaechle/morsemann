
/*-------------------------------------------------------- Includes */

#include "mmscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <sstream>  

#include <unistd.h>

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

/** Schreibt den String \a string an die aktuelle
Position auf dem Fenster \a curwin.
@param curwin Fenster für die Ausgabe
@param string String
*/
void writeStringW(WINDOW *curwin, const string& str)
{
  wprintw(curwin, "%s", str.c_str());
  wrefresh(curwin);
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

/** Schreibt die Zahl \a number an die aktuelle
Position auf dem Fenster \a curwin.
@param curwin Fenster für die Ausgabe
@param number Zahl
*/
void writeNumberW(WINDOW *curwin, int number)
{
  wprintw(curwin, "%d", number);
  wrefresh(curwin);
}

/** Schreibt die Zahl \a number an die aktuelle
Position auf dem Bildschirm.
@param number Zahl
*/
void writeNumberULong(unsigned long int number)
{
  printw("%lu", number);
  refresh();
}

/** Schreibt die Zahl \a number an die aktuelle
Position auf dem Fenster \a curwin.
@param curwin Fenster für die Ausgabe
@param number Zahl
*/
void writeNumberWULong(WINDOW *curwin, unsigned long int number)
{
  wprintw(curwin, "%lu", number);
  wrefresh(curwin);
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

/** Schreibt den Buchstaben \a b an die aktuelle
Position auf dem Fenster \a curwin.
@param curwin Fenster für die Ausgabe
@param b Buchstabe
*/
void writeCharW(WINDOW *curwin, char b)
{
  waddch(curwin, b);
  wrefresh(curwin);
}

/** Bewegt den Cursor an die Position \a xpos, \a ypos.
@param xpos X-Koordinate
@param ypos Y-Koordinate
*/
void gotoxy(int xpos, int ypos)
{
  move(ypos - 1, xpos - 1);
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
  mvprintw(ypos - 1, xpos - 1, fmt, str.c_str());
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

/** Schaltet in den ``normalen'' Text-Modus, d.h.
weisse Schrift auf schwarzem Grund im Fenster \a curwin .
*/
void textModusNormalW(WINDOW *curwin)
{
#ifdef NO_COLORS
  wattroff(curwin, A_STANDOUT);
#else
  wattrset(curwin, COLOR_PAIR(7));
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

/** Schaltet in den Text-Modus für ``Fehler'', d.h.
rote Schrift auf schwarzem Grund im Fenster \a curwin .
*/
void textModusErrorW(WINDOW *curwin)
{
#ifdef NO_COLORS
  wattron(curwin, A_STANDOUT);
#else
  wattrset(curwin, COLOR_PAIR(1));
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

/** Liest einen String an der Position (xpos,ypos) im
Fenster \a curwin mit maximal \a max Buchstaben linksbündig ein.
@warning Erlaubt bei der Eingabe keine Leerzeichen!
@param curwin Fenster
@param xpos x-Koordinate
@param ypos y-Koordinate
@param max Maximale Anzahl der Buchstaben
@param str Aktueller/Neuer String
@return Fehlercode, MM_ACCEPT oder MM_REPEAT oder MM_ESCAPE
*/
int confirmString(WINDOW *curwin, int xpos, int ypos, int max, string& str)
{
  keyChar letter = '0';
  string stringCopy(str);
  int stringLength = stringCopy.size();

  showCursor();

  wmove(curwin, ypos, xpos);
  wprintw(curwin, "%s", stringCopy.c_str());
  wrefresh(curwin);

  while ((letter != KEY_ESCAPE) && 
         (letter != ENTER_CHAR) &&
         (letter != KEY_REPEAT_MORSE))
  {
    letter = wgetch(curwin);
    if (kbhit() != 0)
    {
      while (kbhit() != 0) wgetch(curwin);
    }

    if (letter == KEY_BACKSPACE)
    {
      if (stringLength > 0)
      {
        --stringLength;
        stringCopy = stringCopy.substr(0, stringLength);
        wmove(curwin, ypos, xpos);
        wprintw(curwin, "%s ", stringCopy.c_str());
        wmove(curwin, ypos, xpos+stringLength);
        wrefresh(curwin);
      }
    }
    else
    {
      // 'Printable' Buchstabe und kein Leerzeichen? 
      if ((letter > 32) && (letter != KEY_REPEAT_MORSE))
      {
        if (stringLength < max)
        {
          stringCopy += letter;
          ++stringLength;
          wmove(curwin, ypos, xpos);
          wprintw(curwin, "%s", stringCopy.c_str());
          wrefresh(curwin);
        }
      }
    }
  }
  hideCursor();

  wclear(curwin);
  box(curwin, 0, 0);
  wrefresh(curwin);

  switch (letter)
  {
    case KEY_ESCAPE: // Abbruch
                     return MM_ESCAPE;
    case KEY_REPEAT_MORSE: // Nochmal die Morsezeichen abspielen
                 return MM_REPEAT;
    case ENTER_CHAR: // Accept -> Geänderte Kopie des Strings übernehmen
                    str = stringCopy;
                    return MM_ACCEPT;
  }

  return MM_ESCAPE;
}

/** Liest einen String an der Position (xpos,ypos) mit maximal
\a max Buchstaben linksbündig ein.
@warning Erlaubt die Eingabe von Leerzeichen im String!
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

  while ((letter != KEY_ESCAPE) && 
         (letter != ENTER_CHAR))
  {
    letter = getch();

    if (letter == KEY_BACKSPACE)
    {
      if (stringLength > 0)
      {
        --stringLength;
        stringCopy = stringCopy.substr(0, stringLength);
        moveWrite(ypos, xpos, "%s ", stringCopy);
        gotoxy(xpos+stringLength, ypos);
      }
    }
    else
    {
      // 'Printable' Buchstabe oder Leerzeichen
      if (letter > 31)
      {
        if (stringLength < max)
        {
          stringCopy += letter;
          ++stringLength;
          moveWrite(ypos, xpos, "%s", stringCopy);
        }
      }
    }
  }
  if (letter == KEY_ESCAPE)
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
unsigned long int readNumber(int xpos, int ypos, int max, unsigned long int number)
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

  while ((letter != KEY_ESCAPE) && (letter != ENTER_CHAR))
  {
    letter = getch();
    if (letter == KEY_BACKSPACE)
    {
      if (stringLength > 0)
      {
        --stringLength;
        stringCopy = stringCopy.substr(0, stringLength);
        moveWrite(ypos, xpos, "%s ", stringCopy);
        gotoxy(xpos+stringLength, ypos);
      }
    }
    else
    {
      // Ist der Buchstabe eine 'Nummer'?
      if (isdigit(letter))
      {
        if (stringLength < max)
        {
          stringCopy += letter;
          ++stringLength;
          moveWrite(ypos, xpos, "%s", stringCopy);
        }
      }
    }
  }

  if (letter != KEY_ESCAPE)
  {
    // String zu int konvertieren
    stream.clear();
    stream << stringCopy;
    stream >> number;
  }

  hideCursor();

  return number;
}
