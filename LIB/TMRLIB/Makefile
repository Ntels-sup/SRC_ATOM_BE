include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomtimer.a

OBJS	= Timer.o\
		TimerEntry.o\
		TimerEvent.o\
		TimerNode.o

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
