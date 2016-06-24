include ../../Makefile.inc

CXXFLAGS += -pg -fPIC

TARGET = libatomrabbitalloc.a

OBJS	= CRabbitAlloc.o\

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
