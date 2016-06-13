#ifndef __DEF_MONITOR_EX_DISK__
#define __DEF_MONITOR_EX_DISK__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_EX_DISK_STATUS = 0,
	IDX_EX_DISK_DEVICE,
	IDX_EX_DISK_FILE_SYSTEM,
	IDX_EX_DISK_UTIL,
	MAX_EX_DISK_IDX
}EX_DISK_IDX;

const char *EX_DISK_COLUMN[] =
{
	"status",
	"device",
	"file_system",
	"util"
};


typedef struct _ex_disk_value
{
	vector<string> vecStringValue;
}EX_DISK_VALUE;

class EX_DISK : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		EX_DISK();
		virtual ~EX_DISK();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
