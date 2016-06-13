include ../../Makefile.inc

#CXXFLAGS += -fPIC
CXXFLAGS += -g

TARGET = libatomsocket.a

OBJS	= CSocket.o \
		  CSocketServer.o \
		  CProtocol.o \
		  CMesgExchSocket.o \
		  CMesgExchSocketServer.o

all:: library
	
library: $(OBJS)
	ar rc $(TARGET) $(OBJS)

install::
	cp $(TARGET) ..
	cp *.hpp ../../INCLUDE
