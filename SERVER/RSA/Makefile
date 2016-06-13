#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC
CXXFLAGS += -I$(MYSQL_INC)
CXXFLAGS += -L$(MYSQL_LIB)
CXXFLAGS += -L$(ATOM_SRC_HOME)/AGENT/API/LIB
CXXFLAGS += -I$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
#CXXFLAGS += -I../INCLUDE
#CXXFLAGS += -L../LIB
#CXXFLAGS += -D_USE_HPACUCLI

LIBS     += -latomlog
LIBS     += -latomdb
LIBS     += -latomeventapi
LIBS     += -lmysqlclient
LIBS     += -latomsocket
LIBS     += -latomqueue
LIBS     += -latomqueue_common
LIBS     += -latomlog
LIBS     += -latomconfig
LIBS     += -latomstmresapi
LIBS     += -latommodule
LIBS     += -latomutil
LIBS     += -latomcliapi
LIBS     += -lrt


TARGET = ATOM_NM_RSA.so

OBJS =	../NM/CModule.o \
		../NM/CAddress.o \
		../NM/CModuleIPC.o \
		../NM/CMesgRoute.o \
		RSAMain.o \
		RSAMsg.o

all :: $(TARGET) 

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
