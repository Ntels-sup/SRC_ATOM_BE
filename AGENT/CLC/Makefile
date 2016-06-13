#
## ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g
CXXFLAGS += -DNM_DEBUG

INC_PATH += -I$(MYSQL_INC)
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -lpthread
LIBS     += -latomnminter
LIBS     += -latomsocket
LIBS     += -latomcli
LIBS     += -latomcliapi
LIBS     += -latomlog
LIBS     += -lmysqlclient
LIBS     += -latomdb
LIBS     += -latomtimer
LIBS     += -latomthread
LIBS     += -latomconfig
LIBS     += -lreadline
LIBS     += -latomrabbitalloc
LIBS     += -latomcliregapi
LIBS     += -lssl

TARGET = ATOM_CLC

OBJS    =   CGlobal.o\
			CCliDoc.o\
			CCli.o\
			CUser.o\
			CConsole.o\
			CClaInterface.o\
			CConsoleSession.o\
			CConsoleThread.o\
			CMain.o\

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install::
	cp $(TARGET) $(ATOM_INSTALL_PATH)
