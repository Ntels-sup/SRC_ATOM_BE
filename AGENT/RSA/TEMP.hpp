#ifndef __DEF_MONITOR_TEMP__
#define __DEF_MONITOR_TEMP__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_TEMP_CPU_NO = 0,
	IDX_TEMP_MAX,
	IDX_TEMP_AVG,
	IDX_TEMP_MIN,
	MAX_TEMP_IDX
}TEMP_IDX;

const char *TEMP_COLUMN[] =
{
	"cpu_no",
	"max",
	"avg",
	"min"
};


typedef struct _temp_value
{
	vector<double> vecSumValue;
	vector<double> vecAvgValue;
	vector<double> vecCurValue;
}TEMP_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class TEMP : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		uint32_t	m_unRunCount;
		RSARoot *m_pclsMain;
	public :
		TEMP();
		virtual ~TEMP();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int MakeStatJson(time_t a_tCur);
        int MakeTrapJson();
};


#endif
