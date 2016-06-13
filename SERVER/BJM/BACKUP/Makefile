#
#	BJM (Batch Job Manager)
#

include ../../Makefile.inc

INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I.

#LIB_PATH += -L$(LD_LIBRARY_PATH)
LIB_PATH += -L$(MYSQL_LIB)
LIBS	+= -lmysqlclient

CFLAGS	+= -Wall -g
LIBS    += -lpthread -ldl -latomlog -latomconfig -latomsocket -latomdb

TARGET		= ATOM_BJM

SRCS		= CMain.cpp \
			CConfigBJM.cpp	\
			CScheduler.cpp \
			CATMTime.cpp	\
			CATMConv.cpp	\
			CATMDate.cpp	\
			CBatchGroup.cpp	\
			CBatchJob.cpp	\
			CBatchFlow.cpp	\
			CProcess.cpp	\
			CBatchHist.cpp	\
			CNMSession.cpp
#			CBackupHist.cpp	\
			CBackupDelete.cpp
#			CBackupRestore.cpp	\

OBJS = $(SRCS:.cpp=.o)

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

clean ::
	$(RM) -f $(OBJS) $(TARGET)

install ::
	cp  $(TARGET)  $(ATOM_INSTALL_PATH)
