#include <stdio.h>
#include <string.h>
#include <typeinfo>
#include "CGlobal.hpp"
#include "CMerge.hpp"

CMerge::CMerge(CThreadQueue<CMergeSession*> *a_cRcvQueue)
{
    m_cDb = new MariaDB();
    m_cRcvQueue = a_cRcvQueue;
}

CMerge::~CMerge()
{
    m_cRcvQueue = NULL;
    delete m_cDb;

}

string CMerge::GetUpdateColumnStr(list<string> &a_lstData)
{
    string strDataList;
    list<string>::iterator iter;

    if(a_lstData.size() == 0){
        return strDataList;
    }

    iter = a_lstData.begin();

    strDataList.append(*iter);
    strDataList.append("= VALUES(");
    strDataList.append(*iter);
    strDataList.append(")");

    iter++;

    for(;iter != a_lstData.end();iter++){
        strDataList.append(",");
        strDataList.append(*iter);
        strDataList.append("= VALUES(");
        strDataList.append(*iter);
        strDataList.append(")");
    }

    return strDataList;
}

string CMerge::GetColumnStr(list<string> &a_lstPrimaryKey)
{
    string strPrimaryKeyList;
    list<string>::iterator iter;

    if(a_lstPrimaryKey.size() == 0){
        return strPrimaryKeyList;
    }

    iter = a_lstPrimaryKey.begin();

    strPrimaryKeyList.append(*iter);

    iter++;

    for(;iter != a_lstPrimaryKey.end();iter++){
        strPrimaryKeyList.append(",");
        strPrimaryKeyList.append(*iter);
    }

    return strPrimaryKeyList;
}

string CMerge::GetSelectValueColumnStr(list<string> &a_lstValue)
{
    string strValueList;
    list<string>::iterator iter;

    if(a_lstValue.size() == 0){
        return strValueList;
    }

    iter = a_lstValue.begin();

    strValueList.append("SUM(");
    strValueList.append(*iter);
    strValueList.append(")");

    iter++;

    for(;iter != a_lstValue.end();iter++){
        strValueList.append(", SUM(");
        strValueList.append(*iter);
        strValueList.append(")");

    }

    return strValueList;
}

int CMerge::ConvertTimestamp(time_t a_curTm, string &a_strTime)
{
    int nTimeLen = 0;
    char szTime[62];
    struct tm *t = NULL;

    t = localtime(&a_curTm);

    nTimeLen = strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S",t);
    szTime[nTimeLen] = '\0';

    a_strTime = szTime;

    return STM_OK;
}

int CMerge::DropProcess(time_t a_dropTime, string &a_strTableName)
{
    int nRet = 0;
    char szQuery[1024];
    int nQueryLen = 0;

    nQueryLen = snprintf(szQuery,1024, "DELETE FROM %s WHERE PRC_DATE <= FROM_UNIXTIME(%lu)",
            a_strTableName.c_str(), a_dropTime);

    szQuery[nQueryLen] = '\0';

    nRet = m_cDb->Execute(szQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"(DROP PROCESS) DB Query Failed(tableName=%s, nRet=%d, err=%s)\n",
                a_strTableName.c_str(), nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    m_cDb->Commit();
    return STM_OK;

}

int CMerge::AlmQueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval)
{
    int nRet = 0;
    char szQuery[8192];
    int nQueryLen = 0;
    time_t startTime = 0;
    time_t endTime = 0;
    int nLoopCnt = 0;

    startTime = a_startTime;

    while(1){
        endTime = startTime + a_nInterval;

        /* send query */
        nQueryLen = snprintf(szQuery, 8192, 
                "INSERT INTO TAT_STS_ALM (PRC_DATE, NODE_NO, DST_YN, INDETERMINATE, CRITICAL, MAJOR, MINOR, WARNING, CLEARED, FAULT, NOTICE) (SELECT FROM_UNIXTIME(%lu), NODE_NO, DST_YN, SUM(INDETERMINATE), SUM(CRITICAL), SUM(MAJOR), SUM(MINOR), SUM(WARNING), SUM(CLEARED), SUM(FAULT), SUM(NOTICE) FROM TAT_STS_ALM WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE <= FROM_UNIXTIME(%lu) GROUP BY NODE_NO, DST_YN) ON DUPLICATE KEY UPDATE PRC_DATE = FROM_UNIXTIME(%lu), NODE_NO = VALUES(NODE_NO), INDETERMINATE = VALUES(INDETERMINATE), CRITICAL =  VALUES(CRITICAL), MAJOR =  VALUES(MAJOR), MINOR =  VALUES(MINOR), WARNING = VALUES(WARNING), CLEARED = VALUES(CLEARED), FAULT = VALUES(FAULT), NOTICE = VALUES(NOTICE)",
                endTime,
                startTime,
                endTime,
                endTime);

        szQuery[nQueryLen] = '\0';

        //printf("%s\n",szQuery);

        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE QUERY PROCESS) DB Query Failed(tableName=TAT_STS_ALM, nRet=%d, err=%s)\n",
                    nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        nQueryLen = snprintf(szQuery,8192, "DELETE FROM TAT_STS_ALM WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE < FROM_UNIXTIME(%lu)",
                startTime, endTime);
        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE AND DELETE QUERY PROCESS) DB Query Failed(tableName=TAT_STS_ALM, nRet=%d, err=%s)\n",
                    nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        szQuery[nQueryLen] = '\0';

        startTime = endTime;
        if(nLoopCnt == 200){
            m_cDb->Commit();
            nLoopCnt = 0;
        }
        else {
            nLoopCnt++;
        }

        if(endTime >= a_endTime){
            break;
        }
    }

    if(nLoopCnt != 0){
        m_cDb->Commit();
    }

    return STM_OK;
}

int CMerge::RscQueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval)
{
    int nRet = 0;
    char szQuery[8192];
    int nQueryLen = 0;
    time_t startTime = 0;
    time_t endTime = 0;
    int nLoopCnt = 0;

    startTime = a_startTime;

    while(1){
        endTime = startTime + a_nInterval;

        /* send query */
        nQueryLen = snprintf(szQuery, 8192, "INSERT INTO TAT_STS_RSC (RSC_ID, PRC_DATE, NODE_NO, COLUMN_ORDER_CCD, STAT_DATA, DST_FLAG) ( SELECT RSC_ID, FROM_UNIXTIME(%lu), NODE_NO, COLUMN_ORDER_CCD, AVG(STAT_DATA), DST_FLAG FROM TAT_STS_RSC WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE <= FROM_UNIXTIME(%lu) GROUP BY RSC_ID, NODE_NO, COLUMN_ORDER_CCD, DST_FLAG) ON DUPLICATE KEY UPDATE PRC_DATE = FROM_UNIXTIME(%lu), RSC_ID = VALUES(RSC_ID), NODE_NO = VALUES(NODE_NO), COLUMN_ORDER_CCD = VALUES(COLUMN_ORDER_CCD), STAT_DATA = VALUES(STAT_DATA), DST_FLAG = VALUES(DST_FLAG) ",
                endTime,
                startTime,
                endTime,
                endTime);

        szQuery[nQueryLen] = '\0';

        //printf("%s\n",szQuery);

        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE QUERY PROCESS) DB Query Failed(tableName=TAT_STS_RSC, nRet=%d, err=%s)\n",
                    nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        nQueryLen = snprintf(szQuery,8192, "DELETE FROM TAT_STS_RSC WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE < FROM_UNIXTIME(%lu)",
                startTime, endTime);
        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE AND DELETE QUERY PROCESS) DB Query Failed(tableName=TAT_STS_RSC, nRet=%d, err=%s)\n",
                    nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        szQuery[nQueryLen] = '\0';

        startTime = endTime;
        if(nLoopCnt == 200){
            m_cDb->Commit();
            nLoopCnt = 0;
        }
        else {
            nLoopCnt++;
        }

        if(endTime >= a_endTime){
            break;
        }
    }

    if(nLoopCnt != 0){
        m_cDb->Commit();
    }

    return STM_OK;
}

int CMerge::QueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval,
				string &a_strTableName,
				string &a_strPrimaryKeyList, string &a_strValueList,
				string &a_strSelectValueList, string &a_strUpdateValueList,
				string &a_strUpdatePrimaryKeyList)
{
    int nRet = 0;
    char szQuery[8192];
    int nQueryLen = 0;
    time_t startTime = 0;
    time_t endTime = 0;
    bool primaryKeyFlag = false;
    int nLoopCnt = 0;

    if(a_strPrimaryKeyList.size() == 0){
        primaryKeyFlag = false;
    }
    else {
        primaryKeyFlag = true;
    }

    startTime = a_startTime;
    while(1){
        endTime = startTime + a_nInterval;

        /* send query */
        nQueryLen = snprintf(szQuery, 8192, "INSERT INTO %s(PRC_DATE, DST_FLAG, NODE_NAME, %s %c %s) (SELECT FROM_UNIXTIME(%lu), DST_FLAG, NODE_NAME, %s %c %s FROM %s WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE <= FROM_UNIXTIME(%lu) GROUP BY NODE_NAME, DST_FLAG %c %s ) ON DUPLICATE KEY UPDATE PRC_DATE = FROM_UNIXTIME(%lu), DST_FLAG = VALUES(DST_FLAG), NODE_NAME = VALUES(NODE_NAME), %s %c %s",
                a_strTableName.c_str(),
                a_strPrimaryKeyList.c_str(), 
                primaryKeyFlag?',':' ',
                a_strValueList.c_str(), 
                endTime,
                a_strPrimaryKeyList.c_str(),
                primaryKeyFlag?',':' ',
                a_strSelectValueList.c_str(),
                a_strTableName.c_str(),
                startTime,
                endTime,
                primaryKeyFlag?',':' ',
                a_strPrimaryKeyList.c_str(),
                endTime,
                a_strUpdatePrimaryKeyList.c_str(),
                primaryKeyFlag?',':' ',
                a_strUpdateValueList.c_str());

        szQuery[nQueryLen] = '\0';

        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE QUERY PROCESS) DB Query Failed(tableName=%s, nRet=%d, err=%s)\n",
                    a_strTableName.c_str(), nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        nQueryLen = snprintf(szQuery,8192, "DELETE FROM %s WHERE PRC_DATE > FROM_UNIXTIME(%lu) AND PRC_DATE < FROM_UNIXTIME(%lu)",
                a_strTableName.c_str(),
                startTime, endTime);
        nRet = m_cDb->Execute(szQuery, nQueryLen);
        if(nRet < 0){
            STM_LOG(STM_ERR,"(MERGE AND DELETE QUERY PROCESS) DB Query Failed(tableName=%s, nRet=%d, err=%s)\n",
                    a_strTableName.c_str(), nRet, m_cDb->GetErrorMsg(nRet));
            return STM_NOK;
        }

        szQuery[nQueryLen] = '\0';

        startTime = endTime;
        if(nLoopCnt == 200){
            m_cDb->Commit();
            nLoopCnt = 0;
        }
        else {
            nLoopCnt++;
        }

        if(endTime >= a_endTime){
            break;
        }
    }

    if(nLoopCnt != 0){
        m_cDb->Commit();
    }


    return STM_OK;
}

int CMerge::TableProcess(int a_nType, CMergeTableSession *a_cTableSession)
{
    string strTableName;
    string strEndTime;
    string strStartTime;
    string strPrimaryKeyList;
    string strValueList;
    string strSelectValueList;
    string strUpdateValueList;
    string strUpdatePrimaryKeyList;
    list<string> *lstValue = NULL;
    list<string> *lstPrimaryKey = NULL; 
    time_t hourStartTime = 0;
    time_t hourEndTime = 0;
    time_t dailyStartTime = 0;
    time_t dailyEndTime = 0;
    time_t dropTime = 0;

    lstPrimaryKey = a_cTableSession->GetPrimaryKeyColumnP();
    lstValue =  a_cTableSession->GetColumnP();

    a_cTableSession->GetTime(hourStartTime, hourEndTime, dailyStartTime, dailyEndTime, dropTime);

    strTableName = a_cTableSession->GetTableName();


    if(a_nType == STM_TABLE_TYPE_STS){
        DropProcess(dropTime, strTableName);

        if(lstPrimaryKey != NULL){
            strPrimaryKeyList = GetColumnStr(*lstPrimaryKey);
            strUpdatePrimaryKeyList = GetUpdateColumnStr(*lstPrimaryKey);
        }

        strValueList = GetColumnStr(*lstValue);
        strSelectValueList = GetSelectValueColumnStr(*lstValue);

        strUpdateValueList = GetUpdateColumnStr(*lstValue);

        DEBUG_LOG("TABLE = %s", strTableName.c_str());
        DEBUG_LOG("TIME HOR=%lu, DAY=%lu, DRP=%lu\n", hourStartTime, dailyStartTime, dropTime);
        DEBUG_LOG("DROP START=%s", ctime(&dropTime));
        DEBUG_LOG("STS DAILY START=%s", ctime(&dailyStartTime));
        QueryProcess(dailyStartTime, dailyEndTime, 86400, strTableName, strPrimaryKeyList, strValueList,
                strSelectValueList, strUpdateValueList, strUpdatePrimaryKeyList);

        DEBUG_LOG("STS HOUR START=%s", ctime(&hourStartTime));
        QueryProcess(hourStartTime, hourEndTime, 3600, strTableName, strPrimaryKeyList, strValueList,
                strSelectValueList, strUpdateValueList, strUpdatePrimaryKeyList);
    }
    if(a_nType == STM_TABLE_TYPE_HIST){
        DEBUG_LOG("TABLE = %s", strTableName.c_str());
        DropProcess(dropTime, strTableName);
    }
    else if(a_nType == STM_TABLE_TYPE_RES){
        strTableName = "TAT_STS_RSC";
        DropProcess(dropTime, strTableName);
        DEBUG_LOG("RES DAILY START=%s", ctime(&dailyStartTime));
        RscQueryProcess(dailyStartTime, dailyEndTime, 86400);

        DEBUG_LOG("RES HOUR START=%s", ctime(&hourStartTime));
        RscQueryProcess(hourStartTime, hourEndTime, 3600);
    }
    else if(a_nType == STM_TABLE_TYPE_ALM){
        strTableName = "TAT_STS_ALM";
        DropProcess(dropTime, strTableName);
        DEBUG_LOG("ALM DAILY START=%s", ctime(&dailyStartTime));
        AlmQueryProcess(dailyStartTime, dailyEndTime, 86400);

        DEBUG_LOG("ALM HOUR START=%s", ctime(&hourStartTime));
        AlmQueryProcess(hourStartTime, hourEndTime, 3600);
    }

    delete a_cTableSession;

    return STM_OK;
}

int CMerge::Run()
{
    int nRet = 0;
    bool blnRet = true;
    string strDBName;
    string strDBIp;
    int nDBPort;
    string strDBUser;
    string strDBPassword;
    string strPackageName;
    CMergeSession *cSession = NULL;
    CMergeTableSession *cMergeTableSession = NULL;
    /* test time */
    time_t stTm =0 , endTm= 0;

    while(1){
        nRet = m_cRcvQueue->PopWait(&cSession);
        if(nRet != CThreadQueue<CMergeSession*>::CTHRD_Q_OK){
            STM_LOG(STM_ERR,"Queue pop failed(nRet=%d)\n",nRet);
            continue;
        }

        DEBUG_LOG("MERGE START\n");

        cSession->GetDBInfo(strDBName, strDBIp, nDBPort, strDBUser, strDBPassword, strPackageName);

        blnRet = m_cDb->Connect(strDBIp.c_str(), nDBPort, strDBUser.c_str(), strDBPassword.c_str(), strDBName.c_str());
        if(blnRet == false){
            STM_LOG(STM_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n",
                    strDBIp.c_str(), nDBPort, strDBUser.c_str(), strDBPassword.c_str(), strDBName.c_str());
            return STM_NOK;
        }

        m_cDb->SetAutoCommit(false);

        stTm = time(NULL);

        while(1){
            cMergeTableSession = cSession->GetFirstTableSession();
            if(cMergeTableSession == NULL){
                break;
            }
            nRet = TableProcess(cSession->GetType(), cMergeTableSession);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"Table process failed(%d)\n",nRet);
            }
        }

        endTm = time(NULL);

        DEBUG_LOG("MERGE END : PROC COUNT(%lu)\n", endTm - stTm);

        delete cSession;

        m_cDb->Close();
    }

    return STM_OK;
}
