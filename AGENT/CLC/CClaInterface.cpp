#include <stdio.h>
#include "CCliRegApi.hpp"
#include "CCliRegReqApi.hpp"
#include "CCliRegRspApi.hpp"
#include "CClaInterface.hpp"

int CClaInterface::RegReqFunc(CProtocol &cProto)
{
	CCliRegReqApi cReqApi;
	string strPayload;

	/* send reg message */

	cReqApi.Init(m_strUserId);
	cReqApi.EncodeMessage(strPayload);

	cProto.SetCommand("0000080002");
	cProto.SetFlagRequest();
	cProto.SetSource(0, 0);
	cProto.SetDestination(0, 0);
	cProto.SetPayload(strPayload);

	return RESULT_OK;
}

int CClaInterface::RegRspFunc(CProtocol &cProto)
{
	CCliRegRspApi cRspApi;
	string strPayload;

	strPayload = cProto.GetPayload();

	cRspApi.DecodeMessage(strPayload);

	if(cRspApi.GetCode() != CCliRegRspApi::RESULT_CODE_REG_SUCCESS){
		return RESULT_REG_FAILED;
	}

	return RESULT_OK;
}

