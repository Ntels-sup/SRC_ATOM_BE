#ifndef __DEF_MONITOR_QUEUE__
#define __DEF_MONITOR_QUEUE__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"
#include "CLQManager.hpp"

typedef enum
{
	IDX_QUEUE_USAGE = 0,
	MAX_QUEUE_IDX
}QUEUE_IDX;

const char *QUEUE_COLUMN[] =
{
	"usage"
};

typedef struct _queue_value
{
	vector<string> vecStringValue;
}QUEUE_VALUE;

double Rounding (double x, int digit)
{
        return ( floor( (x) *  pow( float(10), digit ) + 0.5f ) / pow( float(10), digit ) );
}


class QUEUE : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		CLQManager *m_pclsCLQ;
		RSARoot *m_pclsMain;
		CConfig *m_pclsConfig;
//        EventAPI *m_pclsEvent;
	public :
		QUEUE();
		virtual ~QUEUE();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int MakeTrapJson();

};


#endif
