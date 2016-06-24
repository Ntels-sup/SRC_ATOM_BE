#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC -shared
CXXFLAGS += -DPRA_DEBUG

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I../INCLUDE/QUEUE
INC_PATH += -I../API/INCLUDE

LIB_PATH += -L$(MYSQL_LIB)
LIB_PATH += -L../API/LIB

LIBS     += -latomlog
LIBS	 += -latomsocket
LIBS     += -latommodule
LIBS	 += -latomeventapi
LIBS     += -latomutil
LIBS	 += -latomqueue
LIBS	 += -latomqueue_common
LIBS	 += -latomconfig
LIBS	 += -latomcliapi
LIBS	 += -latomrabbitalloc
LIBS	 += -latomdb
LIBS	 += -lnuma

TARGET = ATOM_NA_PRA.so

OBJS =	CConfigPra.o \
		CCmdBase.o \
		CCmdAppInit.o \
		CCmdAppLogLevel.o \
		CCmdBatchStart.o \
		CCmdAppCtl.o \
		CCommandPra.o \
		CAppQueue.o \
		CProcStatus.o \
		CProcess.o \
		CProcessManager.o \
		PRAMain.o

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
