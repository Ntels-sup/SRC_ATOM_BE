/* vim:ts=4:sw=4
 */
/**
 * \file	PRAMain.cpp
 * \brief	APP 프로세스 실행, 정지 등의 제어 담당, 통신 중계
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include <string>

#include "CommandFormat.hpp"
#include "CFileLog.hpp"

#include "CModule.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"
#include "STAAPI.h"
#include "CTableData.hpp"
#include "CStaAnsApi.hpp"
#include "CCliRsp.hpp"
#include "CCliReq.hpp"

using std::list;
using std::string;

int localNode = 0;
int localPrcId = 0;
int dstNode = 0;
int dstPrcId = 0;

CModuleIPC* clsIPC;

int TEST_CLI_DATA_SEND(unsigned int nSessionId, CProtocol &cProto)
{
	char chCmdCode[12];
	bool blnRet = true;
	string payload;
	CCliReq decReq;
	CCliRsp encRsp;
	string strMsg;

	payload = cProto.GetPayload();


	decReq.DecodeMessage(payload);

	encRsp.SetSessionId(nSessionId);

	encRsp.SetResultCode(1,"SUCCESS");
	encRsp.SetText("================\n");
	//encRsp.SetTextAppend("CLI TEST SUCCESS\n");

	for(unsigned int i=0;i<decReq.GetCount();i++){
		encRsp.NPrintf(1024, "%s(%d)\n",decReq[i].GetName().c_str(), decReq[i].GetCount());
		for(unsigned int j=0;j<decReq[i].GetCount();j++){
			if(decReq[i][j].IsNumber() == true){
				encRsp.NPrintf(1024,"NUMBER: (%d)", decReq[i][j].GetNumber());
			}
			else {
				encRsp.NPrintf(1024,"STRING: (\"%s\")", decReq[i][j].GetString().c_str());
			}

		}
		encRsp.NPrintf(1024,"\n");
	}

	encRsp.SetTextAppend("================\n");

	encRsp.EncodeMessage(strMsg);

	cProto.SetSequence(nSessionId);
	cProto.SetSource(localNode, localPrcId);

	/* TEST */
	//cProto.SetDestination(localNode, PROCID_ATOM_NA_CLA);
	fprintf(stderr,"dstNode=%d, dstPrc=%d\n",dstNode, dstPrcId);
	cProto.SetDestination(dstNode, dstPrcId);

	snprintf(chCmdCode, 11,"00000%d",CMD_CLI_COMMAND);
	chCmdCode[11] = '\0';
	cProto.SetCommand(chCmdCode);

	cProto.SetFlagResponse();
	cProto.SetPayload(strMsg);

	fprintf(stderr,"SEND CLI RESPONSE\n");
	blnRet = clsIPC->SendMesg(cProto);
	if(blnRet == false){
		printf("SEND FAILED\n");

		return -1;
	}

	return 0;
}

int TEST_STS_DATA_SEND()
{
	bool blnRet = true;
	time_t tm = time(NULL);
	CProtocol cProto;
	char chCmdCode[12];
	char msgBuff[2096];
	unsigned int msgBuffLen = 0;
	CStaAnsApi encApi;

	cProto.SetSource(localNode, localPrcId);

	/* TEST */
	cProto.SetDestination(localNode, PROCID_ATOM_NA_STA);

	snprintf(chCmdCode, 11,"00000%d",CMD_STS_APP);
	chCmdCode[11] = '\0';
	cProto.SetCommand(chCmdCode);

	cProto.SetFlagResponse();

	encApi["DIAM_STS"].SetTableInfo(STAAPI_TABLE_TYPE_STS, tm);
	encApi["DIAM_STS"].InsertPrimaryKey("PEER1");
	encApi["DIAM_STS"].InsertValue(10);
	encApi["DIAM_STS"].InsertValue(20);

#if 0
	encApi["DIAM_STS"].SetTableInfo(STAAPI_TABLE_TYPE_STS, tm);
	encApi["DIAM_STS"].InsertPrimaryKey("PEER2");
	encApi["DIAM_STS"].InsertValue(30);
	encApi["DIAM_STS"].InsertValue(40);
#endif

	encApi.EncodeMessage(msgBuff, 2096, &msgBuffLen);


	cProto.SetPayload(msgBuff, msgBuffLen);


	blnRet = clsIPC->SendMesg(cProto);
	if(blnRet == false){
		printf("SEND FAILED\n");

		return -1;
	}

	return 0;
}

int GetCmdCode(std::string &a_strCmdCode){
	if(a_strCmdCode.size() < 10){
		return 0;
	}

	return atoi(&a_strCmdCode.c_str()[4]);
};




void* PRAStart(void* pArg)
{
	CModule::ST_MODULE_OPTIONS stOption = *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);
	//stOption.m_szPkgName;
	//stOption.m_szNodeType;
	//stOption.m_szProcName;
	//stOption.m_nNodeNo;
	//stOption.m_nProcNo;
	//stOption.m_pclsModIpc;
	//stOption.m_pclsAddress;

	//CModuleIPC* clsIPC = stOption.m_pclsModIpc;
	clsIPC = &CModuleIPC::Instance();
	//CAddress*	clsAddr = stOption.m_pclsAddress;
	CAddress*	clsAddr = &CAddress::Instance();

	string strLogPath = "/UDBS/ATOM/LOG";

	localNode = stOption.m_nNodeNo;
	localPrcId = stOption.m_nProcNo;


	printf("NA PRA STS TEST module starting.\n");

	CProtocol clsProto;
	list<CAddress::ST_ADDR> lstAddr;
	int nNodeId, nProcId;
	int nSeq;

	while (true) {
		if (clsIPC->RecvMesg(stOption.m_nProcNo, clsProto, 5) > 0) {		// time 5sec

			printf("RECEIVE MESSAGE\n");
			string strCmdCode;
			unsigned int cmdCode;

			strCmdCode = clsProto.GetCommand();

			cmdCode = GetCmdCode(strCmdCode);

			clsProto.GetSource(dstNode, dstPrcId);
			switch(cmdCode){
				case CMD_CLI_COMMAND:

					TEST_CLI_DATA_SEND(clsProto.GetSequence(), clsProto);
					break;
				case CMD_STS_APP:
					TEST_STS_DATA_SEND();
					break;
			}

		}

	}

	return NULL;
}

// plugin 처리를 위한 handler 등록
#include "CModule.hpp"

MODULE_OP = {
	"ATOM_NA_PRA",		// process name
	true,				// thread running
	NULL,				// Init
	PRAStart,			// Process
	NULL				// CleanUp
};
