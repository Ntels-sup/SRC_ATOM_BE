#
# Socket Library 
#
include ../../../Makefile.inc

CFLAGS += -g

TARGET = test-json 

OBJS =  test3.o

all :: application 

application : $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIB_PATH) $(LIBS)
