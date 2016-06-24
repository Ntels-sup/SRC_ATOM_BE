#ifndef __DEF_MONITOR_PHY_DRIVE__
#define __DEF_MONITOR_PHY_DRIVE__ 1

#include "RSABase.hpp"
#include "RSARoot.hpp"

typedef enum
{
	IDX_PHY_DRIVE_STATUS = 0,
	IDX_PHY_DRIVE_PORT,
	IDX_PHY_DRIVE_BOX,
	IDX_PHY_DRIVE_BAY,
	MAX_PHY_DRIVE_IDX
}PHY_DRIVE_IDX;

const char *PHY_DRIVE_COLUMN[] =
{
	"status",
	"port",
	"box",
	"bay"
};


typedef struct _phy_drive_value
{
	vector<string> vecStringValue;
}PHY_DRIVE_VALUE;

class PHY_DRIVE : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSARoot *m_pclsMain;
	public :
		PHY_DRIVE();
		virtual ~PHY_DRIVE();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
        int MakeTrapJson();
};


#endif
