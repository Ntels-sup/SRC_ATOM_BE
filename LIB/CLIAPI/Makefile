include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomcliapi.a

OBJS	= CCliArg.o\
		  CCliArgPara.o\
		  CCliReq.o\
		  CCliRsp.o\

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
