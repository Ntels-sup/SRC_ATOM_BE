#ifndef __CRESOURCE_INFO_HPP__
#define __CRESOURCE_INFO_HPP__

#include <time.h>
#include <string>
#include <list>

using namespace std;

class CResourceInfo{
	private:
		struct ST_RSC_INFO{
			char szRscGrpId[RSC_GRP_ID_LEN];
			char szRscGrpName[RSC_GRP_ID_NAME_LEN];
			bool blnStatYn;
			int nStatPeriod; /* sec */
		};

		int m_nHourlyCollectTime;
		int m_nDailyCollectTime;
		time_t m_lastHourlyCollectTime;
		time_t m_lastDailyCollectTime;
		int m_nCheckTime;
		int m_nDropTime;

		/* Resource stat database */
		string m_strDbName;
		string m_strDbIp;
		int m_nDbPort;
		string m_strDbUser;
		string m_strDbPasswd;

		list<struct ST_RSC_INFO*> m_lstRscInfo;
		list<list<string>*> m_lstSendQueue;

		int SendResStsProcess(struct ST_RSC_INFO *a_stRscInfo);
	public:
		CResourceInfo();
		~CResourceInfo();
		int SetCollectTime(time_t a_nHourlyCollectTime, time_t a_nDailyCollectTime, 
				time_t a_nCheckTime, time_t a_nDropTime);
		int SetDbInfo(string &a_strDbName, string &a_strDbIp,
				int a_nDbPort, string &a_strDbUser, string &a_strDbPasswd);
		int DropAll();
		int InsertRsc(const char *a_szRscGrpId, const char *a_szRscGrpName,  bool a_blnStatYn, int a_nStatPeriod);
		int StsSendHandler(time_t curTime);
		int CheckMarge(time_t curTm);
		list<string>* GetFirstSendRscGrpId();
};

#endif
