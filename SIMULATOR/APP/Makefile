# Applicaton sample
#

include ../../Makefile.inc

CXXFLAGS += -pg -fPIC
CXXFLAGS += -I$(MYSQL_INC)
CXXFLAGS += -L$(MYSQL_LIB)
CXXFLAGS += -I$(ATOM_SRC_HOME)/AGENT/INCLUDE/QUEUE
CXXFLAGS += -L$(ATOM_SRC_HOME)/AGENT/LIB
CXXFLAGS += -I../
CXXFLAGS += -L../

LIBS     += -latomapi
LIBS     += -latomconfig
LIBS     += -latomutil
LIBS     += -latomstaapi
LIBS     += -latomstaresapi
LIBS     += -lmysqlclient
LIBS     += -latomsocket
LIBS     += -latomqueue
LIBS     += -latomqueue_common
LIBS     += -latomdb
LIBS     += -latomlog
LIBS     += -latommodule
LIBS     += -lrt


APP = APP
APP_OBJS = APPMain.o
#APP_OBJS 	= 	Jhchoi.o \
#				alarm_test.o
all :: $(APP) 

$(APP) : $(APP_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 

install ::
	echo "Nothing"
