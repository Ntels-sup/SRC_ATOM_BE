include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomregapi.a

OBJS	= CRegReqApi.o \
		CRegRespApi.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
