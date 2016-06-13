#include <stdio.h>
#include <string.h>
#include "STAAPI.h"
#include "CommandFormat.hpp"
#include "CGlobal.hpp"
#include "CProtocol.hpp"
#include "CStmReqApi.hpp"
#include "CStmAnsApi.hpp"
#include "CStaAnsApi.hpp"
#include "CSessionInfo.hpp"
#include "CEmsInfo.hpp"

CEmsInfo::CEmsInfo(CTableList *a_cTableList, string &a_strPackageName, int &nRet)
{
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    CAddress *cAddr = NULL;
    m_nCollectWaitTime = 3;
    m_nMessageBufferLen = 0;
    m_strPackageName = a_strPackageName;

    m_cTableList = a_cTableList;


    cGlob = CGlobal::GetInstance();
    cIpc = cGlob->GetModuleIPC();
    cAddr = cGlob->GetAddress();

    nRet = cAddr->LookupAtom("ATOM_STM", m_nAtomNodeNo, m_nStmNo);
    if(nRet <= 0){
        STA_LOG(STA_ERR,"Lookup failed\n");
        nRet = STA_NOK;
    }
    STA_LOG(STA_ERR,"ADDRESS LOOKUP(ATOM NODE NO=%d, STM_NO=%d)\n",m_nAtomNodeNo, m_nStmNo);

    nRet = STA_OK;
}

CEmsInfo::~CEmsInfo()
{
    m_cTableList = NULL;
}

int CEmsInfo::ReceiveResourceProcess(CProtocol &cProto)
{
    int nRet = 0;
    std::vector<char> vecPayload;
    string strTableName;
    CStaAnsApi cDecApi;
    CStmAnsApi cEncApi;
    char chFlag = 0;
    CTableData *cTableData = NULL;
    list<string> *lstPrimaryKey = NULL;
    list<int> *lstValue = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;
    CCollectValue *cCollectValue = NULL;

    if(chFlag != CProtocol::FLAG_RESPONSE){
        STA_LOG(STA_ERR,"Invalid Flag(flag=%d)\n",chFlag);
        return STA_NOK;
    }

    cProto.GetPayload(vecPayload);

    nRet = cDecApi.DecodeMessage(&vecPayload[0], vecPayload.size());
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message decoding failed(nRet=%d)\n",nRet);
        return STA_NOK;
    }

    lstCollectValue = new list<CCollectValue*>;
    while(1){
        cTableData = cDecApi.GetFirstTableData();
        if(cTableData == NULL){
            break;
        }

        strTableName = cTableData->GetTableName();
        if(strTableName.size() == 0){
            STA_LOG(STA_ERR,"table name not exist\n");
            delete cTableData;
            continue;
        }

        lstPrimaryKey = cTableData->GetPrimaryKey();
        lstValue = cTableData->GetValue();

        cCollectValue = new CCollectValue();
        cCollectValue->SetPrimaryKey(lstPrimaryKey);
        cCollectValue->SetValue(lstValue);

        lstCollectValue->push_back(cCollectValue);

        delete cTableData;
    }/* end of while(1) */

    cEncApi.SetCollectValue(lstCollectValue);

    nRet = cEncApi.EncodeMessage(m_chMessageBuffer, MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message encoding failed(ret=%d)\n",nRet);
        return STA_NOK;
    }

    SendProcess(CMD_STS_STA_RESOURCE, CProtocol::FLAG_RESPONSE, m_chMessageBuffer, m_nMessageBufferLen);

    return STA_OK;
}

int CEmsInfo::DirectSendProcess(string a_strTableName, string a_strPackageName, int a_nType, 
				list<string> *a_lstPrimaryKey, list<int> *a_lstValue)
{
    int nRet = 0;
    CStmAnsApi *cEncApi = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;
    CCollectValue *cCollectValue = NULL;

    cCollectValue = new CCollectValue();

    cCollectValue->SetPrimaryKey(a_lstPrimaryKey);
    cCollectValue->SetValue(a_lstValue);

    lstCollectValue = new list<CCollectValue*>;

    lstCollectValue->push_back(cCollectValue);

    /* real time stat */
    cEncApi = new CStmAnsApi();
    nRet = cEncApi->Init(0, a_strPackageName, a_strTableName, a_nType);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Encoding api create failed(nRet=%d)\n",nRet);
        delete cCollectValue;
        return STA_NOK;
    }

    nRet = cEncApi->SetCollectValue(lstCollectValue);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"STS Value incoding failed(nRet=%d)\n",nRet);
        delete cEncApi;
        return STA_NOK;
    }

    nRet = cEncApi->EncodeMessage(m_chMessageBuffer, MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message encoding failed(ret=%d)\n",nRet);
        delete cEncApi;
        return STA_NOK;

    }

    delete cEncApi;

    SendProcess(CMD_STS_STA_HIST, CProtocol::FLAG_NOTIFY, m_chMessageBuffer, m_nMessageBufferLen);

    return STA_OK;
}

int CEmsInfo::ReceiveStaRequestProcess(CProtocol &cProto)
{
    std::vector<char> vecPayload;
    CSessionInfo *cSessionInfo = NULL;
    CStmReqApi cDecApi;
    char *szPayload = NULL;
    char chFlag = 0;
    time_t tm = 0;

    tm = time(NULL);

    chFlag = cProto.GetFlag();

    if(chFlag != CProtocol::FLAG_REQUEST){
        STA_LOG(STA_ERR,"Invalid Flag(flag=%d)\n",chFlag);
        return STA_NOK;
    }

    cProto.GetPayload(vecPayload);

    cSessionInfo = new CSessionInfo();

    cSessionInfo->SetReceiveTime(tm);

    szPayload = new char[vecPayload.size() + 1];

    strncpy(szPayload, &vecPayload[0], vecPayload.size());
    szPayload[vecPayload.size()] = '\0';
    cSessionInfo->SetReceiveData(szPayload, vecPayload.size());

    DEBUG_LOG("RECEIVE DATA (Wait 3 time)\n");
    m_lstPendingQueue.push_back(cSessionInfo);

    return STA_OK;
}

int CEmsInfo::SendErrorToStm(int a_nSessionId, int a_nErrorCode)
{
    int nRet = 0;
    CStmAnsApi cEncApi;

    STA_LOG(STA_ERR,"ERORR SEND TO STM(code=%d)\n",a_nErrorCode);
    nRet = cEncApi.EncodeErrorMessage(a_nSessionId, a_nErrorCode, m_chMessageBuffer, 
            MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message encoding failed(ret=%d)\n",nRet);
        return STA_NOK;

    }

    SendProcess(CMD_STS_STA, CProtocol::FLAG_RESPONSE, m_chMessageBuffer, m_nMessageBufferLen);

    return STA_OK;
}

int CEmsInfo::CollectAnswer(CSessionInfo *cSession)
{
    int nRet = 0;
    CStmReqApi cDecApi;
    CStmAnsApi *cEncApi = NULL;
    int nSessionId = 0;
    char *chReceiveData = NULL;
    int nReceiveDataLen = 0;
    time_t startTime = 0;
    int nCollectTime = 0;
    string strTableName;
    CTableInfo *cTable = NULL;
    list<CCollectValue*> *lstCollectValue = NULL;

    nRet = cSession->GetReceiveData(&chReceiveData, &nReceiveDataLen);
    if(nRet != STA_OK){
        STA_LOG(STA_ERR,"Session data receive failed(nRet=%d)\n",nRet);
        return STA_NOK;
    }

    nRet = cDecApi.DecodeMessage(chReceiveData, nReceiveDataLen);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message decoding failed(nRet=%d)\n",nRet);
        delete[] chReceiveData;
        return STA_NOK;
    }

    delete[] chReceiveData;

    nSessionId = cDecApi.GetSessionId();
    startTime = cDecApi.GetStartTime();
    nCollectTime = cDecApi.GetCollectTime();
    strTableName = cDecApi.GetTableName();

    cTable = m_cTableList->SearchTableInfo(strTableName);
    if(cTable == NULL){
        STA_LOG(STA_ERR,"Table not exist(%s)\n", strTableName.c_str());
        nRet = SendErrorToStm(nSessionId, CStmApi::RESULT_CODE_TABLE_NOT_EXIST);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Error Message send failed(nRet=%d)\n",nRet);
        }
        return STA_NOK;
    }

    lstCollectValue = cTable->GetCollectValue(startTime, nCollectTime);
    if(lstCollectValue == NULL){
        STA_LOG(STA_ERR,"Collect value not exist(startTime=%lu, collectTime=%d, tablename=%s)\n", 
                startTime, nCollectTime, strTableName.c_str());
        nRet = SendErrorToStm(nSessionId, CStmApi::RESULT_CODE_DATA_NOT_EXIST);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Error Message send failed(nRet=%d)\n",nRet);
        }
        return STA_NOK;
    }

    /* drop prev collect Value */
    cTable->DropStsValue(startTime);

    if(cTable->GetValueCount() == 0){
        m_cTableList->DeleteTableInfo(strTableName);
    }

    cEncApi = new CStmAnsApi();

    nRet = cEncApi->Init(nSessionId, m_strPackageName, strTableName, CStmApi::TABLE_TYPE_STS);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Encoding api create failed(ret=%d err=%s)\n",nRet, cEncApi->GetStrError().c_str());
        nRet = SendErrorToStm(nSessionId, CStmApi::RESULT_CODE_UNKNOWN_ERROR);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Error Message send failed(nRet=%d)\n",nRet);
        }
        delete cEncApi;
        return STA_NOK;
    }

    nRet = cEncApi->SetCollectValue(lstCollectValue);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"STS Value incoding failed(nRet=%d)\n",nRet);
        nRet = SendErrorToStm(nSessionId, CStmApi::RESULT_CODE_UNKNOWN_ERROR);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Error Message send failed(nRet=%d)\n",nRet);
        }
        delete cEncApi;
        return STA_NOK;
    }

    nRet = cEncApi->EncodeMessage(m_chMessageBuffer, MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
    if(nRet != CStmApi::RESULT_OK){
        STA_LOG(STA_ERR,"Message encoding failed(ret=%d)\n",nRet);
        nRet = SendErrorToStm(nSessionId, CStmApi::RESULT_CODE_UNKNOWN_ERROR);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Error Message send failed(nRet=%d)\n",nRet);
        }
        delete cEncApi;
        return STA_NOK;

    }

    delete cEncApi;

    SendProcess(CMD_STS_STA, CProtocol::FLAG_RESPONSE, m_chMessageBuffer, m_nMessageBufferLen);

    return STA_OK;
}

int CEmsInfo::CollectProcess()
{
    int nRet = 0;
    time_t receiveTime = 0;
    time_t currentTime = 0;
    list<CSessionInfo*>::iterator iter;
    CSessionInfo *cSession = NULL;

    currentTime = time(NULL);

    for(iter=m_lstPendingQueue.begin();iter != m_lstPendingQueue.end();){
        cSession = *iter;
        receiveTime = cSession->GetReceiveTime();
        if((currentTime - receiveTime) > RECEIVE_WAIT_TIME){
            DEBUG_LOG("RECEIVE PROCESS\n");
            /* receive process */
            nRet = CollectAnswer(cSession);
            if(nRet != STA_OK){
                STA_LOG(STA_ERR,"STS Collect failed(nRet=%d)\n",nRet);
            }
            m_lstPendingQueue.erase(iter++);
            delete cSession;
        }
        else {
            iter++;
        }
    }

    return STA_OK;
}

int CEmsInfo::SendProcess(int a_nCmdCode, int a_nFlag, char *a_chMessageBuffer, int a_nMessageBufferLen)
{
    bool blnRet = true;
    char strCmdCode[11];
    int nNodeNo = 0;
    int nProcNo = 0;
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    CProtocol cProto;

    DEBUG_LOG("SEND PROCESS\n");

    cGlob = CGlobal::GetInstance();

    nNodeNo = cGlob->GetLocalNodeNo();
    nProcNo =  cGlob->GetLocalProcNo();

    cProto.SetSource(nNodeNo, nProcNo);

    /* TEST */
    cProto.SetDestination(m_nAtomNodeNo, m_nStmNo);

    /* flag setting */
    switch(a_nFlag){
        case CProtocol::FLAG_REQUEST:   cProto.SetFlagRequest();    break;
        case CProtocol::FLAG_RESPONSE:  cProto.SetFlagResponse();   break;
        case CProtocol::FLAG_NOTIFY:    cProto.SetFlagNotify();     break;
        case CProtocol::FLAG_RETRNS:    cProto.SetFlagRetransmit(); break;
        case CProtocol::FLAG_BROAD:     cProto.SetFlagBroadcast();  break;
        case CProtocol::FLAG_ERROR:     cProto.SetFlagError();      break;
        default:
                                        STA_LOG(STA_ERR,"Invalid Message flag(%d)\n",a_nFlag);
                                        return STA_NOK;
    }

    /* PKG CLASS ACTION 
     * 000 003 XXXX
     */
    snprintf(strCmdCode, 11,"00000%d",a_nCmdCode);
    strCmdCode[11] = '\0';
    cProto.SetCommand(strCmdCode);

    cProto.SetPayload(a_chMessageBuffer, a_nMessageBufferLen);

    cIpc = cGlob->GetModuleIPC();

    blnRet = cIpc->SendMesg(cProto);
    if(blnRet != true){
        STA_LOG(STA_ERR,"IPC Send failed(cmdCode=%d, flag=%d)\n", a_nCmdCode, a_nFlag);
        return STA_NOK;
    }

    m_nMessageBufferLen = 0;

    return STA_OK;
}

int CEmsInfo::SendHeartBeatRsp()
{
    bool blnRet = true;
    int nNodeNo = 0;
    int nProcNo = 0;
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    CProtocol cProto;

    DEBUG_LOG("SEND HEARTBEAT RESPONSE PROCESS\n");

    cGlob = CGlobal::GetInstance();

    nNodeNo = cGlob->GetLocalNodeNo();
    nProcNo =  cGlob->GetLocalProcNo();

    cProto.SetSource(nNodeNo, nProcNo);

    cProto.SetDestination(m_nAtomNodeNo, m_nStmNo);

    /* flag setting */
    cProto.SetFlagResponse();

    /* PKG CLASS ACTION 
     * 000 003 XXXX
     */
    cProto.SetCommand("0000030008");

    cIpc = cGlob->GetModuleIPC();

    blnRet = cIpc->SendMesg(cProto);
    if(blnRet != true){
        STA_LOG(STA_ERR,"IPC Send failed(HEARTBEAT RESPONSE)\n");
        return STA_NOK;
    }

    return STA_OK;
}
