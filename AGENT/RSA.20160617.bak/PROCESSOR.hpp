#ifndef __DEF_MONITOR_PROCESSOR__
#define __DEF_MONITOR_PROCESSOR__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_PROCESSOR_STATUS_NO = 0,
	MAX_PROCESSOR_STATUS_IDX
}PROCESSOR_STATUS_IDX;

const char *PROCESSOR_STATUS_COLUMN[] =
{
	"status"
};

typedef struct _processor_value
{
	vector<char> vecCharValue;
}PROCESSOR_VALUE;

class PROCESSOR : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		PROCESSOR();
		virtual ~PROCESSOR();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
