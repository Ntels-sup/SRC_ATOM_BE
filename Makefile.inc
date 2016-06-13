# Makefile.inc

ARCH 		:= $(shell uname -m | sed -e s/i.86/i386/ )
OS			:= $(shell uname -s )
PREFIX		:= $(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))

MYSQL_INC	=/MYSQL/mysql-5.6.24/include
MYSQL_LIB	=/MYSQL/mysql-5.6.24/lib

ATOM_SRC_HOME		= $(HOME)/SRC_ATOM_BE
ATOM_INSTALL_PATH	= $(HOME)/BIN

# alias for command
CC			= ccdv gcc
CXX			= ccdv g++
AR			= ar
RANLIB		= ranlib

# include path
INC_PATH 	= -I$(ATOM_SRC_HOME)/INCLUDE
INC_PATH	+= -I$(ATOM_SRC_HOME)/SERVER/INCLUDE
INC_PATH	+= -I$(ATOM_SRC_HOME)/AGENT/INCLUDE

# library path
LIB_PATH	= -L$(ATOM_SRC_HOME)/LIB
LIB_PATH	+= -L$(ATOM_SRC_HOME)/SERVER/LIB
LIB_PATH	+= -L$(ATOM_SRC_HOME)/AGENT/LIB

# link library
LIBS 		= -lpthread
LIBS 		= -lrt

# compile option
#CFLAGS		= -std=c++0x -Wall -O2
CFLAGS		= -std=c++0x -Wall -fPIC
#CFLAGS		= -std=c++98 -Wall -O2
#CFLAGS		+= -march=x86-64 -msse3
#CFLAGS		+= -pg

# compile option - define
CFLAG		+= -DATOM_RELEASE
CFLAGS		+= -DATOM_DEVELOPMENT

CXXFLAGS	= $(CFLAGS)
MAKEFLAGS 	+= --no-print-directory

# compile suffix
.cpp.o:
	$(CXX) $(CXXFLAGS) $(LOCALFLAGS) $(INC_PATH) -c $<
.c.o:
	$(CC) $(CFLAGS) $(LOCALFLAGS) $(INC_PATH) -c $<
.S.o:
	$(AS) $(INCDIR) -c $<
.s.o:
	$(AS) $(LIBDIR) -c $<

.PHONY:	all clean install


all clean install ::
	@set -e;						\
	for i in $(SUBDIR); 			\
	do ( 							\
		if [ $$i"NULL" != "NULL" ];	\
		then 						\
			echo "    CD " $(CURDIR)/$$i;	\
			$(MAKE) -C $$i $@;		\
		fi 							\
	); 								\
	done;							\

clean ::
	@echo "        CLEAN   " $(CURDIR)
	rm -f $(TARGET) *.o *.gcov *.gcda *.gcno *.a *.so

#install ::
#	@set -e;									\
#	if [ "NULL$(TARGET)" != "NULL" ];				\
#	then										\
#		echo "        INSTALL $(TARGET)";			\
#		cp $(TARGET) $(ATOM_INSTALL_PATH);				\
#	fi
