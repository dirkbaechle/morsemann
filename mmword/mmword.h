#ifndef MMWORD_H
#define MMWORD_H

/*-------------------------------------------------------- Includes */

#include "global.h"

#include <string>

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

extern int selectedCharGroup;
extern int variableWords;
extern int fixedWordLength;
extern int confirmWords;
extern std::string charSet;
extern int charSetLength;
extern std::string groupString[8];

extern int wordMode;
extern std::string fileName;
extern int fileWordsRandom;
extern unsigned long int filePosition;
extern int fileWordsExtendedCharset;

/*--------------------------------------------------- Functions */

extern int mmRandom(int maxNumber);
extern char mapToChar(int letterID);
extern char charSetRandom(void);
extern char signRandom(void);
extern void mmwlSetCountErrorsPerWord(int countWords);
extern int mmwlGetCountErrorsPerWord();
extern int compareStrings(const std::string &userWord, const std::string &lastWord);
extern std::string getNextWord(int &error);
extern void prepareWordFile();
extern void releaseWordFile();

#endif
