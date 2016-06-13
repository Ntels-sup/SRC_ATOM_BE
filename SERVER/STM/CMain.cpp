#include <stdio.h>
#include <string.h>

#include "MariaDB.hpp"
#include "FetchMaria.hpp"

#include "CommandFormat.hpp"
#include "CGlobal.hpp"
#include "CThreadQueue.hpp"
#include "CMergeThread.hpp"
#include "CConfig.hpp"
#include "CStmConfig.hpp"
#include "CResTableData.hpp"
#include "CResourceInfo.hpp"
#include "CAlarmInfo.hpp"
#include "CMain.hpp"
#include "CCollectValue.hpp"

CMain::CMain(int &nRet)
{
    m_lstPackageInfo = new list<CPackageInfo*>;
    m_lstNodeInfo = new list<CNodeInfo*>;
    m_cDb = new MariaDB();
    m_cResInfo = new CResourceInfo();
    m_cAlarmInfo = new CAlarmInfo();
    m_cStmConfig = new CStmConfig(m_cDb, nRet);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Db config init failed(nRet=%d)\n",nRet);
        nRet = STM_NOK;
        return;
    }

    m_cLowerInter = NULL;
}

CMain::~CMain()
{
}

CPackageInfo *CMain::SearchPackage(const char *a_chPackageName, int a_nPackageNameLen)
{
    int nRet = 0;
    CPackageInfo *cFindPackage = NULL;
    list<CPackageInfo*>::iterator iter;

    for(iter = m_lstPackageInfo->begin(); iter != m_lstPackageInfo->end(); iter++){
        cFindPackage = *iter;
        nRet = cFindPackage->CheckName(a_chPackageName, a_nPackageNameLen);
        if(nRet == STM_OK){
            return cFindPackage;
        }
    }

    return NULL;
}

CNodeInfo *CMain::SearchNode(int a_nNodeNo)
{
    CNodeInfo *pcFindNode = NULL;
    list<CNodeInfo*>::iterator iter;

    /* processing */
    for(iter = m_lstNodeInfo->begin(); iter != m_lstNodeInfo->end(); iter++){
        pcFindNode = *iter;

        if(pcFindNode->GetNodeNo() == a_nNodeNo){
            return pcFindNode;
        }
    }

    return NULL;

}

int CMain::PackageRun()
{
    //int nRet = 0;
    int nLoopCnt = 0;
    CPackageInfo *cFindPackage = NULL;
    list<CPackageInfo*>::iterator iter;

    for(iter = m_lstPackageInfo->begin(); iter != m_lstPackageInfo->end(); iter++){
        cFindPackage = *iter;
        nLoopCnt += cFindPackage->Run();
    }

    return nLoopCnt;
}

CPackageInfo *CMain::SearchPackage(string &a_strPackageName)
{
    CPackageInfo *cFindPackage = NULL;
    list<CPackageInfo*>::iterator iter;

    for(iter = m_lstPackageInfo->begin(); iter != m_lstPackageInfo->end(); iter++){
        cFindPackage = *iter;
        if(cFindPackage->CheckName(a_strPackageName) == STM_OK){
            return cFindPackage;
        }
    }

    return NULL;

}

int CMain::TCACheck(CSessionInfo *pcFindSession, CNodeInfo *a_pcNode)
{
    int nRet = 0;
    string strPackageName;
    string strTableName;
    string strColumnName;
    CPackageInfo *cFindPackage = NULL;
    CTableInfo *cFindTable = NULL;
    list<CColumnInfo*> *lstColumnList = NULL;
    list<int> *lstValueList = NULL;
    list<CColumnInfo*>::iterator columnIter;
    list<int>::iterator valueIter;
    CColumnInfo *cFindColumn = NULL;
    int nValue = 0;
    list<CCollectValue*> *lstCollectValue = NULL;
    list<CCollectValue*>::iterator collectIter;

    strPackageName = pcFindSession->GetPackageName();

    cFindPackage = SearchPackage(strPackageName);
    if(cFindPackage == NULL){
        return STM_OK;
    }

    strTableName = pcFindSession->GetTableName();

    cFindTable = cFindPackage->GetTable(strTableName);
    if(cFindTable == NULL){
        return STM_OK;
    }

    lstColumnList = cFindTable->GetColumnLst();
    if(lstColumnList == NULL){
        STM_LOG(STM_ERR,"Column list not exit(tableName=%s)\n",strTableName.c_str());
        return STM_NOK;
    }

    lstCollectValue = pcFindSession->GetCollectValueP();
    if(lstCollectValue == NULL){
        STM_LOG(STM_ERR,"Collect value not exist(tableName=%s)\n",strTableName.c_str());
        return STM_NOK;
    }

    for(collectIter = lstCollectValue->begin(); collectIter != lstCollectValue->end();collectIter++){
        lstValueList = (*collectIter)->GetValueListP();

        for(columnIter = lstColumnList->begin(), valueIter = lstValueList->begin();
                (columnIter != lstColumnList->end()) && (valueIter != lstValueList->end());
                columnIter++, valueIter++ ){
            cFindColumn = *columnIter;
            nValue = *valueIter;

            strColumnName = cFindColumn->GetColumnName();
            nRet = a_pcNode->CheckTCA(strPackageName, strTableName, strColumnName, nValue);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"TCA Check failed(tableName=%s)\n", strTableName.c_str());
                return STM_NOK;
            }
        }
    }

    return STM_OK;
}

int CMain::UpdateColumnEnumIndex(string a_strTableName, string a_strPackageName, string a_strColumnName, string a_strEnumValue)
{
    int nRet = STM_OK;
    FetchMaria cData;
    int nQueryLen = 0;
    char chQuery[1024];

    nQueryLen = snprintf(chQuery,1024, "SELECT ENUM_VALUE FROM TAT_STS_COLUMN_ENUM_INFO\
            WHERE TABLE_NAME=\"%s\" AND PKG_NAME=\"%s\"\
            AND COLUMN_NAME=\"%s\" AND ENUM_VALUE=\"%s\"",
            a_strTableName.c_str(), a_strPackageName.c_str(), 
            a_strColumnName.c_str(), a_strEnumValue.c_str());

    nRet = m_cDb->Query(&cData, chQuery, nQueryLen);
    if(nRet > 0){
        return STM_OK;
    }
    else if(nRet < 0){
        STM_LOG(STM_ERR,"Colummn Enum index select query failed(nRet=%d, err=%s)\n",
                nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    nQueryLen = snprintf(chQuery,1024, "INSERT INTO TAT_STS_COLUMN_ENUM_INFO(TABLE_NAME, PKG_NAME, COLUMN_NAME, ENUM_VALUE)  VALUES(\"%s\", \"%s\", \"%s\", \"%s\")",
            a_strTableName.c_str(), a_strPackageName.c_str(), a_strColumnName.c_str(), a_strEnumValue.c_str());

    nRet = m_cDb->Execute(chQuery, nQueryLen);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"TAT_STS_COLUMN_ENUM_INFO INSERT FAILED(nRet=%d, err=(%s))\n", 
                nRet, m_cDb->GetErrorMsg(nRet));
        return STM_NOK;
    }

    return STM_OK;
}

int CMain::UpdateResStsValue(CSessionInfo *a_cSession, CNodeInfo *a_cNodeInfo)
{
    int nRet = STM_OK;
    int nQueryLen = 0;
    char chQuery[1024];
    time_t prcDate = 0;
    struct tm *t = NULL;
    list<CResGroup*> *lstResGroup = NULL;
    CResGroup *cResGroup = NULL;
    CResTableData *cTableData = NULL;

    lstResGroup = a_cSession->GetResGroupP();

    while(lstResGroup->size() != 0){
        cResGroup = lstResGroup->front();
        lstResGroup->pop_front();

        while(1){
            cTableData = cResGroup->GetFirstResTableData();
            if(cTableData == NULL){
                break;
            }
            prcDate = cTableData->GetPrcDate();

            t = localtime(&prcDate);

            nQueryLen = snprintf(chQuery,1024, "INSERT INTO TAT_STS_RSC(RSC_ID, PRC_DATE, DST_FLAG, NODE_NO, COLUMN_ORDER_CCD, STAT_DATA)  VALUES(\"%s\", FROM_UNIXTIME(%lu), '%c', %d, \"%s\", %lf)",
                    cResGroup->GetRscId().c_str(), 
                    prcDate,
                    t->tm_isdst==0?'N':'Y',
                    cTableData->GetNodeNo(),
                    cTableData->GetColumnOrder().c_str(),
                    cTableData->GetStatData());

            DEBUG_LOG("RESOURCE QUERY=%s\n",chQuery);

            delete cTableData;

            nRet = m_cDb->Execute(chQuery, nQueryLen);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"TAT_STS_RSC INSERT FAILED(nRet=%d, err=%s)\n", 
                        nRet, m_cDb->GetErrorMsg(nRet));
                break;
            }
        }
        delete cResGroup;

    }

    return STM_OK;
}

int CMain::UpdateStsValue(CSessionInfo *a_cSession, CNodeInfo *a_cNodeInfo)
{
    int nRet = 0;
    char szQueryHeader[1024];
    char szValue[512];
    string strPackageName;
    string strTableName;
    string strPrimaryKey;
    string strNodeName;
    CPackageInfo *cPackage = NULL;
    CTableInfo *cTable = NULL;
    CCollectValue *cCollectValue = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;
    list<CCollectValue*>::iterator collectIter;
    list<string> *lstPrimaryKey;
    list<int> *lstValue;
    list<string>::iterator primaryIter;
    list<int>::iterator valueIter;
    string strQuery;

    lstCollectValue = a_cSession->GetCollectValueP();
    if(lstCollectValue == NULL){
        STM_LOG(STM_ERR,"Collect value not exist\n");
        return STM_NOK;
    }

    strTableName = a_cSession->GetTableName();

    strNodeName = a_cNodeInfo->GetNodeName();
    strPackageName = a_cSession->GetPackageName();

    cPackage = SearchPackage(strPackageName);
    if(cPackage == NULL){
        STM_LOG(STM_ERR,"Package not found(name=%s)\n", strPackageName.c_str());
        return STM_NOK;
    }

    cTable = cPackage->GetTable(strTableName);
    if(cTable == NULL){
        STM_LOG(STM_ERR,"Table not found(pkgName=%s, tableName=%s)\n", strPackageName.c_str(), strTableName.c_str());
        return STM_NOK;
    }

    if(a_cSession->GetType() == STM_TABLE_TYPE_STS){
        snprintf(szQueryHeader,1024, "INSERT INTO %s VALUES(FROM_UNIXTIME(%lu), \"%s\", '%c'",
                strTableName.c_str(), a_cSession->GetLastTime(), strNodeName.c_str(), 
                a_cSession->GetDstFlag()?'Y':'N');
    }
    else if(a_cSession->GetType() == STM_TABLE_TYPE_HIST){
        string strColumnStrList;

        strColumnStrList = cPackage->GetRcdInsertColumnStrList(strTableName);
        if(strColumnStrList.size() == 0){
            STM_LOG(STM_ERR,"Column list make error(packageName=%s)\n", strPackageName.c_str());
            return STM_NOK;

        }
        snprintf(szQueryHeader,1024, "INSERT INTO %s(%s) VALUES(FROM_UNIXTIME(%lu), \"%s\", '%c'",
                strTableName.c_str(), strColumnStrList.c_str(), a_cSession->GetHistCollectTime(), strNodeName.c_str(), 
                a_cSession->GetDstFlag()?'Y':'N');

        DEBUG_LOG("HISTORY QUERY=%s\n", szQueryHeader);
    }

    strQuery.append(szQueryHeader);

    for(collectIter = lstCollectValue->begin(); collectIter != lstCollectValue->end();collectIter++){
        int i = 0;
        string strColumnName;
        cCollectValue = *collectIter;

        lstPrimaryKey = cCollectValue->GetPrimaryKeyListP();
        /* set primary key */
        for(i=1, primaryIter = lstPrimaryKey->begin(); primaryIter != lstPrimaryKey->end();primaryIter++, i++){
            strPrimaryKey = *primaryIter;

            strQuery.append(", \"");
            strQuery.append(strPrimaryKey);
            strQuery.append("\"");

            strColumnName.clear();
            strColumnName = cTable->GetColumnName(i);
            if(strColumnName.size() == 0){
                STM_LOG(STM_ERR,"Column not found(index=%d)\n",i);
                continue;
            }

            /* update Column Index */
            nRet = UpdateColumnEnumIndex(strTableName ,strPackageName, strColumnName, strPrimaryKey);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"Column index update faield(tableName=%s, columnName=%s)\n", 
                        strTableName.c_str(), strColumnName.c_str());
            }
        }

        lstValue = cCollectValue->GetValueListP();
        /* set values */
        for(valueIter = lstValue->begin();valueIter != lstValue->end();valueIter++){
            sprintf(szValue,"%d", *valueIter);

            strQuery.append(", ");
            strQuery.append(szValue);

        }

        strQuery.append(")");
        /* update */
        DEBUG_LOG("STATISTIC QUERY=%s\n",strQuery.c_str());

        cPackage->InsertQuery(strQuery);

		strQuery = szQueryHeader;
    }/* end of for(collectIter = lstCollectValue->begin(); collectIter != lstCollectValue->end();collectIter++) */

    return STM_OK;
}

int CMain::NodeSendHeartBeat()
{
    CNodeInfo *pcFindNode = NULL;
    list<CNodeInfo*>::iterator iter;

    /* processing */
    for(iter = m_lstNodeInfo->begin(); iter != m_lstNodeInfo->end(); iter++){
        pcFindNode = *iter;

        pcFindNode->SendHeartBeat();
    }

    return STM_OK;
}

int CMain::MargeHandler(time_t a_curTm)
{
    int nRet = 0;
    CPackageInfo *cFindPackage = NULL;
    list<CPackageInfo*>::iterator iter;

    /* Check Resource Merge */
    m_cResInfo->CheckMarge(a_curTm);

    m_cAlarmInfo->CheckMarge(a_curTm);

    for(iter = m_lstPackageInfo->begin(); iter != m_lstPackageInfo->end(); iter++){
        cFindPackage = *iter;
        nRet = cFindPackage->CheckMarge(a_curTm);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Marge check failed(nRet=%d)\n",nRet);
            continue;
        }


    }


    return STM_OK;
}

int CMain::ResourceHandler(time_t a_curTm, bool a_blnDstFlag)
{
    int nRet = STM_OK;
    list<CNodeInfo*>::iterator iter;
    list<string> *lstSendGrpId = NULL;
    CNodeInfo *pcFindNode = NULL;

    nRet = m_cResInfo->StsSendHandler(a_curTm);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource send handler faiiled(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    while(1){
        lstSendGrpId = m_cResInfo->GetFirstSendRscGrpId();
        if(lstSendGrpId == NULL){
            break;
        }

        /* processing */
        for(iter = m_lstNodeInfo->begin(); iter != m_lstNodeInfo->end(); iter++){
            pcFindNode = *iter;

            nRet = pcFindNode->MakeResSession(lstSendGrpId, a_curTm, a_blnDstFlag);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"Resource session make failed(nRet=%d)\n",nRet);
            }
        }
    }

    return STM_OK;
}

int CMain::ReceiveHandler()
{
    int nRet = STM_OK;
    int nLoopCnt = 0;
    int nNodeNo = 0;
    int nProcNo = 0;
    int nCmdCode = NULL;
    string strCmdCode; 
    CNodeInfo *cNode = NULL;
    CGlobal *cGlob = NULL;
    CLowerInterface *cLowerInter = NULL;
    CProtocol *cProto = NULL;
    string strPayload;

    cGlob = CGlobal::GetInstance();

    cLowerInter = cGlob->GetLowerInterfaceP();

    while(1){
        cProto = cLowerInter->Receive();
        if(cProto == NULL){
            return nLoopCnt;
        }

        nLoopCnt++;

        cProto->GetSource(nNodeNo, nProcNo);

        cNode = SearchNode(nNodeNo);
        if(cNode == NULL){
            STM_LOG(STM_ERR,"Node not exist(no=%d)\n",nNodeNo);
            delete cProto;
            return nLoopCnt;
        }

        /* update last rcv Time */
        cNode->SetLastRcvTm();

        if(cNode->GetState() != ACTIVE_STATUS){
            cNode->SetActiveState();

            /* resend all message */
            cNode->ResendAllPendingSession();
        }

        strPayload = cProto->GetPayload();

        cProto->GetCommand(strCmdCode);

        nCmdCode = CGlobal::GetCmdCode(strCmdCode);
        switch(nCmdCode){
            case CMD_STS_STA:
                {
                    CSessionInfo *cSession = NULL;

                    nRet = cNode->RecvStsHandler(strPayload, &cSession);
                    if(nRet != STM_OK){
                        if(nRet != STM_ERR_RESULT_RECEIVE){
                            STM_LOG(STM_ERR,"Message handling failed(nRet=%d)\n",nRet);
                            delete cProto;
                            return nLoopCnt;
                        }
                        else {
                            delete cProto;
                            return nLoopCnt;
                        }
                    }

                    nRet = TCACheck(cSession, cNode);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"TCA Check failed(nRet=%d)\n",nRet);
                    }

                    nRet = UpdateStsValue(cSession, cNode);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"Sts update failed(nRet=%d)\n",nRet);
                        delete cSession;
                        delete cProto;
                        return nLoopCnt;
                    }

                    delete cSession;
                }
                break;
            case CMD_STS_STA_HIST:
                {
                    CSessionInfo *cSession = NULL;

                    nRet = cNode->RecvHistHandler(strPayload, &cSession);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"Hist Message handling failed(nRet=%d)\n",nRet);
                        delete cProto;
                        return nLoopCnt;
                    }

                    nRet = UpdateStsValue(cSession, cNode);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"Sts update failed(nRet=%d)\n",nRet);
                        delete cSession;
                        delete cProto;
                        return nLoopCnt;
                    }

                    delete cSession;
                }
                break;
            case CMD_STS_RSA:
                {
                    CSessionInfo *cSession = NULL;

                    nRet = cNode->RecvResStsHandler(strPayload, &cSession);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"Message handling failed(nRet=%d)\n",nRet);
                        delete cProto;
                        return nLoopCnt;
                    }

                    nRet = UpdateResStsValue(cSession, cNode);
                    if(nRet != STM_OK){
                        STM_LOG(STM_ERR,"Sts update failed(nRet=%d)\n",nRet);
                        delete cSession;
                        delete cProto;
                        return nLoopCnt;
                    }

                    delete cSession;

                }
                break;
            case CMD_STS_STA_HEARTBEAT:
                {
                }
                break;
            default:
                STM_LOG(STM_ERR,"Unknown command code(%d)\n",nCmdCode);
                break;
        }

        delete cProto;
    }

    return nLoopCnt;
}

int CMain::Init()
{
    bool blnRet = false;
    int nRet = 0;
    int nProcNo = 0;
    int nNodeNo = 0;
    const char *szTmpConfVal[3];
    CFileLog *cLog = NULL;
    CGlobal *cGlob = NULL;
    CLowerInterface *cLowerInter = NULL;
    CConfig cStmLocalCfg;

    /* NM Connecton */
    cGlob = CGlobal::GetInstance();

    nRet = cStmLocalCfg.Initialize();
    if(nRet != 0){
        STM_LOG(STM_ERR,"Config init failed\n");
        return STM_NOK;
    }

    cLog = cGlob->GetLogP();

    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","LOG_PATH");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"LOG PATH CONFIG NOT EXIST\n");
        return STM_NOK;

    }

    nRet = cLog->Initialize(szTmpConfVal[0], NULL, (char*)"STM", 0, LV_DEBUG);
    if(nRet != 0){
        STM_LOG(STM_ERR,"Log init failed(nRet=%d)\n", nRet);
        return STM_NOK;
    }

    cLowerInter = cGlob->GetLowerInterfaceP();

    cLowerInter->SetLog(cLog);

    /* set nm ip */
    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","ATOM_SERVER_IP");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"ATOM_SERVER_IP CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    szTmpConfVal[1] = cStmLocalCfg.GetConfigValue("GLOBAL","ATOM_SERVER_PORT");
    if(szTmpConfVal[1] == NULL){
        STM_LOG(STM_ERR,"ATOM_SERVER_PORT CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    szTmpConfVal[2] = cStmLocalCfg.GetConfigValue("STM","CONN_RETRY_TIME");
    if(szTmpConfVal[2] == NULL){
        STM_LOG(STM_ERR,"CONN_RETRY_TIME CONFIG NOT EXIST\n");
        return STM_NOK;
    }

    cLowerInter->SetTransportAddr(szTmpConfVal[0],
            atoi(szTmpConfVal[1]),
            atoi(szTmpConfVal[2])
            );

    /* Get Db Connection */
    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","DB_DATABASE");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"DB_DATABASE CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    m_strDbName = szTmpConfVal[0];

    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","DB_HOST");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"DB_DATABASE CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    m_strDbIp = szTmpConfVal[0];

    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","DB_PORT");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"DB_PORT CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    m_nDbPort = atoi(szTmpConfVal[0]);

    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","DB_USER");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"DB_USER CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    m_strDbUser = szTmpConfVal[0];


    szTmpConfVal[0] = cStmLocalCfg.GetConfigValue("GLOBAL","DB_PASS");
    if(szTmpConfVal[0] == NULL){
        STM_LOG(STM_ERR,"DB_PASS CONFIG NOT EXIST\n");
        return STM_NOK;
    }
    m_strDbPasswd = szTmpConfVal[0];

    /* connect db */
    while(1){
        blnRet = m_cDb->Connect( m_strDbIp.c_str(), m_nDbPort, 
                m_strDbUser.c_str(), m_strDbPasswd.c_str(), m_strDbName.c_str());
        if(blnRet == false){
            STM_LOG(STM_ERR,"DB Connect failed(%s, %d, %s, %s, %s)\n", 
                    m_strDbIp.c_str(), m_nDbPort, m_strDbUser.c_str(),
                    m_strDbPasswd.c_str(), m_strDbName.c_str());
            sleep(1);
        }
        else{
            break;
        }
    }

    nRet = m_cStmConfig->DBGetNodeId((char*)"ATOM", (char*)"EMS", (char*)"ATOM", &nNodeNo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"ATOM NODE NO load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    cGlob->SetLocalNodeNo(nNodeNo);

    nRet = m_cStmConfig->DBGetProcNo((char*)"ATOM", (char*)"EMS", (char*)"NM", &nProcNo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"NM PROC NO load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }


    m_cLowerInter = cGlob->GetLowerInterfaceP();
    m_cLowerInter->SetNmProcNo(nProcNo);

    nRet = m_cStmConfig->DBGetProcNo((char*)"ATOM", (char*)"EMS", (char*)"STM", &nProcNo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"NM PROC NO load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    cGlob->SetLocalProcNo(nProcNo);

    nRet = m_cStmConfig->DBGetProcNo((char*)"ATOM", (char*)"EMS", (char*)"ALM", &nProcNo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"NM PROC NO load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    cGlob->SetAlmNo(nProcNo);

    nRet = m_cStmConfig->DBLoadResourceInfo(m_cResInfo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource Information load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    nRet = m_cStmConfig->DBLoadResourceMergeTime(m_cResInfo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource collect time load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    m_cResInfo->SetDbInfo(m_strDbName, m_strDbIp, m_nDbPort, m_strDbUser, m_strDbPasswd);

    nRet = m_cStmConfig->DBLoadAlarmMergeTime(m_cAlarmInfo);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Resource collect time load failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    m_cAlarmInfo->SetDbInfo(m_strDbName, m_strDbIp, m_nDbPort, m_strDbUser, m_strDbPasswd);

    m_cStmConfig->DBLoadPackage(m_lstPackageInfo);
    m_cStmConfig->DBLoadNode(m_lstNodeInfo, m_lstPackageInfo);

    /* load thread start */ 
    CMergeThread *cMergeThrd = NULL;

    CThreadQueue<CMergeSession*> *rcvQueue = NULL;

    rcvQueue = new CThreadQueue<CMergeSession*>();
    cMergeThrd = new CMergeThread(rcvQueue);

    cMergeThrd->Start();

    cGlob->SetMergeSndQueue(rcvQueue);

    return STM_OK;

}

int CMain::Run()
{
    int nRet = 0;
    int nLoopCnt = 0;
    time_t curTm = 0;
    time_t prevTm1 = 0; /* 1sec */
    time_t prevTm3 = 0; /* 3sec */

    nRet = Init();
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Init failed(nRet=%d)\n",nRet);
        return STM_NOK;
    }

    prevTm1 = prevTm3  = curTm = time(NULL);

    /* config test line end */
    while(1){
        nLoopCnt = 0;
#if 0
        /* marge test */
        DEBUG_LOG("CHECK TIME=%s",ctime(&curTm));
        MargeHandler(curTm);
        sleep(1);
        curTm += 3600;
        /* end ----------*/
#endif
#if 1
        /* package proces */
        nLoopCnt += PackageRun();

        /* node process */
        nLoopCnt += ReceiveHandler();

        m_cLowerInter->Handler();

        if((curTm - prevTm1) >= 1){
            bool blnDstFlag = false;
            struct tm *t = NULL;
            t = localtime(&curTm);

            if(t->tm_isdst == 0){
                blnDstFlag = false;
            }
            else{
                blnDstFlag = true;
            }

            /* check resoruce */
            ResourceHandler(curTm,  blnDstFlag);

            /* check Makrage */
            MargeHandler(curTm);

            prevTm1 = curTm;
        }

        if((curTm - prevTm3) >= 3){
            NodeSendHeartBeat();

            prevTm3 = curTm;
        }

        if(nLoopCnt <= 10){
            usleep(1000);
        }
        curTm = time(NULL);	
#endif
    }/* end of while(1) */

    return STM_OK;
}

int main()
{
    CMain *cMain = NULL;
    int nRet = 0;
    CThreadQueue<int> cq;

    cMain = new CMain(nRet);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"main init failed(nRet=%d)\n",nRet);
        return 0;
    }

    cMain->Run();

    delete cMain;

    return 0;
}

