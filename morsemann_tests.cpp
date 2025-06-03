#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "mmword.h"
#include "utf8file.h"

#include <sstream>

TEST_CASE("testing basic parsing of words") {
    resetUtf8Parser();
    selectedCharGroup = 1;

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
