#
# Socket Library 
#
include ../../../../Makefile.inc

CFLAGS += -g
CFLAGS += -fPIC

INC_PATH += 
LIB_PATH += 

LIBS    += 

TARGET = libatomtrace.a

OBJS =  CTraceAPI.o

all :: library

library : $(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

install ::
	cp $(TARGET) $(ATOM_SRC_HOME)/AGENT/LIB
	cp *.hpp 	$(ATOM_SRC_HOME)/AGENT/INCLUDE
