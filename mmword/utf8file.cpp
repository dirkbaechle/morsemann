
/*-------------------------------------------------------- Includes */

#include "utf8file.h"
#include "mmword.h"

#include <iostream>
#include <fstream>

using std::string;
using std::cout;

/*--------------------------------------------------------- Defines */

#define PM_VOID 0
#define PM_WORD 1
#define PM_SPACE 2
#define PM_SPACE_CONT 3
#define PM_PUNCT 4
#define PM_PUNCT_CONT 5
#define PM_PUNCT_BEHIND 6
#define PM_PUNCT_BEHIND_CONT 7

#define TT_NONE 0
#define TT_CHAR 1
#define TT_SPACE 2
#define TT_PUNCT 3
#define TT_PUNCTEXT 4

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

// Stream auf die aktuelle UTF8 Datei
std::ifstream file;
// Der aktuelle Parsing-Modus innerhalb der Datei
int utf8ParseMode = PM_VOID;
// Das letzte gelesene UTF8 Token.
string utf8LastToken = "";
// Liste der erlaubten Sonderzeichen im ASCII-Bereich (1 Byte)
string punctuationChars = ",.?/=!\"$'()+-:;@`";
// Liste der erlaubten Zeichen die zwischen (Halb)Sätzen stehen und
// daher eine Klammerung durch Apostrophe fortsetzen dürfen (1 Byte)
string separatorChars = ",.?!:;";
// Liste der Apostroph Zeichen
string apostrophChars = "\"'`";

/*--------------------------------------------------- Functions */

/** Liefert 'true' (1 == MM_TRUE) wenn die UTF8 Datei nicht
 * leer ist, also ausgebbare Worte enthält,
 * sonst 'false' (0 == MM_FALSE).
 */
int utf8FileContainsWords()
{
  // TODO read a single word from the file,
  // open it first if necessary, and then
  // close it.

  return MM_TRUE;
}

/** Liefert 'true' (1 == MM_TRUE) wenn die UTF8 Datei geöffnet ist,
 * sonst 'false' (0 == MM_FALSE).
 */
int utf8FileIsOpen()
{
  if (!file) {
    return MM_FALSE;
  }
  return MM_TRUE;
}

/** Öffnet die UTF8 Datei und bereitet sie dadurch
 * für das Lesen von Worten vor.
 */
int openUtf8File()
{
  utf8ParseMode = PM_VOID;
  file.open(fileName, std::ios::binary);
  if (!file) {
    return MM_FALSE;
  }
  return MM_TRUE;
}

/** Schließt die aktuelle UTF8 Datei. */
void closeUtf8File()
{
  utf8ParseMode = PM_VOID;
  file.close();
}

/** Hilfsfunktion zum Bestimmen der Anzahl Bytes in einem
 * UTF8 Char, basierend auf dem ersten Byte.
 */
int utf8CharLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0) return 1;           // 0xxxxxxx
    if ((firstByte & 0xE0) == 0xC0) return 2;        // 110xxxxx
    if ((firstByte & 0xF0) == 0xE0) return 3;        // 1110xxxx
    if ((firstByte & 0xF8) == 0xF0) return 4;        // 11110xxx
    return -1; // Invalid UTF-8 start byte
}

/** Liest ein einzelnes UTF8 Zeichen aus der bereits
 * geöffneten Datei.
 */
string readUtf8Char(int &error)
{
  while (!file.eof()) {
    unsigned char firstByte;
    file.read(reinterpret_cast<char*>(&firstByte), 1);

    int charLen = utf8CharLength(firstByte);
    if (charLen == -1) {
      error = MM_UTF8_INVALID_STARTBYTE;
      return "";
    }

    if ((charLen > 1) && (file.eof()))
      break;

    std::string utf8Char(1, firstByte);
    for (int i = 1; i < charLen; ++i) {
      unsigned char nextByte;
      file.read(reinterpret_cast<char*>(&nextByte), 1);
      if (file.eof() || (nextByte & 0xC0) != 0x80) {
        error = MM_UTF8_INVALID_CONTBYTE;
        return "";
      }
      utf8Char += static_cast<char>(nextByte);
    }
  
    error = MM_UTF8_CHAR;
    return utf8Char;
  }

  error = MM_UTF8_EOF;
  return "";
}

/** Liest ein UTF8 Byte aus der bereits geöffneten
 * UTF8 Datei, konvertiert Umlaute und klassifiziert
 * den Token-Typ.
 */
string getUtf8Token(int &type, int &error)
{
  if (file.eof())
  {
    type = TT_NONE;
    error = MM_UTF8_EOF;
    return "";
  }

  string word;
  int charError;
  string utf8Char = readUtf8Char(charError);
  if (charError == MM_UTF8_CHAR)
  {
    if (utf8Char.size() == 1)
    {
      // classify character
      unsigned char c = utf8Char[0];

      if (isspace(c))
      {
        // Whitespace
        word += c;
        type = TT_SPACE;
      }
      else if (isalnum(c))
      {
        // Buchstabe/Zahl
        if ((c >= 65) && (c <= 90))
        {
          // Groß- -> Kleinbuchstabe
          c += 32;
        }
        word += c;
        type = TT_CHAR;
      }
      else
      {
        // Anderes Zeichen
        std::size_t found = punctuationChars.find(c);
        if (found != string::npos)
        {
          if (fileWordsExtendedCharset == MM_FALSE)
          {
            // DTP-Modus, nur hinzufügen wenn eines
            // der ersten 5 Punktuationszeichen...
            if (found < 5)
            {
              word += c;
              type = TT_PUNCT;
            }
            else
            {
              type = TT_NONE;
            }
          }
          else
          {
            word += c;
            if (found < 5)
            {
              type = TT_PUNCT;
            }
            else
            {
              type = TT_PUNCTEXT;
            }
          }
        }
        else
        {
          type = TT_NONE;
        }
      }
    }
    else
    {
      // MultiChar UTF8: Generell behandeln als unbekannt
      type = TT_NONE;

      // Prüfen auf Umlaute
      if ((utf8Char.size() == 2) &&
          (((unsigned char) utf8Char[0]) == 195))
      {
        unsigned int secondByte = (unsigned char) utf8Char[1];

        switch (secondByte)
        {
          case 164: // ä
          case 132:
              word += "ae";
              type = TT_CHAR;
              break;
          case 182: // ö
          case 150:
              word += "oe";
              type = TT_CHAR;
              break;
          case 188: // ü
          case 156:
              word += "ue";
              type = TT_CHAR;
              break;
          case 159: // ß
              word += "ss";
              type = TT_CHAR;
              break;
        }
      }
    }
    error = MM_UTF8_WORD;
  }
  else if (charError == MM_UTF8_EOF)
  {
    error = MM_UTF8_EOF;
    type = TT_NONE;
  }
  else
  {
    // Start or continuation byte error
    error = MM_UTF8_WORD;
    type = TT_NONE;
  }

  return word;
}

/** Liest ein ganzes Wort aus der bereits geöffneten
 * UTF8 Datei.
 */
string readUtf8Word(int &error)
{
  if (file.eof())
  {
    error = MM_UTF8_EOF;
    return "";
  }

  string word;
  string utf8Token;
  int tokenType;
  int tokenError;
  while (!file.eof()) 
  {
    utf8Token = getUtf8Token(tokenType, tokenError);
    if (tokenError == MM_UTF8_WORD)
    {
      if (tokenType == TT_SPACE)
      {
        // Whitespace
        if ((utf8ParseMode == PM_WORD) ||
            (utf8ParseMode == PM_PUNCT) ||
            (utf8ParseMode == PM_PUNCT_CONT) ||
            (utf8ParseMode == PM_PUNCT_BEHIND) ||
            (utf8ParseMode == PM_PUNCT_BEHIND_CONT))
        {
          error = MM_UTF8_WORD;
          utf8ParseMode = PM_SPACE;
          if ((utf8ParseMode == PM_PUNCT_BEHIND) ||
              (utf8ParseMode == PM_WORD))
          {
            word += utf8LastToken;
            utf8LastToken = "";
          }
          return word;
        } else if ((utf8ParseMode == PM_SPACE) ||
                   (utf8ParseMode == PM_SPACE_CONT))
        {
          utf8ParseMode = PM_SPACE_CONT;
        }
        else
        {
          // PM_VOID
          utf8ParseMode = PM_SPACE;
        }
        utf8LastToken = "";
      }
      else if (tokenType == TT_CHAR)
      {
        if ((utf8ParseMode == PM_PUNCT) ||
            (utf8ParseMode == PM_PUNCT_CONT) ||
            (utf8ParseMode == PM_WORD))
        {
          // Ggf. führendes Apostroph einfügen
          word += utf8LastToken;
        }
        else if ((utf8ParseMode == PM_PUNCT_BEHIND) ||
                 (utf8ParseMode == PM_PUNCT_BEHIND_CONT))
        {
          utf8LastToken = utf8Token;
          error = MM_UTF8_WORD;
          utf8ParseMode = PM_WORD;
          return word;
        }
        utf8ParseMode = PM_WORD;
        word += utf8Token;
        utf8LastToken = "";
      }
      else if ((tokenType == TT_PUNCT) ||
            (tokenType == TT_PUNCTEXT))
      {
        if (utf8ParseMode == PM_WORD)
        {
          word += utf8Token;
          utf8ParseMode = PM_PUNCT_BEHIND;
          utf8LastToken = "";
        }
        else if (utf8ParseMode == PM_PUNCT)
        {
          utf8LastToken = "";
          std::size_t found = apostrophChars.find(utf8Token);
          if (found != string::npos)
          {
            utf8LastToken = utf8Token;
          }
          utf8ParseMode = PM_PUNCT_CONT;
        }
        else if (utf8ParseMode == PM_PUNCT_CONT)
        {
          utf8LastToken = "";
          std::size_t found = apostrophChars.find(utf8Token);
          if (found != string::npos)
          {
            utf8LastToken = utf8Token;
          }
        }
        else if (utf8ParseMode == PM_PUNCT_BEHIND)
        {
          std::size_t found = separatorChars.find(utf8Token);
          if (found != string::npos)
          {
            word += utf8Token;
            utf8ParseMode = PM_SPACE;
            return word;
          }

          utf8ParseMode = PM_PUNCT_BEHIND_CONT;
        }
        else if (utf8ParseMode == PM_PUNCT_BEHIND_CONT)
        {
          std::size_t found = separatorChars.find(utf8Token);
          if (found != string::npos)
          {
            word += utf8Token;
            utf8ParseMode = PM_SPACE;
            return word;
          }
        }
        else
        {
          utf8LastToken = "";
          std::size_t found = apostrophChars.find(utf8Token);
          if (found != string::npos)
          {
            utf8LastToken = utf8Token;
          }
          utf8ParseMode = PM_PUNCT;
        }
      }
      else
      {
        utf8LastToken = "";
        // Behandeln als Leerzeichen
        utf8ParseMode = PM_SPACE;
        if ((utf8ParseMode == PM_WORD) ||
            (utf8ParseMode == PM_PUNCT) ||
            (utf8ParseMode == PM_PUNCT_CONT) ||
            (utf8ParseMode == PM_PUNCT_BEHIND) ||
            (utf8ParseMode == PM_PUNCT_BEHIND_CONT))
        {
          // Aktuelles Wort beendet
          error = MM_UTF8_WORD;
          return word;
        }
      }
    }
  }

  if (!word.empty())
  {
    error = MM_UTF8_WORD;
  }
  else
  {
    error = MM_UTF8_EOF;
  }
  return word;
}

/** Gibt die Datei mit ihren erkannten/geparsten Worten
 * auf die Standardausgabe aus.
 */
void parseUtf8FileToStdout(const string &filePath)
{
  fileName = filePath;
  if (openUtf8File() == MM_FALSE)
    return;

  int error = 0;
  string word = readUtf8Word(error);
  if (error != MM_UTF8_WORD)
    return;
  
  cout << word;
  cout.flush();
  while (error != MM_UTF8_EOF)
  {
    word = readUtf8Word(error);
    if (error == MM_UTF8_WORD)
    {
      cout << " " << word;
      cout.flush();
    }
  }
  cout << std::endl;
}
