#include <string.h>
#include "CGlobal.hpp"
#include "STM.h"
#include "CResourceInfo.hpp"

CResourceInfo::CResourceInfo()
{
    m_nHourlyCollectTime = 0;
    m_nDailyCollectTime = 0;
    m_lastHourlyCollectTime = 0;
    m_lastDailyCollectTime = 0;
    m_nCheckTime = 0;
    m_nDropTime = 0;

}

CResourceInfo::~CResourceInfo()
{
    DropAll();
}

int CResourceInfo::SetCollectTime(time_t a_nHourlyCollectTime, time_t a_nDailyCollectTime, 
		time_t a_nCheckTime, time_t a_nDropTime)
{
    m_nHourlyCollectTime = a_nHourlyCollectTime;
    m_nDailyCollectTime = a_nDailyCollectTime; 
    m_nCheckTime = a_nCheckTime;
    m_nDropTime = a_nDropTime;

    return STM_OK;
}

int CResourceInfo::SetDbInfo(string &a_strDbName, string &a_strDbIp,
		int a_nDbPort, string &a_strDbUser, string &a_strDbPasswd)
{
    m_strDbName = a_strDbName;
    m_strDbIp = a_strDbIp;
    m_nDbPort = a_nDbPort;
    m_strDbUser = a_strDbUser;
    m_strDbPasswd = a_strDbPasswd;

    return STM_OK;
}

int CResourceInfo::DropAll()
{
    struct ST_RSC_INFO *stRscInfo = NULL;

    while(m_lstRscInfo.size() != 0){
        stRscInfo = m_lstRscInfo.front();

        delete stRscInfo;

        m_lstRscInfo.pop_front();
    }

    return STM_OK;
}

int CResourceInfo::InsertRsc(const char *a_szRscGrpId, const char *a_szRscGrpName,  bool a_blnStatYn, int a_nStatPeriod)
{
    struct ST_RSC_INFO *stRscInfo = NULL;

    stRscInfo = new struct ST_RSC_INFO;

    strcpy(stRscInfo->szRscGrpId, a_szRscGrpId);
    strcpy(stRscInfo->szRscGrpName, a_szRscGrpName);
    stRscInfo->blnStatYn = a_blnStatYn;

    stRscInfo->nStatPeriod = a_nStatPeriod;

    m_lstRscInfo.push_back(stRscInfo);

    return STM_OK;
}

int CResourceInfo::SendResStsProcess(struct ST_RSC_INFO *a_stRscInfo)
{

    return STM_OK;
}

int CResourceInfo::StsSendHandler(time_t curTime)
{
    list<struct ST_RSC_INFO*>::iterator iter;
    struct ST_RSC_INFO *stRscInfo = NULL;
    list<string> *lstSendResGrpId = NULL;
    string strRscGrpId;

    for(iter = m_lstRscInfo.begin(); iter != m_lstRscInfo.end(); iter++){
        stRscInfo = *iter;

        if((stRscInfo->blnStatYn == true) &&
                ((curTime % stRscInfo->nStatPeriod) == 0)){
            if(lstSendResGrpId == NULL){
                lstSendResGrpId = new list<string>;
            }

            strRscGrpId = stRscInfo->szRscGrpId;
            lstSendResGrpId->push_back(strRscGrpId);
        }
    }

    if(lstSendResGrpId != NULL){
        m_lstSendQueue.push_back(lstSendResGrpId);
    }

    return STM_OK;
}

list<string>* CResourceInfo::GetFirstSendRscGrpId()
{
    list<string> *lstSendResGrpId = NULL;

    if(m_lstSendQueue.size() == 0){
        return NULL;
    }

    lstSendResGrpId = m_lstSendQueue.front();

    m_lstSendQueue.pop_front();

    return lstSendResGrpId;
}


int CResourceInfo::CheckMarge(time_t curTm)
{
    time_t dailyStartTime = 0;
    time_t hourlyStartTime = 0;
    time_t dropTime = 0;
    time_t hourlyStopTime;
    time_t dailyStopTime;
    struct tm *curT = NULL;
    struct tm *lastHourT = NULL;
    CGlobal *cGlob = NULL;
    CThreadQueue<CMergeSession*> *cMergeSndQueue = NULL;
    CMergeSession *cMergeSession = NULL;
    CMergeTableSession *cMergeTableSession = NULL;

    cGlob = CGlobal::GetInstance();

    curT = localtime(&curTm);

    if((m_nHourlyCollectTime == 0) ||
            (m_nDailyCollectTime == 0)){
        STM_LOG(STM_ERR,"Resource merge faeild (Invalid collect time)\n");
        return STM_OK;
    }

    if(curT->tm_hour != m_nCheckTime){
        return STM_OK;
    }

    lastHourT = localtime(&m_lastHourlyCollectTime);

    if((curT->tm_mday-m_nHourlyCollectTime)  == lastHourT->tm_mday){
        /* already colleted */
        return STM_OK;
    }

    dropTime = ((curTm-(curTm%86400)) - ((m_nDropTime * 86400) + curT->tm_gmtoff)) ;

    if((m_lastHourlyCollectTime == 0) ||
            (m_lastDailyCollectTime == 0)){
        dailyStartTime = dropTime;
        hourlyStartTime = ((curTm-(curTm%86400)) - ((m_nDailyCollectTime * 86400) + curT->tm_gmtoff));
    }
    else {
        dailyStartTime = m_lastDailyCollectTime;
        hourlyStartTime = m_lastHourlyCollectTime;
    }

    /* calc time */
    hourlyStopTime = ((curTm-(curTm%86400)) - ((m_nHourlyCollectTime * 86400) + curT->tm_gmtoff));
    dailyStopTime = ((curTm-(curTm%86400)) - ((m_nDailyCollectTime * 86400) + curT->tm_gmtoff));

    DEBUG_LOG("RSC HOUR  START TIME=%s",ctime(&hourlyStartTime));
    DEBUG_LOG("RSC HOUR  STOP  TIME=%s\n",ctime(&hourlyStopTime));
    DEBUG_LOG("RSC DAILY START TIME=%s",ctime(&dailyStartTime));
    DEBUG_LOG("RSC DAILY STOP  TIME=%s\n",ctime(&dailyStopTime));
    DEBUG_LOG("RSC DROP        TIME=%s\n",ctime(&dropTime));

    /* make package merge session */
    cMergeSession = new CMergeSession(STM_TABLE_TYPE_RES, "ATOM", m_strDbName, m_strDbIp, m_nDbPort, m_strDbUser, m_strDbPasswd);

    cMergeTableSession = new CMergeTableSession();

    /* make session */
    cMergeTableSession->SetTime(hourlyStartTime, hourlyStopTime, dailyStartTime, dailyStopTime, dropTime);

    cMergeSession->SetTableSession(cMergeTableSession);

    cMergeSndQueue = cGlob->GetMergeSndQueueP();
    cMergeSndQueue->Push(cMergeSession);

    /* update last collect time */
    m_lastHourlyCollectTime = hourlyStopTime;
    m_lastDailyCollectTime = dailyStopTime;

    return STM_OK;
}

