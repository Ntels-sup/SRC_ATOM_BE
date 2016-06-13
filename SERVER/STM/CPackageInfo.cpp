#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CGlobal.hpp"
#include "CMergeSession.hpp"
#include "CMergeTableSession.hpp"
#include "CMerge.hpp"
#include "CPackageInfo.hpp"

CPackageInfo::CPackageInfo()
{
	m_lstTableInfo = new list<CTableInfo*>;
	m_lstNodeInfo = new list<CNodeInfo*>;
	m_cDb = new MariaDB();
	m_lastDailyCollectTime = 0;
	m_nDailyCollectTime = 0;
	m_lastHourlyCollectTime = 0;
	m_nHourlyCollectTime = 0;
	m_nDropTime = 0;
	m_nCheckTime = 0;
	m_blnCollectFlag = false;

}

CPackageInfo::~CPackageInfo()
{
    delete m_lstTableInfo;
    delete m_lstNodeInfo;
    delete m_cDb;
}

int CPackageInfo::Init(const char *a_szPackageName, const char *a_szDBName, const char *a_szDBIp, int a_nDBPort, 
				const char *a_szDBUser, const char *a_szDBPassword)
{
    bool blnRet = 0;
    int nLen = 0;

    if(strlen(a_szDBName) == 0){
        STM_LOG(STM_ERR,"DB Name not exist\n");
        return STM_NOK;
    }

    m_strDBName = a_szDBName;
    m_strDBIp = a_szDBIp;
    m_nDBPort = a_nDBPort;
    m_strDBUser = a_szDBUser;
    m_strDBPassword = a_szDBPassword;

    blnRet = m_cDb->Connect(a_szDBIp, a_nDBPort, a_szDBUser, a_szDBPassword, a_szDBName);
    if(blnRet == false){
        STM_LOG(STM_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n",
                a_szDBIp, a_nDBPort, a_szDBUser, a_szDBPassword, a_szDBName);
    }

    if(a_szPackageName == NULL){
        STM_LOG(STM_ERR,"Package Name not exist\n");
        return STM_NOK;
    }

    nLen = strlen(a_szPackageName);
    if(nLen >= PACKAGE_NAME_LEN){
        STM_LOG(STM_ERR,"Invalid Package name length(nLen=%d, max=%d)\n", nLen, PACKAGE_NAME_LEN);
        return STM_NOK;
    }

    m_strPackageName = a_szPackageName;

    return STM_OK;
}

int CPackageInfo::InitConfig(int a_nDailyCollectTime, time_t a_lastDailyCollectTime,
                                int a_nHourlyCollectTime, time_t a_lastHourlyCollectTime,
                                int a_nDropTime, int a_nCheckTime, int a_nHistDropTime)
{
    m_nDailyCollectTime = a_nDailyCollectTime;
    m_lastDailyCollectTime = a_lastDailyCollectTime;
    m_nHourlyCollectTime = a_nHourlyCollectTime;
    m_lastHourlyCollectTime = a_lastHourlyCollectTime;
    m_nDropTime = a_nDropTime;
    m_nCheckTime = a_nCheckTime;
    m_nHistDropTime = a_nHistDropTime;

    m_blnCollectFlag = true;

    return STM_OK;
}

int CPackageInfo::CheckName(string &a_strPackageName)
{
    int nRet = 0;


    if(a_strPackageName.size() != m_strPackageName.size()){
        return STM_NOK;
    }

    nRet = strcasecmp(m_strPackageName.c_str(), a_strPackageName.c_str());
    if(nRet == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CPackageInfo::CheckName(const char *a_chPackageName, unsigned int a_nPackageNameLen)
{
    int nRet = 0;


    if(a_nPackageNameLen != m_strPackageName.size()){
        return STM_NOK;
    }

    nRet = strncasecmp(m_strPackageName.c_str(), a_chPackageName, a_nPackageNameLen);
    if(nRet == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CPackageInfo::InsertTable(CTableInfo *cTable)
{
    if(cTable == NULL){
        STM_LOG(STM_ERR,"Table not exist\n");
        return STM_NOK;
    }

    m_lstTableInfo->push_back(cTable);

    return STM_OK;
}

int CPackageInfo::InsertNode(CNodeInfo *cNode)
{
    if(cNode == NULL){
        STM_LOG(STM_ERR,"NODE not exist\n");
        return STM_NOK;
    }

    m_lstNodeInfo->push_back(cNode);
    return STM_OK;
}

int CPackageInfo::SendCollectMessage(string &a_strTableName,
				time_t a_nStartTime, unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlag)
{
    int nRet = 0;
    list<CNodeInfo*>::iterator iter;
    CNodeInfo *findNode = NULL;

    for(iter=m_lstNodeInfo->begin();iter != m_lstNodeInfo->end();iter++){
        findNode = *iter;
        nRet = findNode->MakeStsSession(m_strPackageName,
                a_strTableName, a_nStartTime, a_nCollectTime,  a_blnDstFlag, a_blnActFlag);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Session make failed(ret=%d,tableName=%s, startTime=%lu, collectTime=%d, actFlag=%d\n",
                    nRet, a_strTableName.c_str(), a_nStartTime, a_nCollectTime,  a_blnActFlag);
        }
    }

    return STM_OK;
}

int CPackageInfo::Run()
{
    int nRet = 0;
    int nLoopCnt = 0;
    time_t tm = 0;
    string strTableName;
    bool dstFlag = false;
    CTableInfo *cFindTable = NULL;
    list<CTableInfo*>::iterator iter;
    struct tm *t = NULL;

    tm = time(NULL);

    t = localtime(&tm);
    if(t->tm_isdst == 0){
        dstFlag = false;
    }
    else{
        dstFlag = true;
    }

    for(iter=m_lstTableInfo->begin();iter != m_lstTableInfo->end();iter++){
        cFindTable = *iter;

        if((cFindTable->GetType() == STM_TABLE_TYPE_STS) &&
                (cFindTable->GetLastSendTime() != tm) && 
                (cFindTable->GetCollectTime() != 0) &&
                ((tm % cFindTable->GetCollectTime()) == 0)){
            /* send message */
            strTableName = cFindTable->GetTableName();

            nRet = SendCollectMessage(strTableName, (unsigned int)(tm - cFindTable->GetCollectTime()), 
                    cFindTable->GetCollectTime(), dstFlag, cFindTable->GetCollectFlag());
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"Collect message send failed(ret=%d\n",nRet);
                continue;
            }

            cFindTable->SetLastSendTime(tm);

            nLoopCnt++;
        }
    }


    return nLoopCnt;
}

CTableInfo* CPackageInfo::GetTable(string &a_strTableName)
{
    CTableInfo *cFindTable = NULL;
    list<CTableInfo*>::iterator iter;

    for(iter = m_lstTableInfo->begin(); iter != m_lstTableInfo->end(); iter++){
        cFindTable = *iter;
        if(cFindTable->CheckTableName(a_strTableName) == STM_OK){
            return cFindTable;
        }
    }

    return NULL;

}

string CPackageInfo::GetRcdInsertColumnStrList(string a_strTableName)
{
    CTableInfo *cTable = NULL;
    string strReturnValue; 

    strReturnValue.clear();

    cTable = GetTable(a_strTableName);
    if(cTable == NULL){
        STM_LOG(STA_ERR,"Table not exist(%s)\n",a_strTableName.c_str());
        return strReturnValue;
    }

    strReturnValue = cTable->GetRcdInsertColumnStrList();

    return strReturnValue;
}


int CPackageInfo::InsertQuery(string a_strQuery)
{
    int nRet = 0;
    nRet = m_cDb->Execute(a_strQuery.c_str(), a_strQuery.size());
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Insert Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    return STM_OK;
}

int CPackageInfo::CheckMarge(time_t curTm)
{
    time_t dailyStartTime = 0;
    time_t hourlyStartTime = 0;
    time_t dropTime = 0;
    time_t histDropTime = 0;
    time_t hourlyStopTime;
    time_t dailyStopTime;
    struct tm *curT = NULL;
    struct tm *lastHourT = NULL;
    CGlobal *cGlob = NULL;
    CThreadQueue<CMergeSession*> *cMergeSndQueue = NULL;
    list<CTableInfo*>::iterator iter;
    CMergeSession *cStsMergeSession = NULL;
    CMergeSession *cHistMergeSession = NULL;
    CMergeTableSession *cMergeTableSession = NULL;

    cGlob = CGlobal::GetInstance();

    if(m_blnCollectFlag == false){
        return STM_OK;
    }

    curT = localtime(&curTm);

    if((m_nHourlyCollectTime == 0) || 
            (m_nDailyCollectTime == 0)){
        STM_LOG(STM_ERR,"Marge failed Invalid collect time\n");
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
    histDropTime = ((curTm-(curTm%86400)) - ((m_nHistDropTime * 86400) + curT->tm_gmtoff)) ;

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

    DEBUG_LOG("HOUR  START TIME=%s",ctime(&hourlyStartTime));
    DEBUG_LOG("HOUR  STOP  TIME=%s\n",ctime(&hourlyStopTime));
    DEBUG_LOG("DAILY START TIME=%s",ctime(&dailyStartTime));
    DEBUG_LOG("DAILY STOP  TIME=%s\n",ctime(&dailyStopTime));
    DEBUG_LOG("DROP        TIME=%s\n",ctime(&dropTime));
    DEBUG_LOG("HIST DROP   TIME=%s\n",ctime(&histDropTime));


    for(iter = m_lstTableInfo->begin();iter != m_lstTableInfo->end();iter++){
        CTableInfo *cTable = NULL;
        list<string> *lstPrimaryKey = NULL;
        list<string> *lstValue = NULL;
        string strTableName;

        cTable = *iter;

        if(cTable->GetType()  == STM_TABLE_TYPE_STS){
            if(cStsMergeSession == NULL){
                /* make package merge session */
                cStsMergeSession = new CMergeSession(STM_TABLE_TYPE_STS, m_strPackageName, 
                        m_strDBName, m_strDBIp, m_nDBPort, m_strDBUser, m_strDBPassword);
            }

            strTableName = cTable->GetTableName();

            lstPrimaryKey = cTable->GetPrimaryKeyColumnList();
            lstValue = cTable->GetValueColumnList();

            cMergeTableSession = new CMergeTableSession();

            /* make session */
            cMergeTableSession->SetTime(hourlyStartTime, hourlyStopTime, dailyStartTime, dailyStopTime, dropTime);

            cMergeTableSession->SetTableName(strTableName);
            cMergeTableSession->SetPrimaryKeyColumn(lstPrimaryKey);
            cMergeTableSession->SetColumn(lstValue);

            cStsMergeSession->SetTableSession(cMergeTableSession);
        }
        else if(cTable->GetType()  == STM_TABLE_TYPE_HIST){
            if(cHistMergeSession == NULL){
                /* make package merge session */
                cHistMergeSession = new CMergeSession(STM_TABLE_TYPE_HIST, m_strPackageName, 
                        m_strDBName, m_strDBIp, m_nDBPort, m_strDBUser, m_strDBPassword);
            }

            strTableName = cTable->GetTableName();

            cMergeTableSession = new CMergeTableSession();

            /* make session */
            cMergeTableSession->SetTime(0, 0, 0, 0, histDropTime);

            cMergeTableSession->SetTableName(strTableName);
        }
    }/* end of for(iter = m_lstTableInfo->begin();iter != m_lstTableInfo->end();iter++) */

    cMergeSndQueue = cGlob->GetMergeSndQueueP();

    if(cStsMergeSession != NULL){
        cMergeSndQueue->Push(cStsMergeSession);
    }

    if(cHistMergeSession != NULL){
        cMergeSndQueue->Push(cHistMergeSession);
    }

    /* update last collect time */
    m_lastHourlyCollectTime = hourlyStopTime;
    m_lastDailyCollectTime = dailyStopTime;

    return STM_OK;
}

