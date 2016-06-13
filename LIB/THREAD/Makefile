include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomthread.a

OBJS	= CThread.o \
		CThread.o \
		CThread.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
