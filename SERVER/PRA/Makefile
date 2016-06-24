#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC -shared
CXXFLAGS += -DPRA_DEBUG

INC_PATH += -I$(MYSQL_INC)
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -latomlog
LIBS	 += -latomsocket
LIBS     += -latomutil
LIBS	 += -latomconfig
LIBS	 += -latomdb
LIBS	 += -lnuma

TARGET = ATOM_NM_PRA.so

OBJS =	CConfigPra.o \
		CCmdBase.o \
		CCmdAppInit.o \
		CCmdAppLogLevel.o \
		CCmdBatchStart.o \
		CCmdAppCtl.o \
		CCommandPra.o \
		CProcStatus.o \
		CProcess.o \
		CProcessManager.o \
		../NM/CAddress.o \
		../NM/CModuleIPC.o \
		../NM/CMesgRoute.o \
		PRAMain.o

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
