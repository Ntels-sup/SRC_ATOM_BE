#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC
CXXFLAGS += -DNA_DEBUG


INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I../INCLUDE/QUEUE

LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -lpthread 
LIBS     += -ldl
LIBS     += -latomlog
LIBS     += -latomsocket
LIBS     += -lmysqlclient
LIBS     += -latomutil
LIBS	 += -latomconfig
LIBS	 += -latomqueue
LIBS	 += -latomqueue_common
LIBS     += -latomdb

TARGET = ATOM_NA

OBJS =	CConfigNa.o \
		CCmdBase.o \
		CCmdRegistNode.o \
		CCmdVnfReady.o \
		CCmdVnfInstall.o \
		CCmdVnfStart.o \
		CCmdVnfStop.o \
		CCmdAppStart.o \
		CCmdNodeStatus.o \
		CCommandNa.o \
		CAddress.o \
		CMesgRoute.o \
		CModule.o \
		CModuleIPC.o \
		CModuleManager.o \
		CPkgManage.o \
		NAMain.o 

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)
	ar rc libatommodule.a CAddress.o CMesgRoute.o CModuleIPC.o

install::
	cp libatommodule.a ../LIB
	cp CAddress.hpp ../INCLUDE
	cp CModule.hpp ../INCLUDE
	cp CModuleIPC.hpp ../INCLUDE
	cp CMesgRoute.hpp ../INCLUDE
	cp $(TARGET) $(ATOM_INSTALL_PATH) 
