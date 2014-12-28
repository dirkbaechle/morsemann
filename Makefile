# Makefile for/f"ur ``Morsemann v1.1''
# by Dirk B"achle (dl9obn@darc.de), 07.03.2003
#
# Available targets/Verf"ugbare ``Targets'':
#   ncurses, curses, nocolor, allusers, clean
#

# Your favourite C compiler/Ihr C-Kompiler
CC	= gcc

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr "andern!!!
#

TARGET = morsemann
CLIBS = -lm -lncurses

default: ncurses
	

ncurses: morsemann.c beep.h beepLinux.c alarm.h alarm.c
	$(CC) *.c -o $(TARGET) $(CLIBS)

curses: morsemann.c beep.h beepLinux.c alarm.h alarm.c
	$(CC) *.c -o $(TARGET) -lm -lcurses

nocolor: morsemann.c beep.h beepLinux.c alarm.h alarm.c
	$(CC) *.c -DNO_COLORS -o $(TARGET) -lm -lcurses

allusers:
	chown root:root ./morsemann
	chmod a+sx ./morsemann

clean:
	rm -f morsemann

