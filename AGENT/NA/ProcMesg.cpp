
/* vim:ts=4:sw=4
 */
/**
 * \file	ProcMesg.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include "CProtocol.hpp"
#include "CMesgExchSocket.hpp"
#include "CommandFormat.hpp"

#include "CConfigNa.hpp"
#include "CMesgRoute.hpp"
#include "CModuleIPC.hpp"
#include "CCmdBase.hpp"

using std::string;

// function prototype
void ProcCommand(CProtocol& CProtocol, CMesgExchSocket* a_pclsExSock);
void CmdPing(CProtocol& a_clsProto, CMesgExchSocket* a_pclsExSock);

extern CFileLog* g_pclsLog;


void ProcMesgSock(CProtocol& a_clsProto, CMesgExchSocket& a_clsMsgEx)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

	#ifdef NA_DEBUG
	g_pclsLog->DEBUG("Recvive Socket message");
	a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
	#endif
	
	int nDstNode = -1;
	int nDstProc = -1; 
	a_clsProto.GetDestination(nDstNode, nDstProc);

	// local nodeno 와 다르면 오류
	if (nDstNode != clsCfg.m_nNodeNo) {
		g_pclsLog->ERROR("PRC, not my nodeno");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return;
	}
	// NA가 처리해 할 명령인가?
	if (nDstNode == clsCfg.m_nNodeNo && nDstProc == clsCfg.m_nProcNo) {
		ProcCommand(a_clsProto, &a_clsMsgEx);
		return;
	}

	// 명령을 전송할 agent를 찾기 위해 agent 경로를 찾는다. 
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	int	nToProc = -1;
	if (clsRoute.GetRoute(nDstNode, nDstProc, nToProc) == false) {
		g_pclsLog->ERROR("PRC, routing failed");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return;
	}
	
	// agent에 명령 전송
	CModuleIPC& clsIPC = CModuleIPC::Instance();
	if (clsIPC.SendMesg(a_clsProto) == false) {
		g_pclsLog->ERROR("PRC, mode ipc send failed");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return;
	}
	
	return;
}

void ProcMesgThread(CProtocol& clsProto, CMesgExchSocket& a_clsMsgEx)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

	#ifdef NA_DEBUG
	g_pclsLog->DEBUG("Receive Module message");
	clsProto.Print(g_pclsLog, LV_DEBUG, true);
	#endif
	
	int nDstNode = -1;
	int nDstProc = -1; 
	clsProto.GetDestination(nDstNode, nDstProc);

	// NA가 처리해 할 명령인가?
	if (nDstNode == clsCfg.m_nNodeNo && nDstProc == clsCfg.m_nProcNo) {
		ProcCommand(clsProto, NULL);
		return;
	}

	// local nodeno 와 다르면 NM으로 전송
	if (nDstNode != clsCfg.m_nNodeNo) {
		if (a_clsMsgEx.SendMesg(clsProto) == false) {
			g_pclsLog->ERROR("PRC, messag send failed to NM, Error %d:%s",
							a_clsMsgEx.CSocket::m_enError, 
							a_clsMsgEx.CSocket::m_strErrorMsg.c_str());
		}
		return;
	}

	g_pclsLog->ERROR("unknow route message");
	clsProto.Print(g_pclsLog, LV_DEBUG, true);

	return;
}

void ProcCommand(CProtocol& a_clsProto, CMesgExchSocket* a_pclsExSock)
{
	int nCmd = atoi(a_clsProto.GetCommand().c_str());

	switch (nCmd) {
		case CMD_PING :
			CmdPing(a_clsProto, a_pclsExSock);
			break;
		default :
			g_pclsLog->ERROR("PRC, not support command");
			a_clsProto.Print(g_pclsLog, LV_INFO);
			return;
	}

	return;
}

void CmdPing(CProtocol& a_clsReq, CMesgExchSocket* a_pclsExSock)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CProtocol clsResp;
	CCmdBase clsBase;
	string strBody;

	if (a_clsReq.IsFlagRequest()) {
		strBody = clsBase.ErrorGen(0, "ok i am alive");
	} else {
		strBody = clsBase.ErrorGen(0, "not set requst flag");
		clsResp.SetFlagError();

		g_pclsLog->ERROR("MSGP, ping, not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
	}

	clsResp.SetCommand(CMD_PING);
	clsResp.SetFlagResponse();
	clsResp.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsResp.SetDestination(a_clsReq.GetSource());
	clsResp.SetSequence(a_clsReq.GetSequence());
	clsResp.SetPayload(strBody);

	a_pclsExSock->SendMesg(clsResp);

	return;
}
