#ifndef GLOBAL_H
#define GLOBAL_H

/*-------------------------------------------------------- Includes */

/*--------------------------------------------------------- Defines */

#define ENTER_CHAR 13
#define KEY_ESCAPE 27
#define KEY_REPEAT_MORSE 35

#define MM_TRUE 1
#define MM_FALSE 0

#define MM_ACCEPT 0
#define MM_ESCAPE 1
#define MM_REPEAT 2
#define MM_CONTINUE 3

#define MM_WM_RANDOM 0
#define MM_WM_FILE 1
#define MM_WM_PARIS 2

#define MM_UTF8_CHAR 0
#define MM_UTF8_INVALID_STARTBYTE 1
#define MM_UTF8_INVALID_CONTBYTE 2
#define MM_UTF8_EOF 3
#define MM_UTF8_WORD 4

#define CG_NONE 0
#define CG_ALL_CHARS 1
#define CG_LETTERS_ONLY 2
#define CG_DIGITS_ONLY 3
#define CG_PUNCT_ONLY 4
#define CG_LETTERS_AND_DIGITS 5
#define CG_LETTERS_AND_PUNCT 6
#define CG_DIGITS_AND_PUNCT 7
#define CG_ENTERED_CHAR_SET 8

#define MM_CONFIG_FOLDER "/.config/"
#define MM_CONFIG_FILE "mmconfig.ini"
#define MM_RESOURCES_FOLDER "/.config/morsemann/"

/*-------------------------------------------------------- Typedefs */

typedef unsigned int keyChar;

/*---------------------------------------------------- Const values */

/*------------------------------------------------ Global variables */

/*------------------------------------------------------- Functions */

#endif

