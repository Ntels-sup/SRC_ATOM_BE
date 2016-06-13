include ../../../../Makefile.inc

#CXXFLAGS += 

TARGET = libatomstaresapi.a

OBJS	= CResTableData.o \
		CStaResReqApi.o \
		CStaResRspApi.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ../../../LIB
	cp *.hpp ../../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
