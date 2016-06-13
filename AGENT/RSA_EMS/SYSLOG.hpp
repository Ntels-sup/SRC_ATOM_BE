#ifndef __DEF_MONITOR_SYSLOG__
#define __DEF_MONITOR_SYSLOG__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

#include <sys/timeb.h>

typedef enum
{
	IDX_SYSLOG = 0,
	MAX_SYSLOG_IDX
}SYSLOG_IDX;

const char *SYSLOG_COLUMN[] =
{
	"nothing"
};



typedef struct _snmp_value
{
    bool bFind;
}SYSLOG_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class SYSLOG : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
		std::string m_strSysLogPath;		
		std::string m_strOldSysLogPath;		
		std::string m_strPrevLogLine;		

        int m_nInitDate;
        int m_nInitTime;

        time_t tPrevTime;
	public :
		SYSLOG();
		virtual ~SYSLOG();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        
        char *GetAscTime(const char *a_szFormat, struct timeb a_tCur);
        int MakeTrapJson();
};


#endif
