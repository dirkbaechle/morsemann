
/*-------------------------------------------------------- Includes */

#include "mmword.h"

#include <stdio.h>
#include <stdlib.h>

using std::string;

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

/** Auswahl der Zeichenmenge (1-8) */
int selectedCharGroup = 1;
/** Art der Wortgruppen (fest=MM_FALSE oder variabel=MM_TRUE) */
int variableWords = 0;
/** Länge der festen Wortgruppen */
int fixedWordLength = 5;
/** Bestätigung/Abfrage der einzelnen Wörter? (0=nein, 1=ja) */
int confirmChars = 0;
/** Zeichenmenge als String (Auswahl = 8) */
char charSet[255];
/** Länge des Zeichenmenge-Strings */
int charSetLength = 0;
/** Array mit den Strings für die Zeichenauswahl */
char groupString[8][50] = {"Alle Zeichen",
			   "Nur Buchstaben",
			   "Nur Zahlen",
			   "Nur Sonderzeichen",
			   "Buchstaben und Zahlen",
			   "Buchstaben und Sonderzeichen",
			   "Zahlen und Sonderzeichen",
			   "Zeichen eingeben"};
/** Kalibrierungsmodus, liefert immer nur das Wort "paris". */
int calibrateMode = MM_FALSE;

/*--------------------------------------------------- Functions */

/** Erzeugt eine Zufallszahl im Bereich von 0 bis \a maxNumber minus
1.
@param maxNumber Anzahl der möglichen Zufallszahlen
@return Die Zufallszahl
*/
int mmRandom(int maxNumber)
{
  return ((int) (maxNumber*1.0*random()/(RAND_MAX+1.0)));
}

/** Ermittelt den zugehörigen ANSI/ASCII-Kode des Zeichens mit
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

/** Bestimmt durch Zufall das nächste Zeichen aus dem eingegebenen String.
@return Zeichen
*/
char charSetRandom(void)
{
  return charSet[mmRandom(charSetLength)];
}

/** Bestimmt durch Zufall das nächste Zeichen.
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

/** Vergleicht die beiden Strings und liefert die Anzahl
der gemachten Fehler zurück.
@param userWord Eingabe des Benutzers
@param lastWord Das richtige Wort
@return Anzahl der gemachten Fehler
*/
int compareStrings(const string &userWord, const string &lastWord)
{
  size_t pos = 0;
  int errors = 0;

  while ((pos < userWord.size()) && (pos < lastWord.size()))
  {
    if (userWord[pos] != lastWord[pos])
    {
      ++errors;
    }
    ++pos;
  };

  /* Sind noch Zeichen übrig? */
  if (pos < lastWord.size())
  {
    errors += lastWord.size() - pos;
  }

  return errors;
}


/** Ermittelt das nächste Wort, basierend auf den aktuell gewählten
Einstellungen (Wortlänge, Zeichenmenge usw.) und liefert dieses
als String zurück. 
*/
string getNextWord()
{
  string word;

  if (calibrateMode == MM_TRUE)
    return string("paris");

  int wordLength = fixedWordLength;
  int charCount = 0;

  if (variableWords == MM_TRUE)
    wordLength = mmRandom(6) + 3;

  do
  {
    ++charCount;
    word += signRandom();

  } while (charCount < wordLength);

  return word;
}
