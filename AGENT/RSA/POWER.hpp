#ifndef __DEF_MONITOR_POWER__
#define __DEF_MONITOR_POWER__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_POWER_STATUS_NO = 0,
	MAX_POWER_STATUS_IDX
}POWER_STATUS_IDX;

const char *POWER_STATUS_COLUMN[] =
{
	"status"
};

typedef struct _power_value
{
	vector<char> vecCharValue;
}POWER_VALUE;

class POWER : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		POWER();
		virtual ~POWER();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
