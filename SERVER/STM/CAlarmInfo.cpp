#include <string.h>
#include "CGlobal.hpp"
#include "STM.h"
#include "CAlarmInfo.hpp"

CAlarmInfo::CAlarmInfo()
{
    m_nHourlyCollectTime = 0;
    m_nDailyCollectTime = 0;
    m_lastHourlyCollectTime = 0;
    m_lastDailyCollectTime = 0;
    m_nCheckTime = 0;
    m_nDropTime = 0;

}

CAlarmInfo::~CAlarmInfo()
{
}

int CAlarmInfo::SetCollectTime(time_t a_nHourlyCollectTime, time_t a_nDailyCollectTime, 
		time_t a_nCheckTime, time_t a_nDropTime)
{
    m_nHourlyCollectTime = a_nHourlyCollectTime;
    m_nDailyCollectTime = a_nDailyCollectTime; 
    m_nCheckTime = a_nCheckTime;
    m_nDropTime = a_nDropTime;

    return STM_OK;
}

int CAlarmInfo::SetDbInfo(string &a_strDbName, string &a_strDbIp,
		int a_nDbPort, string &a_strDbUser, string &a_strDbPasswd)
{
    m_strDbName = a_strDbName;
    m_strDbIp = a_strDbIp;
    m_nDbPort = a_nDbPort;
    m_strDbUser = a_strDbUser;
    m_strDbPasswd = a_strDbPasswd;

    return STM_OK;
}

int CAlarmInfo::CheckMarge(time_t curTm)
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
        STM_LOG(STM_ERR,"Invalid collect time\n");
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

    DEBUG_LOG("ALARM HOUR  START TIME=%s",ctime(&hourlyStartTime));
    DEBUG_LOG("ALARM HOUR  STOP  TIME=%s\n",ctime(&hourlyStopTime));
    DEBUG_LOG("ALARM DAILY START TIME=%s",ctime(&dailyStartTime));
    DEBUG_LOG("ALARM DAILY STOP  TIME=%s\n",ctime(&dailyStopTime));
    DEBUG_LOG("ALARM DROP        TIME=%s\n",ctime(&dropTime));

    /* make package merge session */
    cMergeSession = new CMergeSession(STM_TABLE_TYPE_ALM, "ATOM", m_strDbName, m_strDbIp, m_nDbPort, m_strDbUser, m_strDbPasswd);

    cMergeTableSession = new CMergeTableSession();

    /* make session */
    cMergeTableSession->SetTime(hourlyStartTime, hourlyStopTime, dailyStartTime, dailyStopTime, dropTime);

    cMergeSession->SetTableSession(cMergeTableSession);

    /* running */
    cMergeSndQueue = cGlob->GetMergeSndQueueP();
    cMergeSndQueue->Push(cMergeSession);

    /* update last collect time */
    m_lastHourlyCollectTime = hourlyStopTime;
    m_lastDailyCollectTime = dailyStopTime;

    return STM_OK;
}

