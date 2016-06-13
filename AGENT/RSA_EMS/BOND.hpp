#ifndef __DEF_MONITOR_BOND__
#define __DEF_MONITOR_BOND__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"
#include <net/if.h>
#include <sys/ioctl.h>

typedef enum
{
	IDX_BOND_STATUS = 0,
	IDX_BOND_SLAVE,
	IDX_BOND_ACTIVE_SLAVE,
	MAX_BOND_IDX
}BOND_IDX;

const char *BOND_COLUMN[] =
{
	"status",
	"slave",
	"active_slave"
};


typedef struct _bond_value
{
	vector<string> vecStringValue;
}BOND_VALUE;

class BOND : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		BOND();
		virtual ~BOND();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int GetSlaves(BOND_VALUE *a_pstData, char *a_pszIF);
        int MakeTrapJson();
};


#endif
