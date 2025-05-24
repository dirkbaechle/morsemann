
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
#define PM_PUNCT 3

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

// Stream auf die aktuelle UTF8 Datei
std::ifstream file;
// Der aktuelle Parsing-Modus innerhalb der Datei
int utf8ParseMode = PM_VOID;
// Das letzte gelesene UTF8 Zeichen.
unsigned char utf8LastChar = 0;

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

    if (file.eof()) break;

    int charLen = utf8CharLength(firstByte);
    if (charLen == -1) {
      error = MM_UTF8_INVALID_STARTBYTE;
      return "";
    }

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
  string utf8Char;
  int charError;
  while (!file.eof()) {
    utf8Char = readUtf8Char(charError);
    if (charError == MM_UTF8_CHAR)
    {
      if (utf8Char.size() == 1)
      {
        // classify character
        unsigned char c = utf8Char[0];

        if (isspace(c))
        {
          // Whitespace
          if (utf8ParseMode == PM_WORD)
          {
            error = MM_UTF8_WORD;
            return word;
          }
          utf8ParseMode = PM_SPACE;
        }
        else if (isalnum(c))
        {
          // Buchstabe/Zahl
          if ((c >= 65) && (c <= 90))
          {
            // Groß- -> Kleinbuchstabe
            c += 32;
          }

          if (utf8ParseMode == PM_PUNCT)
          {
            word += utf8LastChar;
          }
          utf8ParseMode = PM_WORD;
          word += c;
        }
        else
        {
          // Anderes Zeichen

        }
        utf8LastChar = c;
      }
      else
      {
        // treat as punctuation char
        if (utf8ParseMode == PM_WORD)
        {
          utf8ParseMode = PM_PUNCT;
        }
        utf8ParseMode = PM_SPACE;
      }
    }
  }

  if (word.size() > 0)
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
