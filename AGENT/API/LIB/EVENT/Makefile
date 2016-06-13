#
# Socket Library 
#
include ../../../../Makefile.inc

CFLAGS += -g
CFLAGS += -fPIC

INC_PATH += 
LIB_PATH += 

LIBS    += 

TARGET = libatomeventapi.a

OBJS =  EventAPI.o

all :: library

library : $(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

install ::
	cp $(TARGET) ../
	cp *.hpp 	$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
