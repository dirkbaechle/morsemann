# Makefile for/für ``MorseMann Inihlib v1.2''
# by Dirk Bächle (dl9obn@darc.de), 18.05.2025
#
# Available targets/Verfügbare ``Targets'':
#   all, clean
#

# Your favourite C compiler/Ihr C-Kompiler
CXX	= g++
# Your favourite archiver (ar)/Ihr Archivierer (ar)
AR = ar
# Your favourite Ranlib (ranlib)/Ihr Ranlib-Programm (ranlib)
RANLIB = ranlib

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = inih
AROBJS = ini.o INIReader.o
CXXFLAGS += -DINI_MAX_LINE=1000

all: lib$(TARGET).a
	

lib$(TARGET).a: $(AROBJS)
	rm -f lib$(TARGET).a
	$(AR) -ro lib$(TARGET).a $(AROBJS)
	$(RANLIB) lib$(TARGET).a

INIReader.o: cpp/INIReader.cpp cpp/INIReader.h
	$(CXX) $< -c -o $@ $(CXXFLAGS) $(CINCLUDES)

ini.o: ini.c ini.h
	$(CXX) $< -c -o $@ $(CXXFLAGS) $(CINCLUDES)

clean:
	rm -f lib$(TARGET).a
	rm -f *.o

