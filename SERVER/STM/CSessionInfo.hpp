#ifndef __CSESSIONINFO_HPP__
#define __CSESSIONINFO_HPP__

#include "STM.h"
#include <list>
#include "CCollectValue.hpp"
#include "CResGroup.hpp"

using namespace std;

class CSessionInfo{
    private :
        int m_nSessionId;
        int m_nType; /* HIST(HISTORY), STS, ALM, RES */
        string m_strPackageName;
        string m_strTableName;
        time_t m_nStartTime;
        int m_nCollectTime;
        time_t m_nHistCollectTime;
        bool m_blnDstFlag;
        bool m_blnActFlag;
        list<string> *m_lstResGrpId;
        list<CCollectValue*> *m_cCollectValue;
        list<CResGroup*> *m_cResGroup;

    public :
        CSessionInfo();
        ~CSessionInfo();
        int MakeSTS(int a_nSessionId, string &a_strPackageName, string &a_strTableName, time_t a_nStartTime, 
                unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlag);
        int MakeRSC(int a_nSessionId, time_t a_nStartTime, bool a_nDstFlag, list<string> *a_lstRscGrpId);
        int MakeHIST(int a_nSessionId, string &a_strPackageName, string &a_strTableName, 
                time_t a_nCollectTime, bool a_blnDstFlag);
        list<string> *GetResGrpIdListP() { return m_lstResGrpId; };
        string GetPackageName() { return m_strPackageName; };
        bool GetDstFlag() { return m_blnDstFlag; };
        int GetType() { return m_nType; };
        int GetSessionId() { return m_nSessionId; };
        time_t GetStartTime() { return m_nStartTime; };
        int GetCollectTime() { return m_nCollectTime; };
        string GetTableName(){ return m_strTableName; }
        time_t GetLastTime() { return m_nStartTime + m_nCollectTime; };
        time_t GetHistCollectTime() { return m_nHistCollectTime; };

        void SetCollectValue(list<CCollectValue*> *a_cCollectValue){
            m_cCollectValue = a_cCollectValue;
        }
        list<CCollectValue*>* GetCollectValueP(){ return m_cCollectValue; };
        void SetResGroup(list<CResGroup*> *a_cResGroup){
            m_cResGroup = a_cResGroup;
        };
        list<CResGroup*>* GetResGroupP() { return m_cResGroup; };
};

#endif
