#
# Socket Library 
#
include ../../Makefile.inc

#CXXFLAGS += -fPIC
CFLAGS += -g

INC_PATH += -I$(MYSQL_INC)
LIB_PATH += -L$(MYSQL_LIB)

LIBS    += -lmysqlclient
LIBS    += -lpthread

TARGET = libatomdb.a

OBJS =  DBConfig.o \
		MariaDB.o \
		FetchMaria.o 

all :: library

library : $(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

install ::
	cp $(TARGET) ../
	cp *.hpp 	$(ATOM_SRC_HOME)/INCLUDE
