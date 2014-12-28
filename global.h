#ifndef GLOBAL_H
#define GLOBAL_H

/*-------------------------------------------------------- Includes */

#include "mmscreen/ScreenInfo.h"

/*--------------------------------------------------------- Defines */

/*-------------------------------------------------------- Typedefs */

#ifdef DOS
typedef char keyChar;
#else
typedef unsigned int keyChar;
#endif

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

extern ScreenInfo currentScreen;

/*--------------------------------------------------- Functions */


#endif

