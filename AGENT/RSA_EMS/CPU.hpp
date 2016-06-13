#ifndef __DEF_MONITOR_CPU__
#define __DEF_MONITOR_CPU__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"

typedef enum
{
	IDX_CPU_NO = 0,
	IDX_CPU_USAGE,
	IDX_CPU_USER,
	IDX_CPU_SYS,
	IDX_CPU_NICE,
	IDX_CPU_IDLE,
	IDX_CPU_WAIT,
	MAX_CPU_IDX
}CPU_IDX;

const char *CPU_COLUMN[] =
{
	"cpu_no",
	"usage",
	"user",
	"sys",
	"nice",
	"idle",
	"wait"
};

typedef struct _cpu_value
{
	vector<uint64_t> vecOldValue;
	vector<double> vecSumValue;
	vector<double> vecAvgValue;
	vector<double> vecCurValue;
}CPU_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class CPU : public RSABase
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

		bool m_bPrintTop;
		time_t m_tNextPrint;
		int m_nPrintCount;
		double m_dThreshold;
	public :
		CPU();
		virtual ~CPU();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();

		int ConnectDB();
		int InsertTopData();
		int MakeStatJson(time_t a_tCur);
		int PrintTopData(time_t a_tCur);
};


#endif
