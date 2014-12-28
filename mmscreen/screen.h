#ifndef SCREEN_H
#define SCREEN_H

/*-------------------------------------------------------- Includes */

#include "global.h"

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

/*--------------------------------------------------- Functions */

extern void writeString(char *string);
extern void writeNumber(int number);
extern void writeChar(char b):

#ifndef DOS
extern void gotoxy(int xpos, int ypos);
extern void clrscr();
extern int kbhit(void);
#endif

extern void moveWrite(int ypos, int xpos, char *fmt, char *string);
extern void textModusNormal(void);
extern void textModusSelect(void);
extern void textModusError(void);
extern void hideCursor(void);
extern void showCursor(void);
extern void writeSelection(char *string, int xpos, int ypos, int stringID, int selected);
extern void readString(int xpos, int ypos, int max, char *string);
extern void readNumber(int xpos, int ypos, int max, int *number);

#endif

