#
# ATOM License Manager
#
include ../../../Makefile.inc

CXXFLAGS += -g
#CXXFLAGS += -DLM_DEBUG

INC_PATH += -I$(MYSQL_INC)
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -lpthread 
LIBS     += -ldl
LIBS     += -latomlog
LIBS     += -latomsocket
LIBS     += -lmysqlclient
LIBS	 += -latomdb
LIBS	 += -lssl

TARGET = UT_LM_GET

OBJS =	CAESCrypt.o \
		CLicense.o \
		CLicenseMain.o

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -MD -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install::
	cp $(TARGET) $(ATOM_INSTALL_PATH) 
