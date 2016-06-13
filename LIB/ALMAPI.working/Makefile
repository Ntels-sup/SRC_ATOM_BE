#
# Socket Library 
#
include ../../Makefile.inc

CFLAGS += -g
CFLAGS += -fPIC

INC_PATH += -I$(MYSQL_INC)

LIB_PATH +=  

LIBS    += 

#LOCALFLAGS = -D_DEBUG_MACRO
LOCALFLAGS = -D_DEBUG_MACRO

TARGET = libatomAlarm.a

OBJS =	MessageId.o \
		AlarmDuplication.o \
		AlarmDecision.o \
		AlarmLevelConf.o \
		AlarmCodeConf.o \
		AlarmDataDef.o 

all :: library

library : $(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

install ::
	cp $(TARGET) ../
	cp *.hpp 	$(ATOM_SRC_HOME)/INCLUDE
