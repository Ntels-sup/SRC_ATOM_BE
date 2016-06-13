include ../../Makefile.inc

TARGET = libatomstmresapi.a

OBJS	= CResTableData.o \
		CResGroup.o \
		CStmResReqApi.o \
		CStmResRspApi.o \

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ../
	cp *.hpp ../../INCLUDE

clean::
	rm -f *.o
	rm -f $(TARGET)
