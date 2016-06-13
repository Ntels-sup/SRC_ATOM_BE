#ifndef __CNODEINFO_HPP__
#define __CNODEINFO_HPP__
#include <list>
#include "CSessionInfo.hpp"
#include "CTCAInfo.hpp"

using namespace std;

class CNodeInfo{
		private:
				int m_nStatus;
				int m_nNodeNo;
				string m_strNodeType;
				time_t m_nLastRcvTime;
				int m_nLastCollectTime;
				int m_nSessionSequenceId;
				string m_strNodeName;
				list<CSessionInfo*> *m_lstQueuedSession;
				list<CSessionInfo*> *m_lstCompleteSession;
				list<CTCAInfo*> *m_lstTCAInfo;

				int SendTCAToALM(string &a_strPackageName, string &a_strTableName, string & strColumnName, 
								int a_nSeverity, int a_nValue);

		public:
				CNodeInfo();
				~CNodeInfo();
				void SetLastRcvTm() { m_nLastRcvTime = time(NULL); };
				int GetState() { return m_nStatus; };
				void SetActiveState() { m_nStatus = ACTIVE_STATUS; };
				void SetInactiveState() { m_nStatus = INACTIVE_STATUS; };
				int Init(int a_nNodeNo, const char *a_strNodeType, const char *a_chNodeName, int a_nLastCollectTime);
				int GetNodeNo() { return m_nNodeNo; };
				string GetNodeName(){ return m_strNodeName; };
				int MakeResSession(list<string> *a_lstSendGrpId, time_t a_startTime, bool a_blnDstFlag);
				int MakeStsSession(string &a_strPackageName, string &a_strTableName, time_t a_nStartTime,
								unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlag);
				int InsertTCAInfo(CTCAInfo *a_cTCA);
				int SendHeartBeat();
				CSessionInfo *RecvSession();
				int SendProcess(CSessionInfo *cSession);
				int CheckTCA(string &a_strPackageName, string &a_strTableName, string &a_strColumnName, int a_nValue);
				CSessionInfo *GetSession(int a_nSessionId);
				int RecvStsHandler(string &a_strData, CSessionInfo **cSession);
				int RecvResStsHandler(string &a_strData, CSessionInfo **cSession);
				int RecvHistHandler(string &a_strData, CSessionInfo **cSession);
				int ResendAllPendingSession();
};

#endif
