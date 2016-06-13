include ../include.mk

CFLAGS += -latomqueue_common
CFLAGS += -latomconfig
default : all

EXES = atomq_init

OBJS = CAtomQInit.o

.PHONY: all clean distclean

all :: $(EXES)

$(EXES) : $(OBJS)
	$(CXX) -o $@ $^ -Wall -O2 $(CFLAGS) -L/usr/lib64 $(INCDIR) $(LDFLAGS)

install ::
	cp $(EXES) ../../../../../BIN
