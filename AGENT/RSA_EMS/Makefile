#
# ATOM Node Agent
#
include ../../Makefile.inc

CXXFLAGS += -pg -fPIC
CXXFLAGS += -I$(MYSQL_INC)
CXXFLAGS += -L$(MYSQL_LIB)
CXXFLAGS += -L../API/LIB
CXXFLAGS += -I../API/INCLUDE
CXXFLAGS += -I../INCLUDE/QUEUE
CXXFLAGS += -I../INCLUDE
CXXFLAGS += -I../LIB
CXXFLAGS += -D_USE_HPACUCLI

LIBS     += -latomlog
LIBS     += -latomdb
LIBS     += -latomeventapi
LIBS     += -lmysqlclient
LIBS     += -latomsocket
LIBS     += -latomqueue
LIBS     += -latomqueue_common
LIBS     += -latomlog
LIBS     += -latomconfig
LIBS     += -latomstmresapi
LIBS     += -latomutil
LIBS     += -latomcliapi
LIBS     += -lrt


TARGET = ATOM_NM_RSA.so
RSA_ROOT = RSA_ROOT

SHARED_CPU = PLUG0001_CPU.so
SHARED_MEM = PLUG0002_MEM.so
SHARED_TEMP = PLUG0003_TEMP.so
SHARED_TOTAL_CPU = PLUG0004_TOT_CPU.so
SHARED_DISK = PLUG0005_DISK.so
SHARED_DB = PLUG0006_DB.so
SHARED_FAN = PLUG0007_FAN.so
SHARED_DIMM = PLUG0008_DIMM.so
SHARED_POWER = PLUG0009_POWER.so
SHARED_PROCESSOR = PLUG0010_PROCESSOR.so
SHARED_NIC = PLUG0011_NIC.so
SHARED_BOND = PLUG0012_NIC_BOND.so
SHARED_PHY_DRIVE = PLUG0013_PHY_DRIVE.so
SHARED_EX_DISK = PLUG0014_EX_DISK.so
SHARED_INVENTORY = PLUG0015_INVENTORY.so
SHARED_QUEUE = PLUG0016_QUEUE.so
SHARED_PING = PLUG0017_PING.so
SHARED_SNMP = PLUG0018_SNMP.so
SHARED_SYSLOG = PLUG0019_SYSLOG.so

USER_SHARED_LIB = $(SHARED_CPU) \
					$(SHARED_MEM) \
					$(SHARED_TOTAL_CPU) \
					$(SHARED_TEMP) \
					$(SHARED_DISK) \
					$(SHARED_DB) \
					$(SHARED_FAN) \
					$(SHARED_DIMM) \
					$(SHARED_POWER) \
					$(SHARED_PROCESSOR) \
					$(SHARED_NIC) \
					$(SHARED_BOND) \
					$(SHARED_PHY_DRIVE) \
					$(SHARED_EX_DISK) \
					$(SHARED_INVENTORY) \
					$(SHARED_QUEUE) \
					$(SHARED_PING) \
					$(SHARED_SNMP) \
					$(SHARED_SYSLOG) \

OBJS =	RSAMain.o RSAMsg.o 
ROOT_OBJS =	RSARoot.o 

all :: $(TARGET) $(USER_SHARED_LIB) $(RSA_ROOT)

#$(TARGET) : $(OBJS)
#	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 
$(TARGET) : $(OBJS)
	$(CXX) -o $@ $^ -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(RSA_ROOT) : $(ROOT_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS) 

$(SHARED_CPU) : CPU.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_MEM) : MEMORY.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_TOTAL_CPU) : TOT_CPU.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_TEMP) : TEMP.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_DISK) : DISK.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_DB) : TABLESPACE.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_FAN) : FAN.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_DIMM) : DIMM.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_POWER) : POWER.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_PROCESSOR) : PROCESSOR.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_NIC) : NIC.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_BOND) : BOND.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_PHY_DRIVE) : PHY_DRIVE.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_EX_DISK) : EX_DISK.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_INVENTORY) : INVENTORY.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_QUEUE) : QUEUE.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_PING) : PING.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_SNMP) : SNMP.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

$(SHARED_SYSLOG) : SYSLOG.cpp
	$(CXX) -o $@ $^ -fPIC -shared $(CXXFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBS)

install ::
	cp $(TARGET) $(ATOM_INSTALL_PATH)
