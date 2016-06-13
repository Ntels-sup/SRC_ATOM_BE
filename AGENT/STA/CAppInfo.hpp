#ifndef __APPINFO_H__
#define __APPINFO_H__

#include "CEmsInfo.hpp"
#include "CTableList.hpp"

class CAppInfo{
    private :
        CTableList *m_cTableList;
        CEmsInfo *m_cEmsInfo;
        string m_strPackageName;
        int m_nCollectTime;
        int m_nLastSendTime;
        unsigned int m_nMessageBufferLen;
        char m_chMessageBuffer[MAX_MESSAGE_BUFFER_LEN];
        int EncRequestMessage(time_t a_time);
        int DecAnswerMessage(char *a_chData, int a_nDataLen);

    public :
        CAppInfo(CEmsInfo *a_cEmsInfo, string &a_strPackageName, CTableList *a_cCollectTableList, int a_nCollectTime);
        ~CAppInfo();
        int SendCollectMessage();
        int ReceiveAppSts(char *a_chMessage, int a_nMessageLen);
        int SendResourceMessage();
        int ReceiveAppHist(char *a_chMessage, int a_nMessageLen);
        int ReceiveProcess(CProtocol &cProto, int cmdCode);
        int GetRequestMessage(char **a_nEncodeMessage, int *a_nEncodeMessageLen);
};

#endif
