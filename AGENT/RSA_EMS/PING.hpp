#ifndef __DEF_MONITOR_PING__
#define __DEF_MONITOR_PING__ 1

#include "RSABase.hpp"
#include "RSAMain.hpp"

//Include For Db Connection
#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#include "EventAPI.hpp"

#define DEF_PEER_STATUS_INIT	0
#define DEF_PEER_STATUS_FAIL	1
#define DEF_PEER_STATUS_SUCC	2

typedef struct _peer_info
{
	string	strPeerName;
	string	strPeerIP;
	string	strPeerType;
	string	strPkgName;
	int		nPeerNo;
	int		nStatus;
}PEER_INFO;

typedef struct _ping_value
{
	vector<PEER_INFO> vecPeerInfo;
}PING_VALUE;

class PING : public RSABase
{
	private :
		CFileLog *m_pclsLog;
		map<string, RESOURCE_ATTR *> *m_pmapRsc;
		RESOURCE *m_pGroupRsc;
		RSAMain *m_pclsMain;
		EventAPI *m_pclsEvent;
		std::string m_strMyIP;
	public :
		PING();
		virtual ~PING();
		virtual int Initialize(CFileLog *a_pclsLog, RESOURCE *a_pRsc, void *a_pclsMain);
		virtual int MakeJson(time_t a_tCur);
		virtual int Run();
		int GetPeerInfo(std::vector<PEER_INFO> &a_vecPeerInfo);
		int GetDBConnInfo(char *a_pszArgs, char **a_pszConnInfo);
		int MakePingJson(std::string &a_strResult);
		int MakeNodeJson(const char *a_szPkgName, const char *a_szPeerType, std::string &a_strResult);
};


#endif
