
/*-------------------------------------------------------- Includes */

#include "mmword.h"
#include "utf8file.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

using std::string;
using std::vector;

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

/** Auswahl der Zeichenmenge (1-8) */
int selectedCharGroup = CG_ALL_CHARS;
/** Art der Wortgruppen (0=fest, 1=variabel) */
int variableWords = 0;
/** Länge der festen Wortgruppen */
int fixedWordLength = 5;
/** Bestätigung/Abfrage der einzelnen Wörter? (0=nein, 1=ja) */
int confirmWords = 0;
/** Werden Fehler pro Wort gezählt? (0=nein jeder Buchstabe einzeln, 1=ja) */
int countErrorsPerWord = MM_TRUE;
/** Zeichenmenge als String (Auswahl Zeichenmenge = 8) */
string charSet;
/** Länge des Zeichenmenge-Strings */
int charSetLength = 0;
/** Array mit den Strings für die Zeichenauswahl */
string groupString[8] = {"Alle Zeichen",
			   "Nur Buchstaben",
			   "Nur Zahlen",
			   "Nur Sonderzeichen",
			   "Buchstaben und Zahlen",
			   "Buchstaben und Sonderzeichen",
			   "Zahlen und Sonderzeichen",
			   "Zeichen eingeben"};

/** Modus für das Erzeugen neuer Wörter. (0=zufällig, 1=aus Datei, 2=PARIS)*/
int wordMode = MM_WM_RANDOM;
/** Name der aktuellen Datei, aus der neue Wörter entnommen werden sollen. */
string fileName;
/** Der volle (aufgelöste) Pfad auf die aktuelle Datei, aus der neue Wörter entnommen werden sollen. */
string filePath;
/** Enthält die Datei einzelne Worte pro Zeile, die zufällig ausgegeben werden
 * sollen, oder ist es ein Text der zusammenhängend gegeben wird (0=nein/text, 1=ja/zufällig)?
 */
int fileWordsRandom = MM_TRUE;
/** Position within the currently set file (counted per word). */
unsigned long int filePosition = 0;
/** Sollen bei der Augabe aus einer Datei alle bekannten Zeichen gegeben werden,
 * oder nur die DTP-relevanten (0=nein/dtp, 1=ja/alle)?
 */
int fileWordsExtendedCharset = MM_FALSE;
/** Worte die aus der gegebenen Datei gelesen wurden und zufällig
 * ausgegeben werden sollen.
 */
vector<string> randomWords;

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
     case CG_ALL_CHARS: return(mapToChar(mmRandom(41)+1));
     case CG_LETTERS_ONLY: return(mapToChar(mmRandom(26)+1));
     case CG_DIGITS_ONLY: return(mapToChar(mmRandom(10)+27));
     case CG_PUNCT_ONLY: return(mapToChar(mmRandom(5)+37));
     case CG_LETTERS_AND_DIGITS: return(mapToChar(mmRandom(36)+1));
     case CG_LETTERS_AND_PUNCT: if (mmRandom(2) == 0)
	     {
	       return(mapToChar(mmRandom(26)+1));
	     }
	     else
	     {
	       return(mapToChar(mmRandom(5)+37));
	     }
     case CG_DIGITS_AND_PUNCT: return(mmRandom(15)+27);
     default: return(charSetRandom());
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

  if (countErrorsPerWord)
  {
    // Ist das gesamte Wort richtig?
    if (userWord.compare(lastWord) != 0)
    {
      errors = 1;
    }
  }
  else
  {
    // Fehler pro Buchstabe zählen
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
  }
  return errors;
}

/** Ermittelt das nächste Wort, basierend auf den aktuell gewählten
Einstellungen (Wortlänge, Zeichenmenge usw.) und liefert dieses
als String zurück. 
*/
string getNextWord(int &error)
{
  string word;

  error = MM_UTF8_WORD;
  if (wordMode == MM_WM_PARIS)
    return string("paris");

  if (MM_WM_FILE == wordMode)
  {
    if (MM_TRUE == fileWordsRandom)
    {
      return randomWords[mmRandom(randomWords.size())];
    }
    else
    {
      ++filePosition;
      return readUtf8WordFromOpenFile(error);
    }
  }

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

/** Öffnet vor der Ausgabe der Morsezeichen die Eingabedatei
 * im UTF8-Format, falls ein Dateiname angegeben wurde.
 * Im Random-Modus werden alle Worte aus der Datei in einen
 * vector<string> gelesen aus dem dann zufällig Worte ausgewählt
 * werden. Im Text-Modus werden die einzelnen Worte sequenziell
 * ausgegeben bis die maximale Zeichenanzahl oder das Ende der
 * Datei erreicht ist.
 */
void prepareWordFile()
{
  if (MM_WM_FILE == wordMode)
  {
    int error;
    string word;
    if (MM_FALSE == fileWordsRandom)
    {
      openUtf8File();
      if (filePosition > 0)
      {
        // try to fast-forward the file
        unsigned long readWords = 0;

        do
        {
          word = readUtf8WordFromOpenFile(error);
          if (error == MM_UTF8_EOF)
          {
            filePosition = 0;
            closeUtf8File();
            openUtf8File();
            break;
          }
          ++readWords;
        } while (readWords < filePosition);
      }
    }
    else
    {
      openUtf8File();
      randomWords.clear();
      do
      {
        word = readUtf8WordFromOpenFileVerbatim(error);
        if ((error != MM_UTF8_EOF) && (word.size() > 0))
        {
          vector<string>::const_iterator s_it = randomWords.begin();
          for (; s_it != randomWords.end(); ++s_it)
          {
            if (*s_it == word)
            {
              break;
            }
          }
          if (s_it == randomWords.end())
          {
            // new word
            randomWords.push_back(word);
          }
        }
      } while (error != MM_UTF8_EOF);
      closeUtf8File();
    }
  }
}

/** Schließt nach der Ausgabe der Morsezeichen
 * die ggf. noch geöffnete Eingabedatei.
 */
void releaseWordFile()
{
  if ((MM_WM_FILE == wordMode) &&
      (MM_FALSE == fileWordsRandom))
  {
    closeUtf8File();
  }
  randomWords.clear();
}

void mmwlSetCountErrorsPerWord(int countWords)
{
  countErrorsPerWord = countWords;
}

int mmwlGetCountErrorsPerWord()
{
  return countErrorsPerWord;
}
