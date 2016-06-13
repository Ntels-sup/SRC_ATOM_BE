#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MariaDB.hpp"
#include "FetchMaria.hpp"
#include "CGlobal.hpp"
#include "CStmConfig.hpp"


CStmConfig::CStmConfig(DB *a_cDb, int &nRet)
{
    if(a_cDb == NULL){
        STM_LOG(STM_ERR,"DB infomation not exist\n");
        nRet = STM_NOK;
        return;
    }

    m_cDb = a_cDb;
    nRet = STM_OK;
}

CStmConfig::~CStmConfig()
{

}

int CStmConfig::DBLoadAlarmMergeTime(CAlarmInfo *a_cAlarm)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    FetchMaria cData;	
    char szDailyCollectTime[BIG_STRING_NUMBER_LEN];
    char szHourlyCollectTime[BIG_STRING_NUMBER_LEN];
    char szDropTime[BIG_STRING_NUMBER_LEN];
    char szCheckTime[BIG_STRING_NUMBER_LEN];

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT DAILY_COLLECT_TIME, HOURLY_COLLECT_TIME, DROP_TIME, CHECK_TIME\
            FROM TAT_STS_PKG_CONFIG WHERE PKG_NAME = \"ATOM\"");

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szDailyCollectTime, sizeof(szDailyCollectTime));
    cData.Set(szHourlyCollectTime, sizeof(szHourlyCollectTime));
    cData.Set(szDropTime, sizeof(szDropTime));
    cData.Set(szCheckTime, sizeof(szCheckTime));

    if(cData.Fetch() == false){
        return STM_OK;
    }

    nRet = a_cAlarm->SetCollectTime(atol(szHourlyCollectTime), atol(szDailyCollectTime), 
            atoi(szCheckTime), atoi(szDropTime));
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource config init failed\n");
        return STM_NOK;
    }

    return STM_OK;
}



int CStmConfig::DBLoadResourceMergeTime(CResourceInfo *a_cResource)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    FetchMaria cData;	
    char szDailyCollectTime[BIG_STRING_NUMBER_LEN];
    char szHourlyCollectTime[BIG_STRING_NUMBER_LEN];
    char szDropTime[BIG_STRING_NUMBER_LEN];
    char szCheckTime[BIG_STRING_NUMBER_LEN];

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT DAILY_COLLECT_TIME, HOURLY_COLLECT_TIME, DROP_TIME, CHECK_TIME\
            FROM TAT_STS_PKG_CONFIG WHERE PKG_NAME = \"ATOM\"");

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szDailyCollectTime, sizeof(szDailyCollectTime));
    cData.Set(szHourlyCollectTime, sizeof(szHourlyCollectTime));
    cData.Set(szDropTime, sizeof(szDropTime));
    cData.Set(szCheckTime, sizeof(szCheckTime));

    if(cData.Fetch() == false){
        return STM_OK;
    }

    nRet = a_cResource->SetCollectTime(atol(szHourlyCollectTime), atol(szDailyCollectTime), 
            atoi(szCheckTime), atoi(szDropTime));
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource config init failed\n");
        return STM_NOK;
    }

    return STM_OK;
}


int CStmConfig::DBLoadResourceInfo(CResourceInfo *a_cResInfo)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    FetchMaria cData;
    bool blnStatYn = true;
    char szRscGrpId[RSC_GRP_ID_LEN];
    char szRscGrpName[RSC_GRP_ID_NAME_LEN];
    char szStatYn[2];
    char szStatPeriod[STRING_NUMBER_LEN];


    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT RSC_GRP_ID, RSC_GRP_NAME, STAT_YN, STAT_PERIOD\
            FROM TAT_RSC_GRP_DEF");
    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szRscGrpId, sizeof(szRscGrpId));
    cData.Set(szRscGrpName, sizeof(szRscGrpName));
    cData.Set(szStatYn, sizeof(szStatYn));
    cData.Set(szStatPeriod, sizeof(szStatPeriod));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        if(cData.Get(2)[0] == 'Y' || cData.Get(2)[0] == 'y'){
            blnStatYn = true;
        }
        else {
            blnStatYn = false;
        }

        nRet= a_cResInfo->InsertRsc(cData.Get(0), cData.Get(1), blnStatYn, atoi(cData.Get(3)));
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Column insert failed(ret=%d, RSC_GRP_ID=%s, RSC_GRP_NAME=%s)\n", nRet, cData.Get(0), 
                    cData.Get(1));
            continue;
        }
    }

    return STM_OK;
}

int CStmConfig::DBLoadColumn(string a_strPkgName, CTableInfo *a_cTable)
{
    int nRet = 0;
    int blnPKFlag = true;
    int nQueryLen = 0;
    char chQuery[1024];
    char chSequence[STRING_NUMBER_LEN];
    string strTableName;
    char chColumnName[COLUMN_NAME_LEN];
    char chPriKeyYn[2];
    char chDataType[DATA_TYPE_LEN];
    FetchMaria cData;

    strTableName = a_cTable->GetTableName();

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT COLUMN_NAME, SEQUENCE, PRI_KEY_YN, DATA_TYPE\
            FROM TAT_STS_COLUMN_INFO\
            WHERE TABLE_NAME=\"%s\" AND PKG_NAME=\"%s\"",strTableName.c_str(), a_strPkgName.c_str());
    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chColumnName, sizeof(chColumnName));
    cData.Set(chSequence, sizeof(chSequence));
    cData.Set(chPriKeyYn, sizeof(chPriKeyYn));
    cData.Set(chDataType, sizeof(chDataType));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        if(cData.Get(2)[0] == 'Y' || cData.Get(2)[0] == 'y'){
            blnPKFlag = true;
        }
        else {
            blnPKFlag = false;
        }

        nRet= a_cTable->InsertColumn(cData.Get(0), atoi(chSequence), cData.Get(3), blnPKFlag);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Column insert failed(ret=%d, COLUMN_NAME=%s, TABLE_NAME=%s)\n", nRet, cData.Get(0), 
                    strTableName.c_str());
            continue;
        }
    }

    return STM_OK;
}

int CStmConfig::DBLoadTable(CPackageInfo *a_cPackage)
{
    int nRet = 0;
    int nType = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    bool collectFlag = true;
    char chTableName[TABLE_NAME_LEN];
    char chType[TABLE_TYPE_LEN];
    char chCollectYn[YN_FLAG_LEN];
    char chCollectPeriod[STRING_NUMBER_LEN];
    string strPackageName;
    FetchMaria cData;

    CTableInfo *cTable= NULL;

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT TABLE_NAME, TYPE, COLLECT_YN, COLLECT_PERIOD\
            FROM TAT_STS_TABLE_INFO\
            WHERE PKG_NAME=\"%s\"",a_cPackage->GetPackageName().c_str());

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chTableName, sizeof(chTableName));
    cData.Set(chType, sizeof(chType));
    cData.Set(chCollectYn, sizeof(chCollectYn));
    cData.Set(chCollectPeriod, sizeof(chCollectPeriod));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        /* Make Node */
        cTable = new CTableInfo();

        if(chCollectYn[0] == 'Y' || chCollectYn[0] == 'y'){
            collectFlag = true;
        }
        else {
            collectFlag = false;
        }

        if(strcasecmp(chType,"STS") == 0){
            nType = STM_TABLE_TYPE_STS;
        }
        else if(strcasecmp(chType,"HIST") == 0){
            nType = STM_TABLE_TYPE_HIST;
        }
        else{
            STM_LOG(STM_ERR,"Invalid table type(table=%s, type=%s)\n",chTableName, chType);
            delete cTable;
            return STM_NOK;
        }

        nRet = cTable->Init(chTableName, atoi(chCollectPeriod), nType, collectFlag);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Table init failed(TABLE_NAME=%s)\n",chTableName);
            delete cTable;
            continue;
        }

        nRet= a_cPackage->InsertTable(cTable);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Table insert failed(ret=%d, TABLE_NAME=%s)\n", nRet, chTableName);
            delete cTable;
            continue;
        }

        strPackageName = a_cPackage->GetPackageName();
        nRet = DBLoadColumn(strPackageName, cTable);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Column Load failed(ret=%d)\n",nRet);
            continue;
        }
    }

    return STM_OK;
}

int CStmConfig::DBLoadPackageStsConfig(CPackageInfo *cPackage)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    FetchMaria cData;	
    char szDailyCollectTime[BIG_STRING_NUMBER_LEN];
    char szHourlyCollectTime[BIG_STRING_NUMBER_LEN];
    char szDropTime[BIG_STRING_NUMBER_LEN];
    char szCheckTime[BIG_STRING_NUMBER_LEN];
    char szHistDropTime[BIG_STRING_NUMBER_LEN];

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT DAILY_COLLECT_TIME, HOURLY_COLLECT_TIME, DROP_TIME, CHECK_TIME, HIST_DROP_TIME\
            FROM TAT_STS_PKG_CONFIG WHERE PKG_NAME = \"%s\"", cPackage->GetPackageName().c_str());

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szDailyCollectTime, sizeof(szDailyCollectTime));
    cData.Set(szHourlyCollectTime, sizeof(szHourlyCollectTime));
    cData.Set(szDropTime, sizeof(szDropTime));
    cData.Set(szCheckTime, sizeof(szCheckTime));
    cData.Set(szHistDropTime, sizeof(szHistDropTime));

    if(cData.Fetch() == false){
        return STM_OK;
    }

    nRet = cPackage->InitConfig(atol(szDailyCollectTime), 0, atol(szHourlyCollectTime), 0,
            atoi(szDropTime), atoi(szCheckTime), atoi(szHistDropTime));
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Package config init failed\n");
        return STM_NOK;
    }

    return STM_OK;
}

int CStmConfig::DBLoadPackage(list<CPackageInfo*> *a_lstPackageInfo)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    CPackageInfo *cPackage = NULL;
    FetchMaria cData;	
    char chPkgName[PACKAGE_NAME_LEN];
    char chDbName[DB_NAME_LEN];
    char chDbIp[DB_IP_LEN];
    char chDbPort[STRING_NUMBER_LEN];
    char chDbUser[DB_USER_LEN];
    char chDbPassword[DB_PASSWORD_LEN];

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT PKG_NAME, DB_NAME, DB_IP, DB_PORT, DB_USER, DB_PASSWORD\
            FROM TAT_PKG WHERE PKG_NAME != \"ATOM\"");

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chPkgName, sizeof(chPkgName));
    cData.Set(chDbName, sizeof(chDbName));
    cData.Set(chDbIp, sizeof(chDbIp));
    cData.Set(chDbPort, sizeof(chDbPort));
    cData.Set(chDbUser, sizeof(chDbUser));
    cData.Set(chDbPassword, sizeof(chDbPassword));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        /* Make Node */
        cPackage = new CPackageInfo();

        nRet = cPackage->Init(chPkgName, chDbName, chDbIp, atoi(chDbPort), chDbUser, chDbPassword);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Package init failed(PKG_NAME=%s, DB_NAME=%s)\n",chPkgName, chDbName);
            delete cPackage;
            return STM_NOK;
        }

        nRet = DBLoadPackageStsConfig(cPackage);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Pacakge config init failed(PKG_NAME=%s, DB_NAME=%s)\n",chPkgName, chDbName);
        }

        DBLoadTable(cPackage);

        a_lstPackageInfo->push_back(cPackage);

    }

    return STM_OK;
}

int CStmConfig::DBLoadNode(list<CNodeInfo*> *a_lstNodeInfo, list<CPackageInfo*> *a_cPackage)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    char chNodeNo[STRING_NUMBER_LEN];
    char chNodeType[NODE_TYPE_LEN];
    char chNodeName[NODE_NAME_LEN];
    unsigned int nPkgNameLen = 0;
    char chPkgName[PACKAGE_NAME_LEN];
    CPackageInfo *cFindPackage = NULL;
    list<CPackageInfo*>::iterator lstPkgIter;
    string strPackageName;
    FetchMaria cData;
    CNodeInfo *cNode = NULL;

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT NODE_NO, NODE_TYPE, NODE_NAME, PKG_NAME FROM TAT_NODE WHERE PKG_NAME != \"ATOM\"");
    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chNodeNo, sizeof(chNodeNo));
    cData.Set(chNodeType, sizeof(chNodeType));
    cData.Set(chNodeName, sizeof(chNodeName));
    cData.Set(chPkgName, sizeof(chPkgName));

    while(1){

        if(cData.Fetch() == false){
            break;
        }

        cNode = new CNodeInfo();

        nRet = cNode->Init(atoi(chNodeNo), chNodeType, chNodeName, 0);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Node init failed(name=%s, nodeId=%d)\n",chNodeName, atoi(chNodeNo));
            delete cNode;
            return STM_NOK;
        }

        nRet = DBLoadTCAInfo(cNode);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"TCA load failed(ret=%d)\n",nRet);
        }

        a_lstNodeInfo->push_back(cNode);

        /* find Package */
        nPkgNameLen = strlen(chPkgName);
        strPackageName.clear();
        for(lstPkgIter = a_cPackage->begin(); lstPkgIter != a_cPackage->end(); lstPkgIter++){
            cFindPackage = *lstPkgIter;
            strPackageName = cFindPackage->GetPackageName();
            if(strPackageName.size() != nPkgNameLen){
                strPackageName.clear();
                continue;
            }
            if(strncasecmp(strPackageName.c_str(), chPkgName, nPkgNameLen) == 0){
                cFindPackage->InsertNode(cNode);
                break;
            }
            strPackageName.clear();
        }

        if(lstPkgIter == a_cPackage->end()){
            STM_LOG(STM_ERR,"Package not exsit(pkgName=%s, nodeName=%s)\n", chPkgName, chNodeName);
            continue;
        }
    }

    return STM_OK;
}

int CStmConfig::DBLoadTCARule(int nNodeNo, CTCAInfo *cTCAInfo)
{
    string strTableName;
    string strColumnName;
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    CTCARule *cTCARule = NULL;
    FetchMaria cData;
    int nSeventy = 0;
    char chSeventy[SEVENTY_LEN];
    char chLeftSign[SIGN_LEN];
    char chLeftVal[STRING_NUMBER_LEN];
    char chRightSign[SIGN_LEN];
    char chRightVal[STRING_NUMBER_LEN];

    strTableName = cTCAInfo->GetTableName();

    strColumnName = cTCAInfo->GetColumnName();

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT SEVERITY_CCD, LEFT_SIGN, LEFT_VAL, RIGHT_SIGN, RIGHT_VAL \
            FROM TAT_STS_TCA_RULE \
            WHERE PKG_NAME=\"%s\" AND NODE_NO=%d AND TABLE_NAME=\"%s\" AND COLUMN_NAME=\"%s\"", 
            cTCAInfo->GetPackageName().c_str(),
            nNodeNo, 
            strTableName.c_str(),
            strColumnName.c_str());

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chSeventy, sizeof(chSeventy));
    cData.Set(chLeftSign, sizeof(chLeftSign));
    cData.Set(chLeftVal, sizeof(chLeftVal));
    cData.Set(chRightSign, sizeof(chRightSign));
    cData.Set(chRightVal, sizeof(chRightVal));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        if(strcasecmp(chSeventy,"OVER CRITICAL") == 0){
            nSeventy = TCA_RULE_OVER_CRITICAL;
        }
        else if(strcasecmp(chSeventy,"OVER MAJOR") == 0){
            nSeventy = TCA_RULE_OVER_MAJOR; 
        }
        else if(strcasecmp(chSeventy,"OVER MINOR") == 0){
            nSeventy = TCA_RULE_OVER_MINOR; 
        }
        else if(strcasecmp(chSeventy,"CLEARED") == 0){
            nSeventy = TCA_RULE_CLEARED; 
        }
        else if(strcasecmp(chSeventy,"UNDER CRITICAL") == 0){
            nSeventy = TCA_RULE_UNDER_CRITICAL; 
        }
        else if(strcasecmp(chSeventy,"UNDER MAJOR") == 0){
            nSeventy = TCA_RULE_UNDER_MAJOR; 
        }
        else if(strcasecmp(chSeventy,"UNDER MINOR") == 0){
            nSeventy = TCA_RULE_UNDER_MINOR; 
        }
        else {
            STM_LOG(STM_ERR,"Invalid Seventy(%s)\n",chSeventy);
            continue;
        }

        cTCARule = new CTCARule();

        nRet = cTCARule->Init(nSeventy , chLeftSign, atoi(chLeftVal), chRightSign, atoi(chRightVal));
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"TCA Rule Init failed(ret=%d)\n",nRet);
            delete cTCARule;
            continue;
        }

        nRet = cTCAInfo->InsertRule(cTCARule);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"TCA Rule insert failed(ret=%d)\n",nRet);
            delete cTCARule;
            continue;
        }
    }

    return STM_OK;
}

int CStmConfig::DBLoadTCAInfo(CNodeInfo *cNode)
{
    int nTableNameLen = 0;
    int nColumnNameLen = 0;
    int nTCANameLen = 0;
    bool actFlag = true;
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    CTCAInfo *cTCA = NULL;
    FetchMaria cData;
    string strPackageName;
    string strTableName;
    string strColumnName;
    string strTCAName;
    char chColumnName[COLUMN_NAME_LEN];
    char chTableName[TABLE_NAME_LEN];
    char chTCAName[TCA_NAME_LEN];
    char chActFlag[YN_FLAG_LEN];
    char chPkgName[PACKAGE_NAME_LEN];

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT COLUMN_NAME, TABLE_NAME, TCA_NAME, ACT_YN, PKG_NAME\
            FROM TAT_STS_TCA \
            WHERE NODE_NO=%d", cNode->GetNodeNo());

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(chColumnName, sizeof(chColumnName));
    cData.Set(chTableName, sizeof(chTableName));
    cData.Set(chTCAName, sizeof(chTCAName));
    cData.Set(chActFlag, sizeof(chActFlag));
    cData.Set(chPkgName, sizeof(chPkgName));

    while(1){
        if(cData.Fetch() == false){
            break;
        }

        /* Make Node */
        cTCA = new CTCAInfo();

        nTableNameLen = strlen(chTableName);
        nColumnNameLen = strlen(chColumnName);
        nTCANameLen = strlen(chTCAName);

        if((chActFlag[0] == 'N') || (chActFlag[0] == 'n')){
            actFlag = false;
        }
        else{
            actFlag = true;
        }

        strPackageName = chPkgName;

        strTableName = chTableName;
        strColumnName = chColumnName;
        strTCAName = chTCAName;

        nRet = cTCA->Init(strTableName, strColumnName, strTCAName,
                strPackageName, actFlag);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"TCA Info init failed(name=%.*s)\n",nTCANameLen, chTCAName);
            delete cTCA;
            continue;
        }

        nRet = DBLoadTCARule(cNode->GetNodeNo(), cTCA);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"TCA Rule load failed(ret=%d)\n",nRet);
        }
        /* insert TCA RULE */

        cNode->InsertTCAInfo(cTCA);
    }

    return STM_OK;
}

int CStmConfig::DBGetNodeId(char *a_szPkgName, char *a_szNodeType, char *a_szNodeName ,int *a_nNodeNo)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    char szNodeNo[STRING_NUMBER_LEN];
    FetchMaria cData;

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT NODE_NO\
            FROM TAT_NODE\
            WHERE PKG_NAME=\"%s\" AND NODE_TYPE=\"%s\" AND NODE_NAME=\"%s\"",
            a_szPkgName, a_szNodeType, a_szNodeName);

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szNodeNo, sizeof(szNodeNo));

    if(cData.Fetch() == false){
        return STM_NOK;
    }

    *a_nNodeNo= atoi(szNodeNo);

    return STM_OK;
}

int CStmConfig::DBGetProcNo(char *a_szPkgName, char *a_szNodeType, char *a_szProcName ,int *a_nProcNo)
{
    int nRet = 0;
    int nQueryLen = 0;
    char chQuery[1024];
    char szProcNo[STRING_NUMBER_LEN];
    FetchMaria cData;

    nQueryLen = snprintf(chQuery, sizeof(chQuery), 
            "SELECT PROC_NO\
            FROM TAT_PROCESS\
            WHERE PKG_NAME=\"%s\" AND NODE_TYPE=\"%s\" AND PROC_NAME=\"%s\"",
            a_szPkgName, a_szNodeType, a_szProcName);

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet < 0){
        STM_LOG(STM_ERR,"DB Query Failed(nRet=%d, err=%s)\n",nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    cData.Set(szProcNo, sizeof(szProcNo));

    if(cData.Fetch() == false){
        return STM_NOK;
    }

    *a_nProcNo = atoi(szProcNo);

    return STM_OK;
}

