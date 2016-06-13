/* vim:ts=4:sw=4
 */
/**
 * \file	???.cpp
 * \brief	???
 *			???
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "CommandFormat.hpp"
#include "CFileLog.hpp"
#include "CProtocol.hpp"
#include "CSocket.hpp"
#include "CMesgExchSocketServer.hpp"

#include "CConfigNm.hpp"
#include "CMesgRoute.hpp"
#include "CDBInOut.hpp"
#include "CCmdRegist.hpp"
#include "CCmdRegistNode.hpp"
#include "CCmdNodeStatus.hpp"
#include "CCommandNm.hpp"
#include "CNodeProcTB.hpp"

using std::string;
using std::vector;

extern CFileLog* g_pclsLog;


bool CCommandNm::MesgProcess(CProtocol& a_clsProto, CMesgExchSocketServer& a_clsServ,
															CSocket* a_pclsCli)
{
	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Receive Message");
	a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	uint32_t nCmd = atoi(a_clsProto.GetCommand().c_str());
	switch (nCmd) {
		case CMD_REGIST :
			return CmdRegist(a_clsProto, a_clsServ, a_pclsCli);
		case CMD_REGIST_NODE :
			return CmdRegistNode(a_clsProto, a_clsServ, a_pclsCli);
		case CMD_PING :
			return CmdPing(a_clsProto);
		default :
			g_pclsLog->ERROR("MSGP, unknown command: %d", nCmd);
			return false;
	}

	return false;
}

bool CCommandNm::CmdRegist(CProtocol& a_clsReq, CMesgExchSocketServer& a_clsServ,
															CSocket* a_pclsCli)
{
	CConfigNm& clsCfg = CConfigNm::Instance();
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	CNodeProcTB& clsNPTab = CNodeProcTB::Instance();
	CProtocol clsResp;
	CCmdRegist clsRegist;
	CNodeProcTB::ST_INFO stInfo;
	vector<char> vecMesg;
	string strBody;


	if (a_clsReq.IsFlagRequest() == false) {
		strBody = clsRegist.ErrorGen(1, "not set request flag");
		a_clsReq.Print(g_pclsLog, LV_INFO, true);
		goto Failed;
	}

	if (clsRegist.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		strBody = clsRegist.ErrorGen(1, "invalied body format");
		a_clsReq.Print(g_pclsLog, LV_INFO, true);
		goto Failed;
	}

	// 수신된 인증 요청 정보를 기준으로 Node, Process 정보 테이블에서 찾는다. 
//TODO
/*
	CNodeProcTB& clsTB = CNodeProcTB::Instance();
	if (clsTB.Find(clsRegist.m_strPkgName, clsRegist.m_strProcName, 
						clsRegist.m_nNodeNo, clsRegist.m_nProcNo) == false) {
		strBody = clsRegist.ErrorGen(1, "failed auth");
		goto Failed;
	}
*/
	// Routing 정보 등록
	if (clsRoute.AddRouteSock(clsCfg.m_nNodeNo, clsRegist.m_nProcNo, 
											a_pclsCli->GetSocket()) == false) {
		strBody = clsRegist.ErrorGen(1, clsRoute.m_strErrorMsg.c_str());
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Failed;
	}

	// Regist 정보 등록
	stInfo.m_strPkgName		= clsRegist.m_strPkgName;
	stInfo.m_strNodeType	= clsRegist.m_strNodeType;
	stInfo.m_strNodeProcName= clsRegist.m_strProcName;
	stInfo.m_nNodeProcNo	= clsRegist.m_nProcNo;
	clsNPTab.AddAtomProc(a_pclsCli->GetSocket(), stInfo);

	g_pclsLog->INFO("MSGP, Registed, pkgnm: %s ntype: %s pname: %s procno: %d", 
					clsRegist.m_strPkgName.c_str(),
					clsRegist.m_strNodeType.c_str(),
					clsRegist.m_strProcName.c_str(),
					clsRegist.m_nProcNo);

	#ifdef NM_DEBUG
	clsRoute.PrintTable(g_pclsLog, LV_DEBUG);
	#endif

	// 등록 성공 메세지 전송
	strBody = clsRegist.ErrorGen(0, "ok registration");

	clsResp.SetCommand(CMD_REGIST);
	clsResp.SetFlagResponse();
	clsResp.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsResp.SetDestination(a_clsReq.GetSource());
	clsResp.SetSequence(a_clsReq.GetSequence());
	clsResp.SetPayload(strBody);
	clsResp.GenStream(vecMesg);
	
	a_pclsCli->Send(&vecMesg[0], vecMesg.size());

	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Send Message");
	clsResp.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	// Server에서는 Route로 넘긴 socket을 삭제 해준다.
	// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
	a_clsServ.DelPeer(a_pclsCli->GetSocket());

	return true;


Failed:
	// 등록 실패 메세지 전송
	clsResp.SetCommand(CMD_REGIST);
	clsResp.SetFlagResponse();
	clsResp.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsResp.SetDestination(a_clsReq.GetSource());
	clsResp.SetSequence(a_clsReq.GetSequence());
	clsResp.SetFlagError();
	clsResp.SetPayload(strBody);
	clsResp.GenStream(vecMesg);

	a_pclsCli->Send(&vecMesg[0], vecMesg.size());

	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Send Message");
	clsResp.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	// Server에서는 Route로 넘긴 socket을 삭제 해준다.
	// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
	a_clsServ.ClosePeer(a_pclsCli);

	return false;
}

bool CCommandNm::CmdRegistNode(CProtocol& a_clsReq, CMesgExchSocketServer& a_clsServ,
															CSocket* a_pclsCli)
{
	CConfigNm& clsCfg = CConfigNm::Instance();
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	CNodeProcTB& clsNPTb = CNodeProcTB::Instance();
	CDBInOut clsDbIO;
	CProtocol clsResp;
	CCmdRegistNode clsRegist;
	CNodeProcTB::ST_INFO stInfo;
	vector<char> vecMesg;
	string strBody;
	int nNodeNo = -1;


	if (a_clsReq.IsFlagRequest() == false) {
		strBody = clsRegist.ErrorGen(1, "not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Failed;
	}

	if (clsRegist.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		strBody = clsRegist.ErrorGen(1, "invalied body format");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Failed;
	}

	// 수신된 인증 요청 정보를 기준으로 Node, Process 정보 테이블에서 찾는다. 
	nNodeNo = clsDbIO.FindNodeNo(clsRegist.m_strUuid);
	if (nNodeNo > 0) {
		clsRegist.m_nNodeNo		= clsDbIO.m_nNodeNo;
		clsRegist.m_strNodeName	= clsDbIO.m_strNodeName;		
		strBody = clsRegist.ErrorGen(0, "node regist ok");
		
	} else if (nNodeNo == 0) {					// not found
		nNodeNo = clsDbIO.NodeCreate(clsRegist.m_strPkgName, clsRegist.m_strNodeType,
									clsRegist.m_strUuid, clsRegist.m_strIp);
		if (nNodeNo <= 0) {
			g_pclsLog->WARNING("MSGP, node create failed");
			strBody = clsRegist.ErrorGen(1, "node create failed");
			goto Failed;
		}
		clsRegist.m_nNodeNo		= clsDbIO.m_nNodeNo;
		clsRegist.m_strNodeName	= clsDbIO.m_strNodeName;
		
	} else {
		g_pclsLog->WARNING("MSGP, find node failed");
		strBody = clsRegist.ErrorGen(1, "find node failed");
		goto Failed;
	}

	// Routing 정보 등록, Agent는 node의 gateway 역할을 수행하므로 default routing(0)
	// 도 추가로 넣는다.
	clsRoute.AddRouteSock(clsRegist.m_nNodeNo, clsRegist.m_nProcNo, a_pclsCli->GetSocket());
	if (clsRoute.AddRouteSock(clsRegist.m_nNodeNo, 0, a_pclsCli->GetSocket()) == false) {
		g_pclsLog->WARNING("MSGP, %s", clsRoute.m_strErrorMsg.c_str());
		strBody = clsRegist.ErrorGen(1, clsRoute.m_strErrorMsg.c_str());
		goto Failed;
	}

	// Regist 정보 등록
	stInfo.m_strPkgName		= clsRegist.m_strPkgName;
	stInfo.m_strNodeType	= clsRegist.m_strNodeType;
	stInfo.m_strNodeProcName= clsRegist.m_strNodeName;
	stInfo.m_nNodeProcNo	= clsRegist.m_nNodeNo;
	stInfo.m_strIp			= clsRegist.m_strIp;	
	clsNPTb.AddNode(a_pclsCli->GetSocket(), stInfo);

	g_pclsLog->INFO("MSGP, node registed");
	g_pclsLog->INFO("      - pkgnm: %s, ntype: %s, pname: %s, nodeno: %d, nname: %s", 
					clsRegist.m_strPkgName.c_str(),
					clsRegist.m_strNodeType.c_str(),
					clsRegist.m_strProcName.c_str(),
					clsRegist.m_nNodeNo,
					clsRegist.m_strNodeName.c_str());
					
	// 등록 성공 메세지 전송
	strBody = clsRegist.ResponseGen();

	clsResp.SetCommand(CMD_REGIST_NODE);
	clsResp.SetFlagResponse();
	clsResp.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsResp.SetDestination(a_clsReq.GetSource());
	clsResp.SetSequence(a_clsReq.GetSequence());
	clsResp.SetPayload(strBody);
	clsResp.GenStream(vecMesg);
	
	a_pclsCli->Send(&vecMesg[0], vecMesg.size());

	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Send message");
	clsResp.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	// Node 상태 alarm 전송
	AlarmNodeStatus(a_pclsCli->GetSocket(), "ACTIVE");

	// Server에서는 Route로 넘긴 socket을 삭제 해준다.
	// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
	a_clsServ.DelPeer(a_pclsCli->GetSocket());

	return true;


Failed:
	// 등록 실패 메세지 전송
	clsResp.SetCommand(CMD_REGIST);
	clsResp.SetFlagResponse();
	clsResp.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsResp.SetDestination(a_clsReq.GetSource());
	clsResp.SetSequence(a_clsReq.GetSequence());
	clsResp.SetFlagError();
	clsResp.SetPayload(strBody);
	clsResp.GenStream(vecMesg);

	a_pclsCli->Send(&vecMesg[0], vecMesg.size());

	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Send message");
	clsResp.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	// Server에서는 Route로 넘긴 socket을 삭제 해준다.
	// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
	a_clsServ.ClosePeer(a_pclsCli);

	return false;
}

bool CCommandNm::CmdPing(CProtocol& a_clsReq)
{
	CConfigNm& clsCfg = CConfigNm::Instance();
	CProtocol clsResp;
	CCmdBase clsBase;
	string strBody;
	vector<char> vecMesg;

	if (a_clsReq.IsFlagRequest()) {
		strBody = clsBase.ErrorGen(0, "NM alive");
	} else {
		strBody = clsBase.ErrorGen(1, "net set request flag");
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

	if (CModuleIPC::Instance().SendMesg(CModuleIPC::TH_ROUTING, clsResp) == false) {
		g_pclsLog->ERROR("CMD, 'PING' send failed");
		return false;
	}

	return true;
}

bool CCommandNm::AlarmNodeStatus(int a_nId, const char* a_szStatus)
{
	CConfigNm& clsCfg = CConfigNm::Instance();
	CNodeProcTB& clsNodeProc = CNodeProcTB::Instance();
	
	CNodeProcTB::ST_INFO stNode;
	if (clsNodeProc.GetNode(a_nId, stNode) == 0) {
		// Node가 아닌 ATOM server process 다.
		return true;
	}

	CCmdNodeStatus clsStatus;
	clsStatus.m_strMessage		= "node status change";
	clsStatus.m_strPkgName		= stNode.m_strPkgName;
	clsStatus.m_nNodeNo			= stNode.m_nNodeProcNo;
	clsStatus.m_strNodeName		= stNode.m_strNodeProcName;
	clsStatus.m_strNodeVersion	= stNode.m_strNodeVersion;
	clsStatus.m_strNodeType		= stNode.m_strNodeType;
	clsStatus.m_strNodeStatus	= a_szStatus;

	// Alarm Manager process no을 찾는다.
	// Regist 과정에서 ALM이 등록한 정보에서 process no을 찾는다.
	int nProcNo_ALM = clsNodeProc.GetAtomProcNo("ALM");
	if (nProcNo_ALM <= 0) {
		g_pclsLog->ERROR("not found ALM process no, node '%s' send failed", a_szStatus);
		return false;
	}

	CProtocol clsNoti;
	clsNoti.SetCommand(CMD_STATUS_NODE_EVENT);
	clsNoti.SetFlagNotify();
	clsNoti.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsNoti.SetDestination(clsCfg.m_nNodeNo, nProcNo_ALM);
	clsNoti.SetSequence();
	clsNoti.SetPayload(clsStatus.NotifyGen());
	
	if (CModuleIPC::Instance().SendMesg(CModuleIPC::TH_ROUTING, clsNoti) == false) {
		g_pclsLog->ERROR("CMD, 'STATUS_NODE_EVENT' send failed to ALM");
		return false;
	}
	
	return true;
}

