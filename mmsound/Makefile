# Makefile for/für ``MorseMann Soundlib''
#
# Available targets/Verfügbare ``Targets'':
#   all, clean
#

#
# You shouldn't have to edit something below here!!!
# Ab dieser Zeile bitte nichts mehr ändern!!!
#

TARGET = mmsound
AROBJS = mmsound.o
CINCLUDES += -I..
CXXFLAGS += -DLINUXSIGS

all: lib$(TARGET).a
	

lib$(TARGET).a: $(AROBJS)
	rm -f lib$(TARGET).a
	$(AR) $(AROPTS) lib$(TARGET).a $(AROBJS)
	$(RANLIB) lib$(TARGET).a

%.o: %.cpp %.h
	$(CXX) $< -c -o $@ $(CXXFLAGS) $(CINCLUDES)

clean:
	rm -f lib$(TARGET).a
	rm -f *.o
