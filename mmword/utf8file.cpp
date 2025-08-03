
/*-------------------------------------------------------- Includes */

#include "utf8file.h"
#include "mmword.h"

#include <iostream>
#include <fstream>

using std::string;
using std::cout;
using std::endl;

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

// Für das Debugging der Haupt-Parsingroutine
//#define DEBUG_PARSER 1

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
// Liste der führenden Zeichen (Apostrophe und Sonderzeichen)
string leadingChars = "\"'`/(+-@";
// Liste der abschließenden Zeichen (vor einem End-Apostroph)
string trailingChars = ".!?";
// Liste der Apostroph Zeichen
string apostropheChars = "\"'`";
// Ziellänge für eine Zeile in der Standardausgabe einer Datei
int stdoutLineLength = 80;

/*--------------------------------------------------- Functions */

#ifdef DEBUG_PARSER

/** Gibt den Namen des TokenTyps als String zurück. */
string tokenTypeString(int tokenType)
{
  string s = "";
  switch (tokenType)
  {
    case 0: s = "NONE";
      break;
    case 1: s = "CHAR";
      break;
    case 2: s = "SPACE";
      break;
    case 3: s = "PUNCT";
      break;
    case 4: s = "PUNCTEXT";
      break;
    default: s = "UNKNOWN";
      break;
  }

  return s;
}

/** Gibt den Namen des TokenErrors als String zurück. */
string tokenErrorString(int tokenError)
{
  string s = "";
  switch (tokenError)
  {
    case 0: s = "CHAR";
      break;
    case 1: s = "INVALID_STARTBYTE";
      break;
    case 2: s = "INVALID_CONTBYTE";
      break;
    case 3: s = "EOF";
      break;
    case 4: s = "WORD";
      break;
    default: s = "UNKNOWN";
      break;
  }

  return s;
}

/** Gibt den Namen des ParsingModes als String zurück. */
string parseModeString(int parseMode)
{
  string s = "";
  switch (parseMode)
  {
    case 0: s = "VOID";
      break;
    case 1: s = "WORD";
      break;
    case 2: s = "SPACE";
      break;
    case 3: s = "SPACE_CONT";
      break;
    case 4: s = "PUNCT";
      break;
    case 5: s = "PUNCT_CONT";
      break;
    case 6: s = "PUNCT_BEHIND";
      break;
    case 7: s = "PUNCT_BEHIND_CONT";
      break;
    default: s = "UNKNOWN";
      break;
  }

  return s;
}

#endif // of: #ifdef DEBUG_PARSER

/** Setzt alle Variablen zurück, die während des Parsens
 * von Utf8-Dateien oder -Streams benutzt werden.
 */
void resetUtf8Parser()
{
  utf8ParseMode = PM_VOID;
  utf8LastToken = "";
}

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

/** Liest ein einzelnes UTF8 Zeichen aus dem bereits
 * geöffneten Stream (Datei).
 */
string readUtf8Char(std::istream &stream,
                    int &error)
{
  while (!stream.eof()) {
    unsigned char firstByte;
    stream.read(reinterpret_cast<char*>(&firstByte), 1);

    int charLen = utf8CharLength(firstByte);
    if (charLen == -1) {
      error = MM_UTF8_INVALID_STARTBYTE;
      return "";
    }

    if ((charLen > 1) && (stream.eof()))
      break;

    std::string utf8Char(1, firstByte);
    for (int i = 1; i < charLen; ++i) {
      unsigned char nextByte;
      stream.read(reinterpret_cast<char*>(&nextByte), 1);
      if (stream.eof() || (nextByte & 0xC0) != 0x80) {
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

/** Filtert entsprechend der aktuell gewählten Zeichenmenge
 * die ungewünschten Zeichen heraus und liefert das neue
 * Wort/Token zurück.
 */
string getFilteredUtf8Chars(string token, int type)
{
  string filteredWord;
  string::const_iterator t_it = token.cbegin();
  for (; t_it != token.cend(); ++t_it)
  {
    if (type == TT_CHAR)
    {
      if (selectedCharGroup == CG_ENTERED_CHAR_SET)
      {
        // Nur eingegebene Zeichen akzeptieren
        std::size_t cpos = charSet.find(*t_it);
        if (cpos != string::npos)
        {
          filteredWord += *t_it;
        }
      }
      else
      {
        // Buchstabe oder Zahl?
        if (isdigit(*t_it))
        {
          // Zahl
          if ((selectedCharGroup == CG_ALL_CHARS) ||
              (selectedCharGroup == CG_DIGITS_ONLY) ||
              (selectedCharGroup == CG_LETTERS_AND_DIGITS) ||
              (selectedCharGroup == CG_DIGITS_AND_PUNCT))
          {
            filteredWord += *t_it;
          }
        }
        else
        {
          // Buchstabe
          if ((selectedCharGroup == CG_ALL_CHARS) || 
              (selectedCharGroup == CG_LETTERS_ONLY) ||
              (selectedCharGroup == CG_LETTERS_AND_DIGITS) ||
              (selectedCharGroup == CG_LETTERS_AND_PUNCT))
          {
            filteredWord += *t_it;
          }
        }
      }
    }
    else if ((type == TT_PUNCT) ||
             (type == TT_PUNCTEXT))
    {
      if (selectedCharGroup == CG_ENTERED_CHAR_SET)
      {
        // Nur eingegebene Zeichen akzeptieren
        std::size_t cpos = charSet.find(*t_it);
        if (cpos != string::npos)
        {
          filteredWord += *t_it;
        }
      }
      else
      {
        if ((selectedCharGroup == CG_ALL_CHARS) || 
            (selectedCharGroup == CG_PUNCT_ONLY) ||
            (selectedCharGroup == CG_LETTERS_AND_PUNCT) ||
            (selectedCharGroup == CG_DIGITS_AND_PUNCT))
        {
          filteredWord += *t_it;
        }
      }
    }
    else
    {
      filteredWord += *t_it;
    }
  }

  return filteredWord;
}

/** Liest ein UTF8 Byte aus der bereits geöffneten
 * UTF8 Datei, konvertiert Umlaute und klassifiziert
 * den Token-Typ.
 */
string getUtf8Token(std::istream &stream,
                    int &type,
                    int &error)
{
  if (stream.eof())
  {
    type = TT_NONE;
    error = MM_UTF8_EOF;
    return "";
  }

  string word;
  int charError;
  string utf8Char = readUtf8Char(stream, charError);
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
          // Wandle Groß- -> Kleinbuchstabe
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

    if ((word.size() > 0) &&
        (type != TT_SPACE))
    {
      word = getFilteredUtf8Chars(word, type);
      // TODO check if we want to have this
      if (word.size() == 0)
      {
        type = TT_NONE;
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

/** Liest ein ganzes Wort aus dem bereits geöffneten
 * Stream (UTF8 Datei).
 */
string readUtf8Word(std::istream &stream,
                    int &error)
{
  if (stream.eof())
  {
    error = MM_UTF8_EOF;
    return "";
  }

  string word;
  string utf8Token;
  int tokenType;
  int tokenError;
  while (!stream.eof()) 
  {
    utf8Token = getUtf8Token(stream, tokenType, tokenError);
#ifdef DEBUG_PARSER    
    std::cerr << "T <" << utf8Token << ">  type " << tokenTypeString(tokenType);
    std::cerr << " error " << tokenErrorString(tokenError);
    std::cerr << " mode " << parseModeString(utf8ParseMode);
    std::cerr << " <" << utf8LastToken << ">\n";
    std::cerr.flush();
#endif
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
          // SPC1: insert last punctuation before a space
          if (utf8ParseMode != PM_PUNCT_BEHIND_CONT)
          {
            // Support for SPC3: don't add single apostrophe again
            if (utf8LastToken != "'")
            {
              word += utf8LastToken;
            }
            utf8LastToken = "";
          }
          utf8ParseMode = PM_SPACE;
          error = MM_UTF8_WORD;
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
          // SPC2: insert leading apostrophe
          word += utf8LastToken;
        }
        else if ((utf8ParseMode == PM_PUNCT_BEHIND) ||
                 (utf8ParseMode == PM_PUNCT_BEHIND_CONT))
        {
          // SPC3: finish word unless I've construct encountered
          if ((utf8ParseMode != PM_PUNCT_BEHIND) ||
              (utf8LastToken != "'"))
          {
            utf8LastToken = utf8Token;
            error = MM_UTF8_WORD;

            utf8ParseMode = PM_WORD;
            return word;
          }
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
          std::size_t found = trailingChars.find(utf8Token);
          if (found == string::npos)
          {
            found = separatorChars.find(utf8Token);
            if (found != string::npos)
            {
              // SPC4: punctation that is not trailing but separating ends the word
              error = MM_UTF8_WORD;
              utf8ParseMode = PM_SPACE;
              return word;
            }
          }

          utf8ParseMode = PM_PUNCT_BEHIND;
          if (utf8Token == "'")
          {
            // Support for SPC3: would've
            utf8LastToken = utf8Token;
          }
          else
          {
            utf8LastToken = "";
          }
        }
        else if (utf8ParseMode == PM_PUNCT)
        {
          utf8LastToken = "";
          if (selectedCharGroup == CG_PUNCT_ONLY)
          {
            // SPC5: allow single punctuation chars to come through
            word += utf8Token;
          }
          else
          {
            std::size_t found = leadingChars.find(utf8Token);
            if (found != string::npos)
            {
              // SPC6: allow and reduce leading punctuation chars
              utf8LastToken = utf8Token;
            }
          }
          utf8ParseMode = PM_PUNCT_CONT;
        }
        else if (utf8ParseMode == PM_PUNCT_CONT)
        {
          utf8LastToken = "";
          if (selectedCharGroup == CG_PUNCT_ONLY)
          {
            // SPC5: allow single punctuation chars to come through
            word += utf8Token;
          }
          else
          {
            std::size_t found = leadingChars.find(utf8Token);
            if (found != string::npos)
            {
              // SPC7: reduce leading punctuation chars
              utf8LastToken = utf8Token;
            }
          }
        }
        else if (utf8ParseMode == PM_PUNCT_BEHIND)
        {
          std::size_t found = separatorChars.find(utf8Token);
          if (found != string::npos)
          {
            // SPC8: trailing separator chars end the word
            word += utf8Token;
            utf8ParseMode = PM_SPACE;
            return word;
          }

          found = apostropheChars.find(utf8Token);
          if (found != string::npos)
          {
            found = trailingChars.find(utf8LastToken);
            if (found != string::npos)
            {
              // SPC9: insert first apostrophe char that is trailing
              word += utf8Token;
            }
          }
          utf8LastToken = "";
          utf8ParseMode = PM_PUNCT_BEHIND_CONT;
        }
        else if (utf8ParseMode == PM_PUNCT_BEHIND_CONT)
        {
          std::size_t found = separatorChars.find(utf8Token);
          if (found != string::npos)
          {
            // SPC10: a separator char behind trailing punctuation is allowed and ends the word
            word += utf8Token;
            utf8ParseMode = PM_SPACE;
            return word;
          }
        }
        else
        {
          // PM_VOID, PM_SPACE, PM_SPACE_CONT
          utf8LastToken = "";
          if (selectedCharGroup == CG_PUNCT_ONLY)
          {
            // SPC12: punctuation char before unknown char is added to the word
            word += utf8Token;
          }
          else
          {
            std::size_t found = leadingChars.find(utf8Token);
            if (found != string::npos)
            {
              // SPC11: leading punctuation char can start a new word
              utf8LastToken = utf8Token;
            }
          }
          utf8ParseMode = PM_PUNCT;
        }
      }
      else
      {
        if (selectedCharGroup == CG_PUNCT_ONLY)
        {
          // SPC12: punctuation char before unknown char is added to the word
          word += utf8LastToken;
        }

        utf8LastToken = "";
        if ((utf8ParseMode == PM_WORD) ||
            (utf8ParseMode == PM_PUNCT) ||
            (utf8ParseMode == PM_PUNCT_CONT) ||
            (utf8ParseMode == PM_PUNCT_BEHIND) ||
            (utf8ParseMode == PM_PUNCT_BEHIND_CONT))
        {
          // SPC13: unknown char ends the word
          utf8ParseMode = PM_SPACE;
          error = MM_UTF8_WORD;
          return word;
        }
        // Behandeln als Leerzeichen
        utf8ParseMode = PM_SPACE;
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

/** Liest ein ganzes Wort aus dem bereits geöffneten
 * Stream (UTF8 Datei) im Verbatim-Mode, d.h. die
 * einzelnen Worte werden nicht als Freitext behandelt
 * und nur an den Leerzeichen/Whitespaces getrennt.
 */
string readUtf8WordVerbatim(std::istream &stream,
                            int &error)
{
  if (stream.eof())
  {
    error = MM_UTF8_EOF;
    return "";
  }

  string word;
  string utf8Token;
  int tokenType;
  int tokenError;
  utf8LastToken = "";
  while (!stream.eof()) 
  {
    utf8Token = getUtf8Token(stream, tokenType, tokenError);
    if (tokenError == MM_UTF8_WORD)
    {
      if (tokenType == TT_SPACE)
      {
        // Whitespace
        if (utf8ParseMode == PM_WORD)
        {
          utf8ParseMode = PM_SPACE;
          error = MM_UTF8_WORD;
          return word;
        }
        else
        {
          // PM_VOID/PM_SPACE
          utf8ParseMode = PM_SPACE;
        }
      }
      else if ((tokenType == TT_CHAR) ||
               (tokenType == TT_PUNCT) ||
               (tokenType == TT_PUNCTEXT))
      {
        utf8ParseMode = PM_WORD;
        word += utf8Token;
      }
      else
      {
        if (utf8ParseMode == PM_WORD)
        {
          // Aktuelles Wort beendet
          utf8ParseMode = PM_SPACE;
          error = MM_UTF8_WORD;
          return word;
        }
        // Behandeln als Leerzeichen
        utf8ParseMode = PM_SPACE;
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

/** Gibt den Input-Stream (Utf8-Datei) mit seinen erkannten/geparsten Worten
 * auf den Output-Stream aus.
 */
void parseUtf8FileToStream(std::istream &stream,
                           std::ostream &out)
{
  int error = 0;
  int lineLength = 0;
  string word;
  // Erstes Wort lesen
  while (1)
  {
    word = readUtf8Word(stream, error);
    if (!word.empty())
      break;
    // Datei am Ende?
    if (error != MM_UTF8_WORD)
      return;
  }
  out << word;
  out.flush();
  lineLength += word.size();
  while (error != MM_UTF8_EOF)
  {
    word = readUtf8Word(stream, error);
    if ((error == MM_UTF8_WORD) &&
        (!word.empty()))
    {
      if (lineLength < stdoutLineLength)
      {
        out << " " << word;
        lineLength += 1 + word.size();
      }
      else
      {
        out << "\n" << word;
        lineLength = word.size();
      }
      out.flush();
    }
  }
  out << endl;
}

/** Gibt die Datei mit ihren erkannten/geparsten Worten
 * auf die Standardausgabe aus.
 */
void parseUtf8FileToStdout(const string &filePath)
{
  fileName = filePath;
  if (openUtf8File() == MM_FALSE)
    return;

  parseUtf8FileToStream(file, cout);
}

/** Gibt den Input-Stream (Utf8-Datei) mit seinen erkannten/geparsten Worten
 * auf den Output-Stream aus. Hierbei wird der Input nicht als Freitext
 * behandelt sonder nur an den Whitespaces (Leerzeichen usw.) getrennt
 * und ausgegeben
 */
void parseUtf8FileToStreamVerbatim(std::istream &stream,
                                   std::ostream &out)
{
  int error = 0;
  int lineLength = 0;
  string word;
  // Erstes Wort lesen
  while (1)
  {
    word = readUtf8WordVerbatim(stream, error);
    if (!word.empty())
      break;
    // Datei am Ende?
    if (error != MM_UTF8_WORD)
      return;
  }
  out << word;
  out.flush();
  lineLength += word.size();
  while (error != MM_UTF8_EOF)
  {
    word = readUtf8WordVerbatim(stream, error);
    if ((error == MM_UTF8_WORD) &&
        (!word.empty()))
    {
      if (lineLength < stdoutLineLength)
      {
        out << " " << word;
        lineLength += 1 + word.size();
      }
      else
      {
        out << "\n" << word;
        lineLength = word.size();
      }
      out.flush();
    }
  }
  out << endl;
}
