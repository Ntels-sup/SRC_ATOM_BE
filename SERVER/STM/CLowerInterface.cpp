#include <stdio.h>
#include "STM.h"
#include "CGlobal.hpp"
#include "CRegApi.hpp"
#include "CRegReqApi.hpp"
#include "CRegRespApi.hpp"
#include "CNMInterface.hpp"
#include "CLowerInterface.hpp"

int CLowerInterface::RegReqFunc(CProtocol &cProto)
{
    CGlobal *cGlob = NULL;
    CRegReqApi cReqApi;
    string strPayload;

    cGlob = CGlobal::GetInstance();

    /* send reg message */
    cReqApi.Init((char*)"ATOM",(char*)"EMS", (char*)"STM",cGlob->GetLocalProcNo());
    cReqApi.EncodeMessage(strPayload);

    cProto.SetCommand("0000000001");
    cProto.SetFlagRequest();
    cProto.SetSource(cGlob->GetLocalNodeNo(), cGlob->GetLocalProcNo());
    cProto.SetDestination(cGlob->GetLocalNodeNo(), m_nNmProcNo);
    cProto.SetPayload(strPayload);

    return RESULT_OK;
}

int CLowerInterface::RegRspFunc(CProtocol &cProto)
{
    int nRet = 0;
    std::string strPayload;
    CRegRespApi cRespApi;

    strPayload = cProto.GetPayload();

    nRet = cRespApi.DecodeMessage(strPayload);
    if(nRet != CRegApi::OK){
        STM_LOG(STM_ERR,"Reg response message decodeing failed(nRet=%d, err=%s)\n(Payload = %s\n",
                nRet, cRespApi.m_strErrString.c_str(), strPayload.c_str());
        return RESULT_REG_FAILED;
    }

    if(cRespApi.GetCode() != 0){
        STM_LOG(STM_ERR,"REG FAILED((cmdCode=%d, text=%s)\n",
                cRespApi.GetCode(), cRespApi.GetText().c_str());
        return RESULT_REG_FAILED;
    }

    STM_LOG(STM_ERR,"REG SUCCESS(text=%s)\n", cRespApi.GetText().c_str());

    return RESULT_OK;
}

