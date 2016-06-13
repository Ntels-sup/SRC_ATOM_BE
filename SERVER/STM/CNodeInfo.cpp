#include <stdio.h>
#include <string.h>
#include <string>
#include "CommandFormat.hpp"
#include "CGlobal.hpp"
#include "CPackageInfo.hpp"
#include "CNodeInfo.hpp"
#include "CStmApi.hpp"
#include "CResGroup.hpp"
#include "CStmReqApi.hpp"
#include "CStmAnsApi.hpp"
#include "CStmResApi.hpp"
#include "CStmResReqApi.hpp"
#include "CStmResRspApi.hpp"

CNodeInfo::CNodeInfo() 
{
    m_nStatus = INACTIVE_STATUS;
    m_nLastRcvTime = 0;
    m_nLastCollectTime = 0;
    m_nSessionSequenceId = 0;
    m_lstTCAInfo = new list<CTCAInfo*>;
    m_lstQueuedSession = new list<CSessionInfo*>;
    m_lstCompleteSession = new list<CSessionInfo*>;
}

CNodeInfo::~CNodeInfo()
{
    delete m_lstQueuedSession;
    delete m_lstCompleteSession;
}

int CNodeInfo::Init(int a_nNodeNo, const char *a_strNodeType, const char *a_chNodeName, int a_nLastCollectTime)
{
    int nLen = 0;

    m_nNodeNo = a_nNodeNo;
    m_strNodeType = a_strNodeType;

    nLen = strlen(a_chNodeName);
    if(nLen >= NODE_NAME_LEN){
        return STM_NOK;
    }

    m_strNodeName = a_chNodeName;

    m_nLastCollectTime = a_nLastCollectTime;

    return STM_OK;
}

int CNodeInfo::InsertTCAInfo(CTCAInfo *a_cTCA)
{
    m_lstTCAInfo->push_back(a_cTCA);

    return STM_OK;
}

int CNodeInfo::MakeResSession(list<string> *a_lstSendGrpId, time_t a_startTime, bool a_blnDstFlag)
{
    int nRet = 0;
    CSessionInfo *cSession = NULL;

    cSession = new CSessionInfo;

    nRet = cSession->MakeRSC(m_nSessionSequenceId, a_startTime,  a_blnDstFlag, a_lstSendGrpId);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Session make failed(ret=%d, seqId=%d\n",nRet, m_nSessionSequenceId);
        delete cSession;
        return STM_NOK;
    }

    DEBUG_LOG("SEND RESOURCE SESSION\n");
    nRet = SendProcess(cSession);
    if(nRet == STM_ERR_NODE_INACT){
        delete cSession;
    }

    m_nSessionSequenceId++;

    m_lstQueuedSession->push_back(cSession);


    return STM_OK;
}

int CNodeInfo::MakeStsSession(string &a_strPackageName, string &a_strTableName, time_t a_nStartTime,
                                unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlag)
{
    int nRet = 0;
    CSessionInfo *cSession = NULL;

    cSession = new CSessionInfo;

    nRet = cSession->MakeSTS(m_nSessionSequenceId, a_strPackageName, a_strTableName, a_nStartTime, 
            a_nCollectTime, a_blnDstFlag, a_blnActFlag);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Session make failed(ret=%d, seqId=%d\n",nRet, m_nSessionSequenceId);
        delete cSession;
        return STM_NOK;
    }

    m_nSessionSequenceId++;

    m_lstQueuedSession->push_back(cSession);

    SendProcess(cSession);

    return STM_OK;
}

CSessionInfo *CNodeInfo::GetSession(int a_nSessionId)
{
    CSessionInfo *pcFindSession = NULL;
    list<CSessionInfo*>::iterator iter;

    for(iter = m_lstQueuedSession->begin(); iter != m_lstQueuedSession->end();iter++){
        pcFindSession = *iter;
        if(pcFindSession->GetSessionId() == a_nSessionId){
            m_lstQueuedSession->erase(iter);
            return pcFindSession;
        }
    }

    return NULL;
}

int CNodeInfo::SendTCAToALM(string &a_strPackageName, string &a_strTableName, string &a_strColumnName, 
				int a_nSeverity, int a_nValue)
{
    char *szTarget = NULL;
    int nTargetLen = 0;
    int nTimeLen = 0;
    char szTime[63];
    char szValue[12];
    time_t curTime = 0;
    struct tm *stTm = NULL;
    string strComplement;
    int nSeverityId = 0;
    string strData;


    switch(a_nSeverity){
        case TCA_RULE_OVER_CRITICAL:  nSeverityId = ALM::eCRITICAL; strComplement = "OVER";   break;
        case TCA_RULE_OVER_MAJOR:     nSeverityId = ALM::eMAJOR;    strComplement = "OVER";   break;
        case TCA_RULE_OVER_MINOR:     nSeverityId = ALM::eMINOR;    strComplement = "OVER";   break;
        case TCA_RULE_CLEARED:        nSeverityId = ALM::eCLEARED;  strComplement = "";       break;
        case TCA_RULE_UNDER_CRITICAL: nSeverityId = ALM::eCRITICAL; strComplement = "UNDER";  break;
        case TCA_RULE_UNDER_MAJOR:    nSeverityId = ALM::eMAJOR;    strComplement = "UNDER";  break;
        case TCA_RULE_UNDER_MINOR:    nSeverityId = ALM::eMINOR;    strComplement = "UNDER";  break;
    };

    try{
        rabbit::object cRoot;
        rabbit::object cBody = cRoot["BODY"];

        cBody["message"] = "tca alarm";
        cBody["node_no"] = m_nNodeNo;
        cBody["node_type"] = m_strNodeType;
        cBody["node_name"] = m_strNodeName;
        cBody["pkg_name"] = a_strPackageName;
        cBody["severity_id"] = nSeverityId;

        curTime = time(NULL);

        stTm = localtime(&curTime);

        nTimeLen = strftime(szTime, 62, "%04Y-%02m-%02d %02H:%02M:%02S.00",stTm);
        szTime[nTimeLen] = '\0';

        cBody["prc_date"] = szTime;
        if(stTm->tm_isdst > 0) { cBody["dst_yn"] = "Y"; }
        else { cBody["dst_yn"] = "N"; }

        szTarget = new char [m_strNodeName.size() + a_strTableName.size() + a_strColumnName.size() + 4];
        nTargetLen = sprintf(szTarget,"/%s/%s/%s",
                m_strNodeName.c_str(), a_strTableName.c_str(), a_strColumnName.c_str());
        szTarget[nTargetLen] = '\0';
        cBody["target"] = szTarget;
        snprintf(szValue,11,"%d",a_nValue);
        szValue[11] = '\0';
        cBody["value"] = szValue;
        cBody["complement"] = strComplement;
        cBody["additional_text"] = "";

        DEBUG_LOG("TCA ALARM SEND : %s\n", cRoot.str().c_str());

        strData = cRoot.str();

        delete[] szTarget;

    } catch(rabbit::type_mismatch   e) {
        STM_LOG(STM_ERR,"%s\n",e.what());
        return STM_NOK;
    } catch(rabbit::parse_error e) {
        STM_LOG(STM_ERR,"%s\n",e.what());
        return STM_NOK;
    } catch(...) {
        STM_LOG(STM_ERR,"UNKNOWN ERROR\n");
        return STM_NOK;
    }

    /* send TCA */
    CProtocol cProto;
    CGlobal *cGlob = NULL;
    CLowerInterface *cLowerInter = NULL;

    cGlob = CGlobal::GetInstance();

    cProto.SetCommand("0000020001");
    cProto.SetFlagNotify();
    cProto.SetSequence(0);
    cProto.SetSource(cGlob->GetLocalNodeNo(), cGlob->GetLocalProcNo());
    cProto.SetDestination(cGlob->GetLocalNodeNo(), cGlob->GetAlmNo());

    cProto.SetPayload(strData);

    /* send message */
    cLowerInter = cGlob->GetLowerInterfaceP();
    if(cLowerInter->Send(cProto) != STM_OK){
        STM_LOG(STM_ERR,"TCA Alarm Message send failed\n");
        return STM_NOK;
    }

    return STM_OK;
}

int CNodeInfo::CheckTCA(string &a_strPackageName, string &a_strTableName, string &a_strColumnName, int a_nValue)
{
    int nRet = 0;
    int nAlarmId = 0;
    list<CTCAInfo*>::iterator iter;
    list<int> lstAlarmList;
    CTCAInfo *cFindTCA = NULL;

    for(iter=m_lstTCAInfo->begin(); iter != m_lstTCAInfo->end(); iter++){
        cFindTCA = *iter;

        nRet = cFindTCA->CheckPackageName(a_strPackageName);
        if(nRet != STM_OK){
            continue;
        }

        nRet = cFindTCA->CheckTableName(a_strTableName);
        if(nRet != STM_OK){
            continue;
        }


        nRet = cFindTCA->CheckColumnName(a_strColumnName);
        if(nRet != STM_OK){
            continue;
        }

        DEBUG_LOG("FIND TCA RULE\n");
        nRet = cFindTCA->CheckRule(a_nValue, &lstAlarmList);
        if(nRet == STM_OK){
            nAlarmId = lstAlarmList.front();
            /* send TCA */
            DEBUG_LOG("SEND TCA(ID=%d)\n",nAlarmId);
            nRet = SendTCAToALM(a_strPackageName, a_strTableName, a_strColumnName, nAlarmId, a_nValue);
            if(nRet != STM_OK){
                STM_LOG(STM_ERR,"TCA Alarm Send failed\n");
            }


            lstAlarmList.pop_front();
        }

    }
    return STM_OK;
}

int CNodeInfo::RecvHistHandler(string &a_strData, CSessionInfo **cSession)
{
    int nRet = STM_OK;
    int nTableType = 0;
    CStmAnsApi decApi;
    string strTableName;
    string strPackageName;
    CSessionInfo *cCurSession = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;
    time_t curTm = 0;
    struct tm *t = NULL;
    bool dstFlag = false;

    nRet = decApi.DecodeMessage((char*)a_strData.c_str(),a_strData.size());
    if(nRet != CStmApi::RESULT_OK){
        STM_LOG(STM_ERR,"Message decoding failed(nRet=%d, str=%s)\n",nRet,decApi.GetStrError().c_str());
        return NULL;
    }

    nTableType = decApi.GetTableType();

    if(nTableType != CStmApi::TABLE_TYPE_HIST){
        STM_LOG(STM_ERR,"Invalid Table Type(TableType=%d)\n",nTableType);
        return STM_NOK;
    }

    /* make session */
    cCurSession = new CSessionInfo();

    strTableName = decApi.GetTableName();

    curTm = time(NULL);

    t = localtime(&curTm);
    if(t->tm_isdst == 0){
        dstFlag = false;
    }
    else{
        dstFlag = true;
    }

    strPackageName = decApi.GetPackageName();
    nRet = cCurSession->MakeHIST(0, strPackageName,
            strTableName, curTm, dstFlag);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Session make filed(nRet=%d)\n",nRet);
    }

    lstCollectValue = decApi.GetCollectValue();

    cCurSession->SetCollectValue(lstCollectValue);

    *cSession = cCurSession;

    return STM_OK;
}

int CNodeInfo::RecvResStsHandler(string &a_strData, CSessionInfo **cSession)
{
    int nRet = STM_OK;
    int nResultCode = 0;
    int nSessionId = 0;
    CSessionInfo *cCurSession = NULL;
    CStmResRspApi decApi;
    CResGroup *cResGroup = NULL;
    list<CResGroup*> *lstResGroup= NULL;

    nRet = decApi.DecodeMessage(a_strData);
    if(nRet != CStmApi::RESULT_OK){
        STM_LOG(STM_ERR,"Message decoding failed(nRet=%d, str=%s)(msg=%s)\n",
                nRet, decApi.GetStrError().c_str(), a_strData.c_str());
        return STM_NOK;
    }

    /* Get session Id */
    nSessionId = decApi.GetSessionId();

    nResultCode = decApi.GetResultCode();
    if(nResultCode != CStmResApi::RESAPI_RESULT_CODE_SUCCESS){
        STM_LOG(STM_ERR,"STA Error occure(session=%d, code=%d)\n",nSessionId, nRet);
    }

    cCurSession = GetSession(nSessionId);
    if(cCurSession == NULL){
        STM_LOG(STM_ERR,"Session not exist (id=%d)\n",nSessionId);
        return STM_NOK;
    }

    if(nResultCode != CStmResApi::RESAPI_RESULT_CODE_SUCCESS){
        string strPkgName;
        string strTableName;

        STM_LOG(STM_ERR,"Delete session(sessionId=%d, node=%s)\n", nSessionId, m_strNodeName.c_str());
        return STM_ERR_RESULT_RECEIVE;
    }

    lstResGroup= new list<CResGroup*>;

    while(1){
        cResGroup = decApi.GetFirstResGroup();
        if(cResGroup == NULL){
            break;
        }

        lstResGroup->push_back(cResGroup);
    }


    cCurSession->SetResGroup(lstResGroup);

    *cSession = cCurSession;

    return STM_OK;
}

int CNodeInfo::RecvStsHandler(string &a_strData, CSessionInfo **cSession)
{
    int nRet = STM_OK;
    int nResultCode = 0;
    int nSessionId = 0;
    int nTableType = 0;
    CSessionInfo *cCurSession = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;
    CStmAnsApi decApi;

    nRet = decApi.DecodeMessage((char*)a_strData.c_str(),a_strData.size());
    if(nRet != CStmApi::RESULT_OK){
        STM_LOG(STM_ERR,"Message decoding failed(nRet=%d, str=%s)(msg=%s)\n",
                nRet, decApi.GetStrError().c_str(), a_strData.c_str());
        return STM_NOK;
    }

    /* Get session Id */
    nSessionId = decApi.GetSessionId();

    nResultCode = decApi.GetResultCode();
    if(nResultCode != CStmApi::RESULT_CODE_SUCCESS){
        STM_LOG(STM_ERR,"STA Error occure(session=%d, code=%d)\n",nSessionId, nRet);
    }

    cCurSession = GetSession(nSessionId);
    if(cCurSession == NULL){
        STM_LOG(STM_ERR,"Session not exist (id=%d)\n",nSessionId);
        return STM_NOK;
    }

    if(nResultCode != CStmApi::RESULT_CODE_SUCCESS){
        string strPkgName;
        string strTableName;

        strPkgName = cCurSession->GetPackageName();
        strTableName = cCurSession->GetTableName();
        STM_LOG(STM_ERR,"Delete session(sessionId=%d, node=%s, pkg=%s, table=%s)\n",nSessionId, m_strNodeName.c_str(), 
                strPkgName.c_str(), strTableName.c_str());
        return STM_ERR_RESULT_RECEIVE;
    }

    nTableType = decApi.GetTableType();

    if(nTableType != CStmApi::TABLE_TYPE_STS){
        STM_LOG(STM_ERR,"Invalid Table Type(TableType=%d)\n",nTableType);
        return STM_NOK;
    }


    lstCollectValue = decApi.GetCollectValue();

    cCurSession->SetCollectValue(lstCollectValue);

    *cSession = cCurSession;

    return STM_OK;
}

int CNodeInfo::SendProcess(CSessionInfo *cSession)
{
    int nRet = 0;
    CGlobal *cGlob = NULL;
    string strTableName;
    CProtocol cProto;
    CLowerInterface *cLowerInter = NULL;
    string strPayload;
    time_t curTm = 0;

    if(m_nStatus == INACTIVE_STATUS){
        STM_LOG(STM_ERR,"INACT NODE\n");
        return STM_OK;
    }

    curTm = time(NULL);

    if((curTm - m_nLastRcvTime) > HEARTBEAT_TIMEOUT_TM){
        STM_LOG(STM_ERR,"NODE TIMEOUT (curTm=%lu, last HBSendTm=%lu)\n", curTm, m_nLastRcvTime);
        m_nStatus = INACTIVE_STATUS;
        return STM_OK;
    }

    cGlob = CGlobal::GetInstance();

    /* make message */
    cProto.SetFlagRequest();
    cProto.SetSequence(cSession->GetSessionId());
    cProto.SetSource(cGlob->GetLocalNodeNo(), cGlob->GetLocalProcNo());

    if(cSession->GetType() == STM_TABLE_TYPE_STS){
        CStmReqApi cReqApi;

        cProto.SetCommand("0000030004");
        strTableName = cSession->GetTableName();
        cReqApi.Init(cSession->GetSessionId(), cSession->GetStartTime(), cSession->GetCollectTime());
        cReqApi.SetTableName(strTableName);

        nRet = cReqApi.EncodeMessage(strPayload);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Message encoding failed(nRet=%d)\n",nRet);
            return STM_NOK;
        }

        /* test destination node */
        cProto.SetDestination(m_nNodeNo , PROCID_ATOM_NA_STA);

        cProto.SetPayload(strPayload);
    }
    else if(cSession->GetType() == STM_TABLE_TYPE_RES){
        CStmResReqApi cResReqApi;
        list<string>  *lstGrpIdList = NULL;
        list<string>::iterator iter;

        cProto.SetCommand("0000030003");
        lstGrpIdList = cSession->GetResGrpIdListP();

        cResReqApi.Init(cSession->GetSessionId(), curTm);

		/* debug */
		int size= 0;
		string test;
		size = lstGrpIdList->size();
		/*---------*/
        for(iter = lstGrpIdList->begin();iter != lstGrpIdList->end();iter++){
			/* debug */
			test = *iter;
			/*-------*/
            cResReqApi.InsertRscGrpId(*iter);
        }

        nRet = cResReqApi.EncodeMessage(strPayload);
        if(nRet != STM_OK){
            STM_LOG(STM_ERR,"Resource Message encoding failed(nRet=%d)\n",nRet);
            return STM_NOK;
        }

        DEBUG_LOG("RESOURCE REQ=%s\n",strPayload.c_str());

        /* test destination node */
        cProto.SetDestination(m_nNodeNo , PROCID_ATOM_NA_RSA);

        cProto.SetPayload(strPayload);
    }
    else {
        STM_LOG(STM_ERR,"Invailed Session type(%d)\n", cSession->GetType());
        return STM_NOK;
    }

    /* send message */
    cLowerInter = cGlob->GetLowerInterfaceP();
    nRet = cLowerInter->Send(cProto);
    if(nRet != CNMInterface::RESULT_OK){
        STM_LOG(STM_ERR,"STS Message send failed(%d)\n", nRet);
        return STM_NOK;
    }

    return STM_OK;
}

int CNodeInfo::SendHeartBeat()
{
    int nRet = 0;
    /* send TCA */
    CProtocol cProto;
    CGlobal *cGlob = NULL;
    CLowerInterface *cLowerInter = NULL;

    cGlob = CGlobal::GetInstance();

    cProto.SetCommand("0000030008");
    cProto.SetFlagRequest();
    cProto.SetSequence(0);
    cProto.SetSource(cGlob->GetLocalNodeNo(), cGlob->GetLocalProcNo());
    cProto.SetDestination(m_nNodeNo , PROCID_ATOM_NA_STA);

    /* send message */
    cLowerInter = cGlob->GetLowerInterfaceP();
    nRet = cLowerInter->Send(cProto);
    if(nRet != CNMInterface::RESULT_OK){
        STM_LOG(STM_ERR,"Heart beat Message send failed(%d)\n",nRet);
        return STM_NOK;
    }

    return STM_OK;
}


int CNodeInfo::ResendAllPendingSession()
{
    list<CSessionInfo*>::iterator iter;

    DEBUG_LOG("RESEND PENDING SESSION\n");

    for(iter = m_lstQueuedSession->begin();iter != m_lstQueuedSession->end();iter++){
        SendProcess(*iter);
    }

    return STM_OK;
}

