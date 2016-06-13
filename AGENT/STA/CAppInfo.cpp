#include <stdio.h>
#include <string.h>
#include "CommandFormat.hpp"
#include "STAAPI.h"
#include "CGlobal.hpp"
#include "CAppInfo.hpp"
#include "CStmApi.hpp"
#include "CStaReqApi.hpp"
#include "CStaAnsApi.hpp"

CAppInfo::CAppInfo(CEmsInfo *a_cEmsInfo, string &a_strPackageName, CTableList *a_cCollectTableList, int a_nCollectTime)
{
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    m_cEmsInfo = a_cEmsInfo;
    m_cTableList = a_cCollectTableList;
    m_nLastSendTime = 0;
    m_strPackageName = a_strPackageName;

    cGlob = CGlobal::GetInstance();
    cIpc = cGlob->GetModuleIPC();

    m_nCollectTime = a_nCollectTime;
    m_nMessageBufferLen = 0;
}

CAppInfo::~CAppInfo()
{
}

int CAppInfo::SendCollectMessage()
{
    int nRet = 0;
    bool blnRet = true;
    int nNodeNo = 0;
    int nProcNo = 0;
    char strCmdCode[11];
    time_t currentTime = 0;
    CProtocol cProto;
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    CStaReqApi cReqApi;

    currentTime = time(NULL);

    if((m_nLastSendTime != currentTime ) &&
            ((currentTime % m_nCollectTime) == 0)){
        /* send message */
        DEBUG_LOG("SEND MESSAGE(time=%d, collectTime=%d , mod=%d, lastSendTime=%d)\n", 
                (int)currentTime, (int)m_nCollectTime, (int)(currentTime % m_nCollectTime), (int)m_nLastSendTime);
        STA_LOG(STA_ERR,"SEND MESSAGE\n");
        /* encode Message */

        cReqApi.SetTime(currentTime);

        nRet = cReqApi.EncodeMessage(m_chMessageBuffer, MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
        if(nRet != STAAPI_OK){
            STA_LOG(STA_ERR,"Message encoding failed(ret=%d, len=%d)\n", nRet, m_nMessageBufferLen);
        }

        cGlob = CGlobal::GetInstance();

        nNodeNo = cGlob->GetLocalNodeNo();
        nProcNo =  cGlob->GetLocalProcNo();

        cProto.SetSource(nNodeNo, nProcNo);

        cProto.SetDestination(nNodeNo, PROCID_ATOM_NA_PRA);

        cProto.SetFlagBroadcast();

        /* PKG CLASS ACTION
         * 000 003 XXXX
         */
        snprintf(strCmdCode, 11,"00000%d",CMD_STS_APP);
        strCmdCode[11] = '\0';
        cProto.SetCommand(strCmdCode);

        cProto.SetPayload(m_chMessageBuffer, m_nMessageBufferLen);

        cIpc = cGlob->GetModuleIPC();

        blnRet = cIpc->SendMesg(cProto);
        if(blnRet != true){
            STA_LOG(STA_ERR,"IPC Send failed(cmdCode=%d)\n", CMD_STS_APP);
            m_nLastSendTime = currentTime;
            return STA_NOK;
        }

        m_nMessageBufferLen = 0;

        m_nLastSendTime = currentTime;
    }/* end of if((m_nLastSendTime != currentTime ) &&
        ((currentTime % m_nCollectTime) == 0)) */

    return STA_OK;
}

int CAppInfo::SendResourceMessage()
{
    int nRet = 0;
    bool blnRet = true;
    int nNodeNo = 0;
    int nProcNo = 0;
    char strCmdCode[11];
    time_t currentTime = 0;
    currentTime = time(NULL);
    CGlobal *cGlob = NULL;
    CModuleIPC *cIpc = NULL;
    CStaReqApi cReqApi;
    CProtocol cProto;
    std::vector<char> vecPayload;

    /* send message */
    DEBUG_LOG("SEND RESOURCE MESSAGE(time=%d, collectTime=%d , mod=%d)\n", 
            (int)currentTime, (int)m_nCollectTime, (int)(currentTime % m_nCollectTime));

    /* encode Message */
    cReqApi.SetTime(currentTime);

    nRet = cReqApi.EncodeMessage(m_chMessageBuffer, MAX_MESSAGE_BUFFER_LEN, &m_nMessageBufferLen);
    if(nRet != STAAPI_OK){
        STA_LOG(STA_ERR,"Message encoding failed(ret=%d, len=%d)\n", nRet, m_nMessageBufferLen);
    }

    cGlob = CGlobal::GetInstance();

    nNodeNo = cGlob->GetLocalNodeNo();
    nProcNo =  cGlob->GetLocalProcNo();

    cProto.SetSource(nNodeNo, nProcNo);

    cProto.SetDestination(nNodeNo, PROCID_ATOM_NA_RSA);

    cProto.SetFlagRequest();

    /* PKG CLASS ACTION
     * 000 003 XXXX
     */
    snprintf(strCmdCode, 11,"00000%d",CMD_STS_RSA);
    strCmdCode[11] = '\0';
    cProto.SetCommand(strCmdCode);

    cProto.SetPayload(m_chMessageBuffer, m_nMessageBufferLen);

    cIpc = cGlob->GetModuleIPC();

    blnRet = cIpc->SendMesg(cProto);
    if(blnRet != true){
        STA_LOG(STA_ERR,"IPC Send failed(cmdCode=%d)\n", CMD_STS_RSA);
        return STA_NOK;
    }

    m_nMessageBufferLen = 0;

    return STA_OK;
}

int CAppInfo::ReceiveAppSts(char *a_chMessage, int a_nMessageLen)
{
    int nRet = 0;
    string strTableName;
    bool blnTableAlloc = false;
    time_t collectTime = 0;
    list<string> *lstPrimaryKey = NULL;
    list<int> *lstValue = NULL;
    CStaAnsApi cDecApi;
    CTableInfo *cTable = NULL;
    CTableData *cTableData = NULL;

    nRet = cDecApi.DecodeMessage(a_chMessage, a_nMessageLen);
    if(nRet != STAAPI_OK){
        STA_LOG(STA_ERR,"Message decoding failed(nRet=%d)\n",nRet);
        return STA_NOK;
    }

    while(1){
        cTableData = cDecApi.GetFirstTableData();
        if(cTableData == NULL){
            break;
        }

        strTableName = cTableData->GetTableName();
        if(strTableName.size() == 0){
            STA_LOG(STA_ERR,"table name not exist\n");
            delete cTableData;
            return STA_NOK;
        }

		DEBUG_LOG("RECEIVE APPLICATION STAT(%s)\n",strTableName.c_str());

        cTable = m_cTableList->SearchTableInfo(strTableName);
        if(cTable == NULL){
            cTable = new CTableInfo(strTableName);
            m_cTableList->InsertTableInfo(cTable);
            blnTableAlloc = true;
        }

        lstPrimaryKey = cTableData->GetPrimaryKey();
        lstValue = cTableData->GetValue();

        collectTime = cTableData->GetCollectTime();

        nRet = cTable->InsertValue(collectTime-m_nCollectTime, collectTime, 
                lstPrimaryKey, lstValue, NULL);	
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Sts message insert failed(nRet=%d)\n",nRet);
            //delete lstPrimaryKey;
            //delete lstValue;
            if(blnTableAlloc == true){
                delete cTable;
            }
            delete cTableData;
            return STA_NOK;
        }
        break;
        delete cTableData;
    }/* end of while(1) */

    return STA_OK;
}

int CAppInfo::ReceiveAppHist(char *a_chMessage, int a_nMessageLen)
{
    int nRet = 0;
    string strTableName;
    list<string> *lstPrimaryKey = NULL;
    list<int> *lstValue = NULL;
    CStaAnsApi cDecApi;
    CTableData *cTableData = NULL;

    nRet = cDecApi.DecodeMessage(a_chMessage, a_nMessageLen);
    if(nRet != STAAPI_OK){
        STA_LOG(STA_ERR,"Message decoding failed(nRet=%d)\n",nRet);
        return STA_NOK;
    }

    while(1){
        cTableData = cDecApi.GetFirstTableData();
        if(cTableData == NULL){
            break;
        }

        int nStmApiType = 0;
        strTableName = cTableData->GetTableName();
        if(strTableName.size() == 0){
            STA_LOG(STA_ERR,"table name not exist\n");
            delete cTableData;
            return STA_NOK;
        }

		DEBUG_LOG("RECEIVE APPLICATION HIST(%s)\n",strTableName.c_str());

        lstPrimaryKey = cTableData->GetPrimaryKey();
        lstValue = cTableData->GetValue();

        nStmApiType = CStmApi::TABLE_TYPE_HIST;

        /* Make Table Info */
        nRet = m_cEmsInfo->DirectSendProcess(strTableName, m_strPackageName, 
                nStmApiType, lstPrimaryKey, lstValue);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Direct send process failed(tableName=%s)\n",
                    strTableName.c_str());
            delete lstPrimaryKey;
            delete lstValue;
            delete cTableData;
            return STA_NOK;
        }
        break;

        delete cTableData;
    }/* end of while(1) */

    return STA_OK;
}
				
int CAppInfo::ReceiveProcess(CProtocol &cProto, int cmdCode)
{
    int nRet = 0;
    std::vector<char> vecPayload;

    cProto.GetPayload(vecPayload);

    if(cmdCode == CMD_STS_APP){
        nRet = ReceiveAppSts(&vecPayload[0],vecPayload.size());
    }
    else if(cmdCode == CMD_STS_APP_HIST){
        nRet = ReceiveAppHist(&vecPayload[0], vecPayload.size());
    }
    else {
        STA_LOG(STA_ERR,"Invalid Command Code(%d)\n",cmdCode);
    }

    if(nRet != STA_OK){
        STA_LOG(STA_ERR,"Application sts Process failed(cmdCode=%d, nRet=%d)\n",cmdCode, nRet);
        return STA_NOK;
    }

    return STA_OK;
}
