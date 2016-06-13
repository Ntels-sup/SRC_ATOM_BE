#
# Socket Library 
#
include ../../Makefile.inc

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I$(ATOM_SRC_HOME)/LIB/ALMAPI
INC_PATH += -I.

LIB_PATH += -L$(MYSQL_LIB)
LIBS    += -latomutil
LIBS    += -latomAlarm
LIBS 	+= -latomsocket
LIBS    += -latomdb
LIBS    += -lmysqlclient
LIBS 	+= -latomconfig
LIBS 	+= -latomlog

#LOCALFLAGS = -D_DEBUG_MACRO
LOCALFLAGS = -D_DEBUG_MACRO
LOCALFLAGS = -g

TARGET = ATOM_ALM 

OBJS =	AlmMain.o \
		AlarmManager.o \
		SequenceByNode.o \
		NodeManagerIF.o \
		EventFactory.o \
		UnknownEventHandler.o \
		TcaAlarmEventHandler.o \
		RetryEventHandler.o \
		NodeStatusEventHandler.o \
		AlarmSyncEventHandler.o \
		AlarmEventHandler.o \
		SubscribeEventHandler.o \
		UserChangeEventHandler.o \
		AlarmSTS.o \
		SnmpTrap.o \
		AlarmCFG.o 	\

all :: application 

application : $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH) 
