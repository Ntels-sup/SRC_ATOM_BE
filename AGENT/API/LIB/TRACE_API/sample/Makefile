include ../../../../../../SRC_ATOM_BE/Makefile.inc

LIB_PATH += 
LIBS    += 

CFLAGS  += -Wall -g -fPIC
LIBS    += -ldl -latomlog -latomsocket

TARGET      = trc_test

SRCS        = CTraceAPI.cpp \
			  trc_test.cpp

OBJS = $(SRCS:.cpp=.o)

all :: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

clean ::
	$(RM) -f $(OBJS) $(TARGET)

install ::
	cp  $(TARGET)  $(ATOM_INSTALL_PATH)

