#ifndef __DEF_MONITOR_NIC__
#define __DEF_MONITOR_NIC__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"
#include <net/if.h>
#include <sys/ioctl.h>

typedef enum
{
	IDX_NIC_STATUS = 0,
	IDX_RX_BYTE,
	IDX_TX_BYTE,
	MAX_NIC_IDX
}NIC_IDX;

const char *NIC_COLUMN[] =
{
	"status",
	"RX",
	"TX"
};


typedef struct _nic_value
{
	char szIPv4[DEF_MEM_BUF_32];
	char szIPv6[DEF_MEM_BUF_64];
	vector<uint64_t> vecOldValue;
	vector<string> vecNewValue;
}NIC_VALUE;

class NIC : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSAMain *m_pclsMain;
        EventAPI *m_pclsEvent;
	public :
		NIC();
		virtual ~NIC();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();

		int SetIPAddress(RESOURCE_ATTR *a_pstResource);
		int MakeIPJson(const char *a_szIFName, const char *a_szVersion, std::string &a_strResult);
};


#endif
