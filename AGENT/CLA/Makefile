#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC -shared
INC_PATH += -I../INCLUDE -I$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
LIB_PATH += -L../LIB -L$(ATOM_SRC_HOME)/AGENT/API/LIB

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I../INCLUDE/QUEUE
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -latomconfig -latomdb -latomcliapi -latomsocket -latomstmapi -latomstaapi -latomlog -latommodule -latomcliregapi -latomrabbitalloc

TARGET = ATOM_NA_CLA.so

OBJS = CCliPeer.o\
		CCliRoute.o\
		CSession.o\
		CClaConfig.o\
		CGlobal.o\
		CMain.o\
		CSession.o\

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
	cp $(TARGET) $(ATOM_INSTALL_PATH)/TEST
