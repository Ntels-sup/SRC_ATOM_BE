include ../../Makefile.inc

#CXXFLAGS += -fPIC
CXXFLAGS += -g

TARGET = libatomutil.a

OBJS	= StringSplit.o \
          CTimeUtil.o \
		  Directory.o

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS) 

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE
