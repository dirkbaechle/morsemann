#ifndef SCREEN_H
#define SCREEN_H

/*-------------------------------------------------------- Includes */

#include "global.h"

#include <string>

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

extern int screenX;
extern int screenY;
extern int centerX;
extern int centerY;

/*--------------------------------------------------- Functions */

extern void writeString(const std::string& str);
extern void writeNumber(int number);
extern void writeChar(char b);

extern void moveWrite(int ypos, int xpos, const char *fmt, const std::string& str);
extern void gotoxy(int xpos, int ypos);
extern void clrscr();
extern int kbhit(void);
extern void textModusNormal(void);
extern void textModusSelect(void);
extern void textModusError(void);
extern void hideCursor(void);
extern void showCursor(void);
extern void writeSelection(const std::string& str, int xpos, int ypos, int stringID, int selected);
extern std::string readString(int xpos, int ypos, int max, const std::string& str);
extern int readNumber(int xpos, int ypos, int max, int number);

#endif

