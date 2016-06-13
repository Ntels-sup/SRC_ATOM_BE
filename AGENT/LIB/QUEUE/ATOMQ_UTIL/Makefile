include ../include.mk

CFLAGS += -latomqueue
CFLAGS += -latomqueue_common
CFLAGS += -latomdb
CFLAGS += -latomconfig
default : all

EXES = atomq_util

OBJS = main.o

.PHONY: all clean distclean

all :: $(EXES)

$(EXES) : $(OBJS)
	$(CXX) -o $@ $^ -Wall -O2 $(CFLAGS) -L/usr/lib64 $(INCDIR) $(LDFLAGS)

install ::
	cp $(EXES) ../../../../../BIN
