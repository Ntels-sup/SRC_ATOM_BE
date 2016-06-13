#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC
INC_PATH += -I../INCLUDE -I$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
LIB_PATH += -L../LIB -L$(ATOM_SRC_HOME)/AGENT/API/LIB
INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I../INCLUDE/QUEUE
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -latomconfig -latomlog -latomsocket -latomrabbitalloc -latomstmapi -latomstaapi -latomlog -latommodule -latomdb -lmysqlclient

TARGET = ATOM_NA_STA.so

OBJS = CGlobal.o \
		CAppInfo.o\
		CEmsInfo.o\
		CMain.o\
		CSessionInfo.o\
		CStsValue.o\
		CStsValueList.o\
		CTableInfo.o \
		CTableList.o \

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/libexec
