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
extern std::string readUtf8Char(std::istream &stream, int &error);
extern std::string getFilteredUtf8Chars(std::string token, int type);
extern std::string getUtf8Token(std::istream &stream, int &type, int &error);
extern std::string readUtf8Word(std::istream &stream, int &error);
extern void parseUtf8FileToStdout(const std::string &filePath);

#endif
