#ifndef __DEF_MONITOR_MEM__
#define __DEF_MONITOR_MEM__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"

typedef enum
{
	IDX_MEM_USAGE = 0,
	IDX_MEM_TOTAL,
	IDX_MEM_FREE,
	IDX_MEM_USED,
	MAX_MEM_IDX
}MEM_IDX;

const char *MEM_COLUMN[] =
{
	"usage",
	"total",
	"free",
	"used"
};

typedef struct _memory_value
{
	vector<double> vecSumValue;
	vector<double> vecAvgValue;
	vector<double> vecCurValue;
}MEMORY_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class MEMORY : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		vector<RESOURCE_ATTR*> vecTop;
		uint32_t  m_unRunCount;	
		RSAMain *m_pclsMain;
		DB *m_pclsDB;
        EventAPI *m_pclsEvent;
	public :
		MEMORY();
		virtual ~MEMORY();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();

		int ConnectDB();
		int InsertTopData();
		int MakeStatJson(time_t a_tCur);
};


#endif
