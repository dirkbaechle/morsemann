# Makefile for/für ``Morsemann v2.0''
# by Dirk B"achle (dl9obn@darc.de), 10.08.2008
#
# Available targets/Verfügbare ``Targets'':
#   all, allusers, clean, clean-dist
#

# Your favourite C++ compiler/Ihr C++-Kompiler
CXX	= g++

# Select your curses lib
#CURSESLIB = -lcurses
CURSESLIB = -lncurses

# Uncomment next line if your curses terminal does not
# properly support colors...
# CXXFLAGS += -DNO_COLORS

SOUNDLIBS =

CXXFLAGS += -DHAVE_ALSA
SOUNDLIBS += -lasound

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = morsemann
CINCLUDES = -I./mmsound
CLIBS = $(CURSESLIB) $(SOUNDLIBS) -L./mmsound -lmmsound

export CXXFLAGS

all: $(TARGET).cpp mmsound/libmmsound.a
	$(CXX) $(TARGET).cpp -o $(TARGET) $(CXXFLAGS) $(CINCLUDES) $(CLIBS)

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

