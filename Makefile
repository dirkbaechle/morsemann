# Makefile for/für ``Morsemann''
#
# Available targets/Verfügbare ``Targets'':
#   all, allusers, clean, clean-dist
#
# Prerequisites for compiling:
#   make, ncurses-dev, libasound2-dev
#

# Your favourite C++ compiler/Ihr C++-Kompiler
CXX	= g++

# Select your curses lib
#CURSESLIB = -lcurses
CURSESLIB = -lncurses

# Uncomment next line if your curses terminal does not
# properly support colors...
# CXXFLAGS += -DNO_COLORS

CXXFLAGS += -DHAVE_ALSA
SOUNDLIBS = -lasound

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = morsemann
CINCLUDES = -I./mmsound -I./mmscreen -I.
CLIBS = -L./mmscreen -lmmscreen $(CURSESLIB) -L./mmsound -lmmsound $(SOUNDLIBS)

export CXXFLAGS

all: $(TARGET).cpp mmsound/libmmsound.a mmscreen/libmmscreen.a
	$(CXX) $(TARGET).cpp -o $(TARGET) $(CXXFLAGS) $(CINCLUDES) $(CLIBS)

mmsound/libmmsound.a:
	make -C mmsound

mmscreen/libmmscreen.a:
	make -C mmscreen

allusers:
	chown root:root ./morsemann
	chmod a+sx ./morsemann

clean:
	rm -f morsemann

clean-dist:
	rm -f morsemann
	make -C mmsound clean
	make -C mmscreen clean

