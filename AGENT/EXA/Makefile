#
# Socket Library 
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC -shared

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
INC_PATH += -I../INCLUDE
INC_PATH += -I../INCLUDE/QUEUE
INC_PATH += -I.

LIB_PATH += -L$(MYSQL_LIB)
LIB_PATH += -L../LIB
LIB_PATH += -L$(ATOM_SRC_HOME)/AGENT/API/LIB

LIBS    += -latomeventapi
LIBS    += -latomutil
LIBS    += -latomdb
LIBS    += -lmysqlclient
LIBS    += -latomsocket
LIBS    += -latomlog
LIBS    += -latomconfig
LIBS    += -latommodule

LOCALFLAGS = -g 

TARGET = ATOM_NA_EXA.so 

OBJS =	VnaMain.o \
        VnfmIF.o \
		VnfmToNodeEvent.o \
		NodeToVnfmEvent.o \
		NodeInterface.o \
		HttpServer.o \
		HttpClient.o \
 		ContentsProvider.o \
		DashBoard.o \
		VnaCFG.o \
        NodeData.o \
        HttpData.o 


all :: $(TARGET) 

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
