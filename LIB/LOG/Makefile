#
# Socket Library 
#
include ../../Makefile.inc

#CXXFLAGS += -DHEHE_HOHO
CXXFLAGS += -g

#INC_PATH += -I$(ATOM_SRC_HOME)/INCLUDE

#LIB_PATH += $(ATOM_SRC_HOME)/LIB

#LIBS     += -lpthread

TARGET = libatomlog.a

OBJS =	CFileLog.o 

all :: library

library : $(OBJS)
	$(AR) rc $(TARGET) $(OBJS)

install ::
	cp $(TARGET) ../
	cp *.h*    $(ATOM_SRC_HOME)/INCLUDE
