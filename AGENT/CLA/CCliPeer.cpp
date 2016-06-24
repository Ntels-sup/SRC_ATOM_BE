#include <time.h>

#include "CommandFormat.hpp"
#include "CLA.h"
#include "CGlobal.hpp"
#include "CCliPeer.hpp"
#include "CCliRsp.hpp"
#include "CCliRegReqApi.hpp"
#include "CCliRegRspApi.hpp"
#include "CMesgExchSocketServer.hpp"

CCliPeer::CCliPeer(time_t a_timestamp, CSocket *a_cSock)
{
        m_timestamp = a_timestamp;
        m_cSock = a_cSock;
}

CCliPeer::~CCliPeer()
{
	if(m_cSock != NULL){
		delete m_cSock;
	}
}

CProtocol* CCliPeer::Receive()
{
	CProtocol *cProto = NULL;
	if(m_lstRcvMsgQ.size() == 0){
		return NULL;
	}

	cProto = m_lstRcvMsgQ.front();

	m_lstRcvMsgQ.pop_front();

	return cProto;
}

int CCliPeer::ReceiveProcess()
{
	int nRet = 0;
	string strCmdCode;
	unsigned int nCmdCode = 0;
	CMesgExchSocketServer cSock;
	CProtocol cRcvMsg;

	while(1){
		nRet = cSock.RecvMesg(m_cSock, &cRcvMsg, -1);
		if(nRet < 0){
			CLA_LOG(CLA_ERR,false,"Receive message failed(nRet=%d, sockerr=%s, err=%d(%s))\n",
					nRet, cSock.m_strErrorMsg.c_str(), errno, strerror(errno));
			return CLA_RSLT_PEER_CLOSED;
		}
		else if(nRet == 0){
			return CLA_OK;
		}

		cSock.GetCommand(strCmdCode);

		nCmdCode = CGlobal::GetCmdCode(strCmdCode);

		switch(nCmdCode){
			case CMD_CLI_REG:
				{
					string strPayload;
					CProtocol cProto;
					CCliRegReqApi cRegReq;
					CCliRegRspApi cRegRsp;

					/* check peer */
					strPayload = cRcvMsg.GetPayload();

					nRet = cRegReq.DecodeMessage(strPayload);
					if(nRet != CCliRegApi::RESULT_OK){
						CLA_LOG(CLA_ERR,false,"CLI Reg message decoding failed(nRet=%d, err=%s)\n",
								nRet, cRegReq.m_strErrString.c_str());
						return CLA_RSLT_PEER_CLOSED;
					}

					m_strUserId = cRegReq.GetUserId();

					cRegRsp.Init(CCliRegRspApi::RESULT_CODE_REG_SUCCESS);

					nRet = cRegRsp.EncodeMessage(strPayload);
					if(nRet != CCliRegApi::RESULT_OK){
						CLA_LOG(CLA_ERR,false,"CLI Reg message decoding failed(nRet=%d, err=%s)\n",
								nRet, cRegReq.m_strErrString.c_str());
						return CLA_RSLT_PEER_CLOSED;
					}

					cProto.SetCommand("0000080002");
					cProto.SetFlagResponse();
					cProto.SetSequence(0);
					cProto.SetSource(0, 0);
					cProto.SetDestination(0, 0);
					cProto.SetPayload(strPayload);

					/* send message */
					Send(cProto);

					return CLA_RSLT_PROCESS_END;
				}
				break;
			case CMD_CLI_COMMAND:
				{
					CProtocol *cProto = NULL;

					cProto = new CProtocol(cRcvMsg);
					//fprintf(stderr,"RCV MESSAGE\n");

					m_lstRcvMsgQ.push_back(cProto);
				}
				break;
			default:
				{
					CLA_LOG(CLA_ERR,false,"Invalid CmdCode(code=%s)\n", strCmdCode.c_str());
				}
				break;
		};
	}/* end of while(1) */

	return CLA_OK;
}

int CCliPeer::SendError(unsigned int a_nSessionId, unsigned int a_nResultCode)
{
	CCliRsp cEncRsp;
	string strPayload;
	CProtocol cProto;

	cEncRsp.SetSessionId(a_nSessionId);
	cEncRsp.SetSeqType(CCliRsp::SEQ_TYPE_END);

	switch(a_nResultCode){
		case CLA_RSLT_CODE_TIME_OUT:
			{
				cEncRsp.SetResultCode(a_nResultCode, "CLA Session Timeout");
			}
			break;
		case CLA_RSLT_CODE_SEND_ERROR:
			{
				cEncRsp.SetResultCode(a_nResultCode, "CLI Message Send error");
			}
			break;
		default:
			{
				cEncRsp.SetResultCode(a_nResultCode, "Unkonwon error");
				CLA_LOG(CLA_ERR,false,"Unknown error occure(err=%d)\n",a_nResultCode);
			}
			break;
	};

	cEncRsp.EncodeMessage(strPayload);

	cProto.SetCommand("0000080001");
	cProto.SetFlagResponse();
	cProto.SetSequence(a_nSessionId);
	cProto.SetSource(0, 0);
	cProto.SetDestination(0, 0);
	cProto.SetPayload(strPayload);

	Send(cProto);

	return CLA_OK;
}

int CCliPeer::Send(CProtocol &cProto)
{
	unsigned int unSize = 0;
	vector<char> vecData;

	cProto.GenStream(vecData);

	int dstNode;
	int dstPrc;

	cProto.GetDestination(dstNode, dstPrc);

	unSize = m_cSock->Send((char*)&vecData[0], vecData.size());
	if(unSize != vecData.size()){
		CLA_LOG(CLA_ERR,false,"Socket send failed(size=%d)(err=%d(%s))\n",unSize, errno, strerror(errno));
		return CLA_NOK;
	}

	return CLA_OK;
}
