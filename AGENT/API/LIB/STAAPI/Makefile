include ../../../../Makefile.inc

#CXXFLAGS += 

TARGET = libatomstaapi.a

OBJS	= CStaAnsApi.o\
		CStaReqApi.o \
		CTableData.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ../../../LIB
	cp *.hpp ../../../INCLUDE
	cp *.h ../../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
