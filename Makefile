# Makefile for/für ``Morsemann v2.0''
# by Dirk B"achle (dl9obn@darc.de), 10.08.2008
#
# Available targets/Verfügbare ``Targets'':
#   all, allusers, clean, clean-dist
#

# Your favourite C++ compiler/Ihr C++-Kompiler
CC	= g++

# Select your curses lib
#CURSESLIB = -lcurses
CURSESLIB = -lncurses

# Uncomment next line if your curses terminal does not
# properly support colors...
# NOCOLORS = -DNO_COLORS

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = morsemann
CINCLUDES = -I./mmsound
CLIBS = $(CURSESLIB) -L./mmsound -lmmsound

all: $(TARGET).cpp mmsound/libmmsound.a
	$(CC) $(TARGET).cpp -o $(TARGET) $(CINCLUDES) $(CLIBS)

mmsound/libmmsound.a:
	make -C mmsound

allusers:
	chown root:root ./morsemann
	chmod a+sx ./morsemann

clean:
	rm -f morsemann

clean-dist:
	rm -f morsemann
	make -C mmsound clean

