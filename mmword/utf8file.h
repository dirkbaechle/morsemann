#ifndef UTF8FILE_H
#define UTF8FILE_H

/*-------------------------------------------------------- Includes */

#include "global.h"

#include <string>

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */


/*------------------------------------------------------- Functions */

extern void resetUtf8Parser();
extern int utf8FileContainsWords();
extern int openUtf8File();
extern void closeUtf8File();
extern std::string readUtf8Char(std::istream &stream, int &error);
extern std::string getFilteredUtf8Chars(std::string token, int type);
extern std::string getUtf8Token(std::istream &stream, int &type, int &error);
extern std::string readUtf8Word(std::istream &stream, int &error);
extern void parseUtf8FileToStream(std::istream &stream, std::ostream &out);
extern void parseUtf8FileToStdout(const std::string &filePath);
extern std::string readUtf8WordVerbatim(std::istream &stream, int &error);
extern void parseUtf8FileToStreamVerbatim(std::istream &stream, std::ostream &out);
extern std::string readUtf8WordFromOpenFile(int &error);
extern std::string readUtf8WordFromOpenFileVerbatim(int &error);

#endif
