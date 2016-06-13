#ifndef __DEF_MONITOR_DIMM__
#define __DEF_MONITOR_DIMM__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_DIMM_NO = 0,
	IDX_DIMM_MODULE,
	IDX_DIMM_STATUS,
	MAX_DIMM_IDX
}DIMM_IDX;

const char *DIMM_COLUMN[] =
{
	"processor",
	"module",
	"status"
};

typedef struct _dimm_value
{
	vector<string> vecStringValue;
}DIMM_VALUE;

class DIMM : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		DIMM();
		virtual ~DIMM();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
