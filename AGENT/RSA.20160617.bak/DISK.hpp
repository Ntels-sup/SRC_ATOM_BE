#ifndef __DEF_MONITOR_DISK__
#define __DEF_MONITOR_DISK__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"
#include <sys/statvfs.h>

typedef enum
{
	IDX_DISK_USAGE = 0,
	IDX_DISK_TOTAL,
	IDX_DISK_FREE,
	IDX_DISK_USED,
	MAX_DISK_IDX
}DISK_IDX;

const char *DISK_COLUMN[] =
{
	"usage",
	"total",
	"free",
	"used"
};

typedef struct _disk_value
{
	vector<string> vecStringValue;
	vector<double> vecSumValue;
	vector<double> vecAvgValue;
}DISK_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class DISK : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		vector<RESOURCE_ATTR*> vecTop;
		uint32_t m_unRunCount;
		RSAMain *m_pclsMain;
		DB *m_pclsDB;
        EventAPI *m_pclsEvent;
	public :
		DISK();
		virtual ~DISK();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int InsertTopData();
		int MakeStatJson(time_t a_tCur);

};


#endif
