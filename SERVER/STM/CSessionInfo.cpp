#include <stdio.h>
#include <string.h>
#include "CGlobal.hpp"
#include "CSessionInfo.hpp"

CSessionInfo::CSessionInfo()
{
    m_nSessionId = 0;
    m_nType = 0;
    m_nStartTime = 0;
    m_nCollectTime = 0;
    m_blnActFlag = true;
    m_blnDstFlag = false;
    m_cCollectValue = NULL;	
    m_cResGroup = NULL;
    m_lstResGrpId = NULL;
}

CSessionInfo::~CSessionInfo()
{
#if 0
    list<CCollectValue*>::iterator iter;
#endif

    if(m_cCollectValue != NULL){
#if 0
        for(iter = m_cCollectValue->begin();iter != m_cCollectValue->end();){
            delete *iter;

            m_cCollectValue->erase(iter++);
        }
#else 
        while(m_cCollectValue->size()){
            delete m_cCollectValue->front();
            m_cCollectValue->pop_front();
        }
#endif
    }

    if(m_cResGroup != NULL){
        while(m_cResGroup->size() != 0){
            CResGroup *cGroup= NULL;
            cGroup = m_cResGroup ->front();
            delete cGroup;
            m_cResGroup->pop_front();
        }

        delete m_cResGroup;
        m_cResGroup = NULL;
    }

    delete m_cCollectValue;
    delete m_lstResGrpId;
}

int CSessionInfo::MakeRSC(int a_nSessionId, time_t a_nStartTime, bool a_blnDstFlag, list<string> *a_lstRscGrpId)
{
    m_nType = STM_TABLE_TYPE_RES;

    m_lstResGrpId = a_lstRscGrpId;

    m_nStartTime = a_nStartTime;

    m_blnDstFlag = a_blnDstFlag;

    return STM_OK;
}

int CSessionInfo::MakeSTS(int a_nSessionId, string &a_strPackageName, string &a_strTableName, 
				time_t a_nStartTime, unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlg)
{
    m_nSessionId = a_nSessionId;

    m_nType = STM_TABLE_TYPE_STS;

    m_strPackageName = a_strPackageName;

    m_strTableName = a_strTableName;

    m_nStartTime = a_nStartTime;

    if(a_nCollectTime > MAX_COLLECT_TIME){
        STM_LOG(STM_ERR,"Invalid collect time(time=%d\n", a_nCollectTime);
        return STM_NOK;
    }

    m_nCollectTime = a_nCollectTime;

    m_blnDstFlag = a_blnDstFlag;
    m_blnActFlag = a_blnActFlg;

    return STM_OK;
}

int CSessionInfo::MakeHIST(int a_nSessionId, string &a_strPackageName, string &a_strTableName, 
				time_t a_nCollectTime, bool a_blnDstFlag)
{
    m_nSessionId = a_nSessionId;

    m_nType = STM_TABLE_TYPE_HIST;

    m_strPackageName = a_strPackageName;

    m_strTableName = a_strTableName;

    m_nHistCollectTime = a_nCollectTime;

    m_blnDstFlag = a_blnDstFlag;

    return STM_OK;
}

