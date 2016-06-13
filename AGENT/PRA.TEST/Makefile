#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g -fPIC -shared
INC_PATH += -I../INCLUDE -I$(ATOM_SRC_HOME)/AGENT/API/INCLUDE
LIB_PATH += -I../LIB -L$(ATOM_SRC_HOME)/AGENT/API/LIB

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I../INCLUDE/QUEUE
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -latomlog
LIBS	 += -latomsocket -latomstmapi -latomstaapi -latomcliapi -latommodule

TARGET = ATOM_NA_PRA.so

OBJS =	PRAMain.o \

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)/TEST
