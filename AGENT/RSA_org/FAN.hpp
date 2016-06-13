#ifndef __DEF_MONITOR_FAN__
#define __DEF_MONITOR_FAN__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_FAN_STATUS_NO = 0,
	MAX_FAN_STATUS_IDX
}FAN_STATUS_IDX;

const char *FAN_STATUS_COLUMN[] =
{
	"status"
};


typedef struct _fan_value
{
	vector<char> vecCharValue;
}FAN_VALUE;

class FAN : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		FAN();
		virtual ~FAN();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
