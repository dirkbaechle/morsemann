#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "mmword.h"
#include "utf8file.h"

#include <sstream>

/** Hilfsfunktion für das Formulieren der folgenden Tests. */
void parseTestWord(std::istringstream &input,
                   std::ostringstream &output)
{
  int error = 0;

  std::string word = readUtf8Word(input, error);
  output << word;
  while (1)
  {
    word = readUtf8Word(input, error);
    if (error == MM_UTF8_EOF) {
      break;
    }
    output << " " << word;
  }
}                  

/** Hilfsfunktion für das Formulieren der folgenden Tests. */
void parseTestWordVerbatim(std::istringstream &input,
                           std::ostringstream &output)
{
  int error = 0;

  std::string word = readUtf8WordVerbatim(input, error);
  output << word;
  while (1)
  {
    word = readUtf8WordVerbatim(input, error);
    if (error == MM_UTF8_EOF) {
      break;
    }
    output << " " << word;
  }
}                  

TEST_CASE("testing basic parsing of words")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("ein paar Worte");
    int error = 0;
    
    std::string word = readUtf8Word(input, error);
    CHECK(word == "ein");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8Word(input, error);
    CHECK(word == "paar");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8Word(input, error);
    CHECK(word == "worte");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8Word(input, error);
    CHECK(word == "");
    CHECK(error == MM_UTF8_EOF);
}

TEST_CASE("testing conversion of german umlauts")
{
    resetUtf8Parser();
    selectedCharGroup = CG_LETTERS_ONLY;

    std::istringstream input("Üble füße mögen Öl als Äquivalent sind bäume grün und blätter sind oft brüchig");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "ueble fuesse moegen oel als aequivalent sind baeume gruen und blaetter sind oft bruechig");
}

TEST_CASE("testing parsing of letters only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_LETTERS_ONLY;

    std::istringstream input("ein paar Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "ein paar wor te you haven t seen bef re");
}

TEST_CASE("testing parsing of digits only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_DIGITS_ONLY;

    std::istringstream input("73 88 55 3+5=8 und Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "73 88 55 3 5 8 8");
}

TEST_CASE("testing parsing of punctuation only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input(",:'/ ../() 3+5=8 und Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == ",:'/ ../() + = ' ()");
}

TEST_CASE("testing simple sentence with punctuation")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("Ein simpler Satz, mit Komma und Punkt.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "ein simpler satz, mit komma und punkt.");
}

TEST_CASE("allow trailing chars inside apostrophes")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Genug damit!' sagte er.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "'genug damit!' sagte er.");
}

TEST_CASE("allow separating chars after apostrophes")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Warum gilt nicht 'veni', 'vidi' und 'vici'?");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "warum gilt nicht 'veni', 'vidi' und 'vici'?");
}

TEST_CASE("separation of URIs into words")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("https://github.com");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "https: /github. com");
}

TEST_CASE("more complicated mix with apostrophes and punctuation")
{
    // Also tests SPC2: insert leading apostrophe
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Hold your tongue!' said the Queen. 'I won't!' said Alice.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "'hold your tongue!' said the queen. 'i won't!' said alice.");
}

TEST_CASE("simple sentence ending in punctuation char")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("What has become of you? I don't know. Have a guess!");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "what has become of you? i don't know. have a guess!");
}

TEST_CASE("SPC1: insert last punctuation before a space")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Why?', did the queen ask. \"'Unclear!'\" at best.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "'why?', did the queen ask. 'unclear!' at best.");
}

TEST_CASE("SPC2: insert leading apostrophe")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("at\"all those ''crazy '`times");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "at\" all those 'crazy `times");
}

TEST_CASE("SPC3: finish word unless I've construct encountered")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("would've been`nice at\"all");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "would've been` nice at\" all");
}

TEST_CASE("SPC4: punctuation that is not trailing but separating ends the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Sti;cks and sto:nes may break my bo,nes");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "sti; cks and sto: nes may break my bo, nes");
}

TEST_CASE("SPC5: allow single punctuation chars to come through")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Sti;cks and sto:nes ,.?/=!$()+-@");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "; : ,.?/=!$()+-@");
}

TEST_CASE("SPC6/SPC7: allow and reduce leading punctuation chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("``Why not?'' he asked, ''Unbelievable.'' Für '\"`alle.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "`why not?' he asked, 'unbelievable.' fuer `alle.");
}

TEST_CASE("reducing trailing punctuation chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("Nach allen`()+-\"' anderen.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "nach allen` anderen.");
}

TEST_CASE("SPC8/SPC9/SPC11: trailing separators and other char combinations")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why',\"he\" asked. 'Unbelievable!' but true.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "`why', \"he\" asked. 'unbelievable!' but true.");
}

TEST_CASE("SPC10: a separator char behind trailing punctuation is allowed and ends the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Unbelievable!',but true.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "'unbelievable!', but true.");
}

TEST_CASE("SPC12: punctuation char before unknown char is added to the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why', '\u2522he asked.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "` ', ' .");
}

TEST_CASE("SPC13: handling of unknown unicode chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why', s\u2522he ask\u2325ed.");
    std::ostringstream output;
    parseTestWord(input, output);
    CHECK(output.str() == "`why', s he ask ed.");
}

TEST_CASE("VERB: testing basic parsing of words")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("ein paar Worte");
    int error = 0;
    
    std::string word = readUtf8WordVerbatim(input, error);
    CHECK(word == "ein");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8WordVerbatim(input, error);
    CHECK(word == "paar");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8WordVerbatim(input, error);
    CHECK(word == "worte");
    CHECK(error == MM_UTF8_WORD);
    word = readUtf8WordVerbatim(input, error);
    CHECK(word == "");
    CHECK(error == MM_UTF8_EOF);
}

TEST_CASE("VERB: testing conversion of german umlauts")
{
    resetUtf8Parser();
    selectedCharGroup = CG_LETTERS_ONLY;

    std::istringstream input("Üble füße mögen Öl als Äquivalent sind bäume grün und blätter sind oft brüchig");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "ueble fuesse moegen oel als aequivalent sind baeume gruen und blaetter sind oft bruechig");
}

TEST_CASE("VERB: testing parsing of letters only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_LETTERS_ONLY;

    std::istringstream input("ein paar Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "ein paar wor te you haven t seen bef re");
}

TEST_CASE("VERB: testing parsing of digits only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_DIGITS_ONLY;

    std::istringstream input("73 88 55 3+5=8 und Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "73 88 55 3 5 8 8");
}

TEST_CASE("VERB: testing parsing of punctuation only")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input(",:'/ ../() 3+5=8 und Wor8te you haven't seen bef()re");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == ",:'/ ../() + = ' ()");
}

TEST_CASE("VERB: testing simple sentence with punctuation")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("Ein simpler Satz, mit Komma und Punkt.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "ein simpler satz, mit komma und punkt.");
}

TEST_CASE("VERB: allow trailing chars inside apostrophes")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Genug damit!' sagte er.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "'genug damit!' sagte er.");
}

TEST_CASE("VERB: allow separating chars after apostrophes")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Warum gilt nicht 'veni', 'vidi' und 'vici'?");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "warum gilt nicht 'veni', 'vidi' und 'vici'?");
}

TEST_CASE("VERB: separation of URIs into words")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("https://github.com");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "https://github.com");
}

TEST_CASE("VERB: more complicated mix with apostrophes and punctuation")
{
    // Also tests SPC2: insert leading apostrophe
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Hold your tongue!' said the Queen. 'I won't!' said Alice.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "'hold your tongue!' said the queen. 'i won't!' said alice.");
}

TEST_CASE("VERB: simple sentence ending in punctuation char")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("What has become of you? I don't know. Have a guess!");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "what has become of you? i don't know. have a guess!");
}

TEST_CASE("VERB: SPC1: insert last punctuation before a space")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Why?', did the queen ask. \"'Unclear!'\" at best.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "'why?', did the queen ask. \"'unclear!'\" at best.");
}

TEST_CASE("VERB: SPC2: insert leading apostrophe")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("at\"all those ''crazy '`times");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "at\"all those ''crazy '`times");
}

TEST_CASE("VERB: SPC3: finish word unless I've construct encountered")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("would've been`nice at\"all");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "would've been`nice at\"all");
}

TEST_CASE("VERB: SPC4: punctuation that is not trailing but separating ends the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Sti;cks and sto:nes may break my bo,nes");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "sti;cks and sto:nes may break my bo,nes");
}

TEST_CASE("VERB: SPC5: allow single punctuation chars to come through")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("Sti;cks and sto:nes ,.?/=!$()+-@");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "; : ,.?/=!$()+-@");
}

TEST_CASE("VERB: SPC6/SPC7: allow and reduce leading punctuation chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("``Why not?'' he asked, ''Unbelievable.'' Für '\"`alle.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "``why not?'' he asked, ''unbelievable.'' fuer '\"`alle.");
}

TEST_CASE("VERB: reducing trailing punctuation chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;

    std::istringstream input("Nach allen`()+-\"' anderen.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "nach allen`()+-\"' anderen.");
}

TEST_CASE("VERB: SPC8/SPC9/SPC11: trailing separators and other char combinations")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why',\"he\" asked. 'Unbelievable!' but true.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "`why',\"he\" asked. 'unbelievable!' but true.");
}

TEST_CASE("VERB: SPC10: a separator char behind trailing punctuation is allowed and ends the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("'Unbelievable!',but true.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "'unbelievable!',but true.");
}

TEST_CASE("VERB: SPC12: punctuation char before unknown char is added to the word")
{
    resetUtf8Parser();
    selectedCharGroup = CG_PUNCT_ONLY;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why', '\u2522he asked.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "` ', ' .");
}

TEST_CASE("VERB: SPC13: handling of unknown unicode chars")
{
    resetUtf8Parser();
    selectedCharGroup = CG_ALL_CHARS;
    fileWordsExtendedCharset = MM_TRUE;

    std::istringstream input("`Why', s\u2522he ask\u2325ed.");
    std::ostringstream output;
    parseTestWordVerbatim(input, output);
    CHECK(output.str() == "`why', s he ask ed.");
}
