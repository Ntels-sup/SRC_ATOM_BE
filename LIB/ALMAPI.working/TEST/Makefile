#
# Socket Library 
#
include ../../../Makefile.inc

#INC_PATH += -I/UDBS/ATOM/SRC_ATOM_BE/LIB/COMMON
INC_PATH += -I.

LIBS    += -latomlog

LOCALFLAGS += -g 

TARGET = test-sequence-id

OBJS =	Test-SequenceId.o \
		SequenceId.o


all :: application 

application : $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) ../
