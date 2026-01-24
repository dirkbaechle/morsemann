# Makefile for/für ``MorseMann Wordlib''
#
# Available targets/Verfügbare ``Targets'':
#   all, clean
#

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = mmword
AROBJS = mmword.o utf8file.o
CINCLUDES = -DLINUX

all: lib$(TARGET).a
	

lib$(TARGET).a: $(AROBJS)
	rm -f lib$(TARGET).a
	$(AR) $(AROPTS) lib$(TARGET).a $(AROBJS)
	$(RANLIB) lib$(TARGET).a

%.o: %.cpp %.h
	$(CC) $< -c -I.. -o $@ $(CINCLUDES)

clean:
	rm -f lib$(TARGET).a
	rm -f *.o
