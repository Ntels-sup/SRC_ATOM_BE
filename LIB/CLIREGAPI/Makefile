include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomcliregapi.a

OBJS	= CCliRegReqApi.o \
		CCliRegRspApi.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
