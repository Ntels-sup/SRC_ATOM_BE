#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -pg -fPIC
CXXFLAGS += -I/usr/include/mysql
CXXFLAGS += -L/usr/lib64/mysql
CXXFLAGS += -I$(ATOM_SRC_HOME)/AGENT/INCLUDE/QUEUE
CXXFLAGS += -L$(ATOM_SRC_HOME)/AGENT/LIB
CXXFLAGS += -I../
CXXFLAGS += -L../

LIBS     += -latomapi
LIBS     += -latomutil
LIBS     += -latomstaapi
LIBS     += -latomtrace
LIBS     += -latomstaresapi
LIBS     += -lmysqlclient
LIBS     += -latomsocket
LIBS     += -latomqueue
LIBS     += -latomqueue_common
LIBS     += -latomdb
LIBS     += -latomlog
LIBS     += -latomconfig
LIBS     += -latommodule
LIBS     += -lrt


READ_PROC = test_recv
READ_OBJS =	test_recv.o
SEND_PROC = test_send
SEND_OBJS =	test_send.o

all :: $(READ_PROC) $(SEND_PROC)

$(READ_PROC) : $(READ_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 

$(SEND_PROC) : $(SEND_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 

install ::
	echo "Nothing"
