#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -g
CXXFLAGS += -DNM_DEBUG

#INC_PATH += -I/usr/include/mysql
#LIB_PATH += -L/usr/lib64/mysql

INC_PATH += -I$(MYSQL_INC)
LIB_PATH += -L$(MYSQL_LIB)

LIBS     += -lpthread 
LIBS     += -ldl
LIBS	 += -latomnminter
LIBS     += -latomlog
LIBS     += -latomsocket
LIBS     += -lmysqlclient
LIBS	 += -latomdb
LIBS	 += -latomstmapi
LIBS	 += -latomtimer
LIBS	 += -latomregapi
LIBS	 += -latomstmresapi
LIBS	 += -latomrabbitalloc
LIBS	 += -latomthread
LIBS	 += -latomconfig

TARGET = ATOM_STM

OBJS 	=	CMergeSession.o\
			CMergeTableSession.o\
			CLowerInterface.o\
			CMerge.o\
			CMergeThread.o\
			CGlobal.o\
			CColumnInfo.o\
			CTableInfo.o\
			CSessionInfo.o\
			CNodeInfo.o\
			CAlarmInfo.o\
			CPackageInfo.o\
			CResourceInfo.o\
			CStmConfig.o\
			CTCAInfo.o\
			CTCARule.o\
			CMain.o\

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install::
	cp $(TARGET) $(ATOM_INSTALL_PATH) 
