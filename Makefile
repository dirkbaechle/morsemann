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

CXXFLAGS += -DHAVE_ALSA -g -O2 -Wall -Wextra
SOUNDLIBS = -lasound

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = morsemann
CINCLUDES = -I./mmsound -I./mmscreen -I./mmword -I./inih -I./inih/cpp -I.
CLIBS = -L./mmscreen -lmmscreen $(CURSESLIB) -L./mmsound -lmmsound $(SOUNDLIBS) -L./mmword -lmmword -L./inih -linih
CTESTLIBS = -L./mmword -lmmword

export CXXFLAGS

all: $(TARGET).cpp mmconfig.cpp mmsound/libmmsound.a mmscreen/libmmscreen.a mmword/libmmword.a inih/libinih.a 
	$(CXX) $(TARGET).cpp mmconfig.cpp -o $(TARGET) $(CXXFLAGS) $(CINCLUDES) $(CLIBS)

test: $(TARGET)_tests.cpp mmword/libmmword.a 
	$(CXX) $(TARGET)_tests.cpp -o $(TARGET)_tests $(CXXFLAGS) $(CINCLUDES) $(CTESTLIBS)

mmsound/libmmsound.a: mmsound/mmsound.cpp mmsound/mmsound.h \
                      mmsound/beep.cpp mmsound/beep.h \
                      mmsound/alarm.cpp mmsound/alarm.h
	make -C mmsound

mmscreen/libmmscreen.a: mmscreen/mmscreen.cpp mmscreen/mmscreen.h
	make -C mmscreen

mmword/libmmword.a: mmword/mmword.cpp mmword/mmword.h \
                    mmword/utf8file.cpp mmword/utf8file.h
	make -C mmword

inih/libinih.a:
	make -C inih

allusers:
	chown root:root ./morsemann
	chmod a+sx ./morsemann

clean:
	rm -f morsemann morsemann_tests

clean-dist:
	rm -f morsemann morsemann_tests
	make -C mmsound clean
	make -C mmscreen clean
	make -C mmword clean
	make -C inih clean
