#ifndef __CEMSINFO_HPP__
#define __CEMSINFO_HPP__

#include <list>
#include <string>
#include "CTableList.hpp"
#include "CSessionInfo.hpp"

using namespace std;

class CEmsInfo{
    private:
        int m_nCollectWaitTime;
        string  m_strPackageName;
        int m_nStmNo;
        int m_nAtomNodeNo;
        CTableList *m_cTableList;
        list<CSessionInfo*> m_lstPendingQueue;
        unsigned int m_nMessageBufferLen;
        char m_chMessageBuffer[MAX_MESSAGE_BUFFER_LEN];

    public:
        CEmsInfo(CTableList *a_cTableList, string &a_strPackageName, int &nRet);
        ~CEmsInfo();
        int CollectAnswer(CSessionInfo *cSession);
        void SetCollectWaitTime(int a_nCollectWaitTime) { m_nCollectWaitTime = a_nCollectWaitTime; };
        int DirectSendProcess(string a_strTableName, string a_strPackageName,
                int a_nType, list<string> *a_lstPrimaryKey, list<int> *a_lstValue);
        int ReceiveProcess();
        int ReceiveStaRequestProcess(CProtocol &cProto);
        int ReceiveResourceProcess(CProtocol &cProto);
        int CollectProcess();
        int SendProcess(int a_nCmdCode, int a_nFlag, char *a_chMessageBuffer, int a_nMessageBufferLen);
        int SendErrorToStm(int a_nSessionId, int a_nErrorCode);
        int SendHeartBeatRsp();
};

#endif
