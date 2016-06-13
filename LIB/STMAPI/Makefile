include ../../Makefile.inc

CXXFLAGS += -g

TARGET = libatomstmapi.a

OBJS	= CCollectValue.o \
		CStmAnsApi.o \
		CStmReqApi.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
