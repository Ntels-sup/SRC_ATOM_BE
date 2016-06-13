#ifndef __DEF_MONITOR_INVEN__
#define __DEF_MONITOR_INVEN__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

//Include For To Get Python Version
#include <python2.6/Python.h>
//Include For Db Connection
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#define DEF_MAX_FILE_COUNT	20	

#define DEF_COMMON_CCD_CPU 		1
//#define DEF_COMMON_CCD_DISK 	2
#define DEF_COMMON_CCD_KERNEL	2
#define DEF_COMMON_CCD_MEMORY	3
#define DEF_COMMON_CCD_MYSQL	4
#define DEF_COMMON_CCD_OS 		5
#define DEF_COMMON_CCD_PYTHON 	6
//#define DEF_COMMON_CCD_IP 		8

typedef struct _inven_value
{
	vector<string> vecStringValue;
}INVENTORY_VALUE;

class INVENTORY : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		DB *m_pclsDB;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		INVENTORY();
		virtual ~INVENTORY();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();

		int GetDBConnInfo(char *a_pszArgs, char **a_pszConnInfo);
		int GetCPUInfo();
//		int GetDiskInfo(DB *a_pclsDB);
		int GetKernelInfo();
		int GetMemoryInfo();
		int GetDBInfo();
		int GetOSInfo();
		int GetPythonInfo();
		int GetIPInfo();

};


#endif
