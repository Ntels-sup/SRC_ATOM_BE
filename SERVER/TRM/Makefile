#
#	TRM (Batch Job Manager)
#

include ../../Makefile.inc


INC_PATH += -I$(MYSQL_INC)
INC_PATH += -I.

LIB_PATH += -L$(MYSQL_LIB)
LIBS	+= -lmysqlclient

CFLAGS	+= -Wall -g
CFLAGS  += -DTRM_DEBUG
LIBS    += -lpthread -ldl -latomlog -latomconfig -latomsocket -latomdb

TARGET		= ATOM_TRM

SRCS		= CMain.cpp \
			CConfigTRM.cpp	\
			CScheduler.cpp \
			CATMTime.cpp	\
			CATMConv.cpp	\
			CATMDate.cpp	\
			CTraceRoute.cpp	\
			CProcess.cpp	\
			CTraceHist.cpp	\
			CNMSession.cpp

OBJS = $(SRCS:.cpp=.o)

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

clean ::
	$(RM) -f $(OBJS) $(TARGET)

install ::
	cp  $(TARGET)  $(ATOM_INSTALL_PATH)
