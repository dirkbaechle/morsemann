# Makefile for/für ``MorseMann Soundlib v1.2''
# by Dirk Bächle (dl9obn@darc.de), 27.05.2007
#
# Available targets/Verfügbare ``Targets'':
#   all, clean
#

# Your favourite C compiler/Ihr C-Kompiler
CC	= gcc
# Your favourite archiver (ar)/Ihr Archivierer (ar)
AR = ar
# Your favourite Ranlib (ranlib)/Ihr Ranlib-Programm (ranlib)
RANLIB = ranlib

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = mmsound
AROBJS = beep.o alarm.o mmsound.o
CINCLUDES = -DLINUXSIGS

all: lib$(TARGET).a
	

lib$(TARGET).a: $(AROBJS)
	rm -f lib$(TARGET).a
	$(AR) -ro lib$(TARGET).a $(AROBJS)
	$(RANLIB) lib$(TARGET).a

%.o: %.c %.h
	$(CC) $< -c -o $@ $(CINCLUDES)

clean:
	rm -f lib$(TARGET).a
	rm -f *.o

