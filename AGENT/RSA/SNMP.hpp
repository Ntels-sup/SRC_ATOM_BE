#ifndef __DEF_MONITOR_SNMP__
#define __DEF_MONITOR_SNMP__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"


typedef enum
{
	IDX_SNMP = 0,
	MAX_SNMP_IDX
}SNMP_IDX;

const char *SNMP_COLUMN[] =
{
	"nothing"
};


typedef struct _snmp_value
{
	bool bFind;
	std::string strValue;
}SNMP_VALUE;

double Rounding (double x, int digit)
{
    return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class SNMP : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
		std::string m_strTrapLogPath;		

	public :
		SNMP();
		virtual ~SNMP();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int MakeTrapJson();

};


#endif
