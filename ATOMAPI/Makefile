#
# Socket Library 
#
include ../Makefile.inc

#CXXFLAGS += -DHEHE_HOHO
CXXFLAGS += -g
CXXFLAGS += -pg -fPIC
CXXFLAGS += -I/usr/include/mysql
CXXFLAGS += -L/usr/lib64/mysql
CXXFLAGS += -I$(ATOM_SRC_HOME)/AGENT/INCLUDE/QUEUE
CXXFLAGS += -L$(ATOM_SRC_HOME)/AGENT/LIB
CXXFLAGS += -I$(ATOM_SRC_HOME)/AGENT/INCLUDE
CXXFLAGS += -L$(ATOM_SRC_HOME)/AGENT/LIB
CXXFLAGS += -I../
CXXFLAGS += -L../

LIBS     += -latomlog
LIBS     += -latomdb
LIBS     += -lmysqlclient
LIBS     += -latomsocket
LIBS     += -latomtrace
LIBS     += -latomqueue
LIBS     += -latomqueue_common
LIBS     += -latomconfig
LIBS     += -latomstaapi
LIBS     += -latommodule
LIBS     += -latomutil
LIBS     += -lrt


ATOM_COMM_LIB_HOME = $(ATOM_SRC_HOME)/LIB
ATOM_AGENT_API_LIB_HOME = $(ATOM_SRC_HOME)/AGENT/LIB

INC_PATH += -I$(ATOM_SRC_HOME)/AGENT/INCLUDE/QUEUE

TARGET = libatomapi.a

OBJS =	CAtomAPI.o CAppControl.o

all :: library

library : $(OBJS)
	cp $(ATOM_COMM_LIB_HOME)/libatomlog.a ./BUILD
	ar -x ./BUILD/libatomlog.a
	cp $(ATOM_COMM_LIB_HOME)/libatomdb.a ./BUILD
	ar -x ./BUILD/libatomdb.a
	cp $(ATOM_COMM_LIB_HOME)/libatomsocket.a ./BUILD
	ar -x ./BUILD/libatomsocket.a
	cp $(ATOM_COMM_LIB_HOME)/libatomconfig.a ./BUILD
	ar -x ./BUILD/libatomconfig.a
	cp $(ATOM_COMM_LIB_HOME)/libatomutil.a ./BUILD
	ar -x ./BUILD/libatomutil.a
	cp $(ATOM_AGENT_API_LIB_HOME)/libatomstaapi.a ./BUILD
	ar -x ./BUILD/libatomstaapi.a
	cp $(ATOM_AGENT_API_LIB_HOME)/libatomtrace.a ./BUILD
	ar -x ./BUILD/libatomtrace.a
	cp $(ATOM_AGENT_API_LIB_HOME)/libatommodule.a ./BUILD
	ar -x ./BUILD/libatommodule.a
	cp $(ATOM_AGENT_API_LIB_HOME)/libatomqueue.a ./BUILD
	ar -x ./BUILD/libatomqueue.a
	cp $(ATOM_AGENT_API_LIB_HOME)/libatomqueue_common.a ./BUILD
	ar -x ./BUILD/libatomqueue_common.a
	mv *.o ./BUILD
	$(AR) rc $(TARGET) ./BUILD/*.o

install ::
	cp $(TARGET) ../LIB
	cp *.h*    $(ATOM_SRC_HOME)/INCLUDE

clean ::
	rm -f ./BUILD/*.o
