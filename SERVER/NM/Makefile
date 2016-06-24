#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g
#CXXFLAGS += -DNM_DEBUG

INC_PATH += -I$(MYSQL_INC) 
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -lpthread 
LIBS     += -ldl
LIBS     += -latomlog
LIBS     += -latomsocket
LIBS     += -lmysqlclient
LIBS	 += -latomdb
LIBS	 += -latomconfig
LIBS	 += -latomutil

TARGET = ATOM_NM

OBJS =	CConfigNm.o \
		CModule.o \
		CModuleManager.o \
		CModuleIPC.o \
		CAddress.o \
		CNodeProcTB.o \
		CDBInOut.o \
		CSocketMultiIO.o \
		CMesgRoute.o \
		ThreadRouting.o \
		CCommandNm.o \
		CCmdBase.o \
		CCmdRegist.o \
		CCmdRegistNode.o \
		CCmdNodeStatus.o \
		CCmdNodeList.o \
		CCmdStatusNodeEvent.o \
		NMMain.o

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install::
	cp $(TARGET) $(ATOM_INSTALL_PATH) 
