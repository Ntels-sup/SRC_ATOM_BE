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
#include "CCmdStatusNodeEvent.hpp"
#include "CCmdNodeStatus.hpp"
#include "CCmdNodeList.hpp"
#include "CCommandNm.hpp"
#include "CNodeProcTB.hpp"
#include "CModuleIPC.hpp"

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
		case CMD_NODE_STATUS :
			return CmdNodeStatus(a_clsProto);
		case CMD_NODE_LIST :
			return CmdNodeList(a_clsProto);
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
		clsResp.SetFlagError();
		a_clsReq.Print(g_pclsLog, LV_INFO, true);
		goto Response_;
	}

	if (clsRegist.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		strBody = clsRegist.ErrorGen(1, "invalied body format");
		clsResp.SetFlagError();
		a_clsReq.Print(g_pclsLog, LV_INFO, true);
		goto Response_;
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
		clsResp.SetFlagError();
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Response_;
	}

	// Regist 정보 등록
	stInfo.m_strPkgName		= clsRegist.m_strPkgName;
	stInfo.m_strNodeType	= clsRegist.m_strNodeType;
	stInfo.m_strNodeProcName= clsRegist.m_strProcName;
	stInfo.m_nNodeProcNo	= clsRegist.m_nProcNo;
	stInfo.m_bIsNode		= false;
	clsNPTab.AddAtomProc(a_pclsCli->GetSocket(), stInfo);

	g_pclsLog->INFO("MSGP, Registed, pkgnm: %s ntype: %s pname: %s procno: %d", 
					clsRegist.m_strPkgName.c_str(),
					clsRegist.m_strNodeType.c_str(),
					clsRegist.m_strProcName.c_str(),
					clsRegist.m_nProcNo);

	clsRoute.PrintTable(g_pclsLog, LV_INFO);

	// 등록 성공 메세지 전송
	strBody = clsRegist.ErrorGen(0, "ok registration");

Response_:
	// 등록 실패 메세지 전송
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

	if (clsResp.IsFlagError()) {
		a_clsServ.ClosePeer(a_pclsCli);
		return false;
	} else {
		// Server에서는 Route로 넘긴 socket을 삭제 해준다.
		// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
		a_clsServ.DelPeer(a_pclsCli->GetSocket());
	}

	return true;
}

bool CCommandNm::CmdRegistNode(CProtocol& a_clsReq, 
							CMesgExchSocketServer& a_clsServ, CSocket* a_pclsCli)
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
		clsResp.SetFlagError();
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Response_;
	}

	if (clsRegist.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		strBody = clsRegist.ErrorGen(1, "invalied body format");
		clsResp.SetFlagError();
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		goto Response_;
	}

	// 수신된 인증 요청 정보를 기준으로 Node, Process 정보 테이블에서 찾는다. 
	nNodeNo = clsDbIO.FindNodeNo(clsRegist.m_strUuid);
	if (nNodeNo > 0) {
		clsRegist.m_nNodeNo		= clsDbIO.m_nNodeNo;
		clsRegist.m_strNodeName	= clsDbIO.m_strNodeName;		
		strBody = clsRegist.ErrorGen(0, "node regist ok");

		clsDbIO.NodeUse(clsRegist.m_strUuid);
		
	} else if (nNodeNo == 0) {					// not found
		nNodeNo = clsDbIO.NodeCreate(clsRegist.m_strPkgName, clsRegist.m_strNodeType,
									clsRegist.m_strUuid, clsRegist.m_strIp);
		if (nNodeNo <= 0) {
			g_pclsLog->WARNING("MSGP, node create failed");
			strBody = clsRegist.ErrorGen(1, "node create failed");
			clsResp.SetFlagError();
			goto Response_;
		}
		clsRegist.m_nNodeNo		= clsDbIO.m_nNodeNo;
		clsRegist.m_strNodeName	= clsDbIO.m_strNodeName;
		
	} else {
		g_pclsLog->WARNING("MSGP, find node failed");
		strBody = clsRegist.ErrorGen(1, "find node failed");
		clsResp.SetFlagError();
		goto Response_;
	}

	// Routing 정보 등록, Agent는 node의 gateway 역할을 수행하므로 default routing(0)
	// 도 추가로 넣는다.
	clsRoute.AddRouteSock(clsRegist.m_nNodeNo, clsRegist.m_nProcNo, a_pclsCli->GetSocket());
	if (clsRoute.AddRouteSock(clsRegist.m_nNodeNo, 0, a_pclsCli->GetSocket()) == false) {
		g_pclsLog->WARNING("MSGP, %s", clsRoute.m_strErrorMsg.c_str());
		strBody = clsRegist.ErrorGen(1, clsRoute.m_strErrorMsg.c_str());
		clsResp.SetFlagError();
		goto Response_;
	}

	// Regist 정보 등록
	stInfo.m_strPkgName		= clsRegist.m_strPkgName;
	stInfo.m_strNodeType	= clsRegist.m_strNodeType;
	stInfo.m_strNodeProcName= clsRegist.m_strNodeName;
	stInfo.m_nNodeProcNo	= clsRegist.m_nNodeNo;
	stInfo.m_strIp			= clsRegist.m_strIp;	
	stInfo.m_bIsNode		= true;
	clsNPTb.AddNode(a_pclsCli->GetSocket(), stInfo);

	AlarmNodeStatus(stInfo, "RUNNING");

	g_pclsLog->INFO("MSGP, node registed");
	g_pclsLog->INFO("      - pkgnm: %s, ntype: %s, pname: %s, nodeno: %d, nname: %s", 
					clsRegist.m_strPkgName.c_str(),
					clsRegist.m_strNodeType.c_str(),
					clsRegist.m_strProcName.c_str(),
					clsRegist.m_nNodeNo,
					clsRegist.m_strNodeName.c_str());

	// 등록 성공 메세지 전송
	strBody = clsRegist.ResponseGen();


Response_:
	// 등록 실패 메세지 전송
	clsResp.SetCommand(CMD_REGIST);
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

	// Server에서는 Route로 넘긴 socket을 삭제 해준다.
	// 넘겨진 socket은 RouteThread에서 메세지 송수신 처리 된다.
	if (clsResp.IsFlagError()) {
		a_clsServ.ClosePeer(a_pclsCli);
		return false;
	} else {
		a_clsServ.DelPeer(a_pclsCli->GetSocket());
	}

	return true;
}

bool CCommandNm::CmdPing(CProtocol& a_clsReq)
{
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

	clsResp.SetResponse(a_clsReq);
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLog->ERROR("CMD, 'PING' send failed");
		return false;
	}

	return true;
}

bool CCommandNm::CmdNodeStatus(CProtocol& a_clsReq)
{
	CCmdNodeStatus clsStatus;

	if (a_clsReq.IsFlagNotify()) {
		g_pclsLog->WARNING("CMD, 'NODE_STATUS' not set notify flag");
	}

	if (clsStatus.NotifyParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, NODE_STATUS invalied body format");
		a_clsReq.Print(g_pclsLog, LV_ERROR, true);
		return false;
	}

	// 노드 상태 update는 NM이 알람으로 알려주면 ALM에서 update 한다.
	if (clsStatus.m_strStatus.compare("SCALEIN") == 0) {
		CDBInOut clsDbIO;
		clsDbIO.NodeUse(clsStatus.m_strUuid, 'N');
	}

	CNodeProcTB::ST_INFO stNode;
	if (CNodeProcTB::Instance().GetNodeByNodeNo(clsStatus.m_nNodeNo, stNode)) {
		AlarmNodeStatus(stNode, clsStatus.m_strStatus.c_str());
	} else {
		g_pclsLog->WARNING("CMD, not found node information");
	}
	
	return true;
}

bool CCommandNm::CmdNodeList(CProtocol& a_clsReq)
{
	CCmdNodeList clsNode;
	CProtocol clsResp;
	CDBInOut clsDbIO;
	CCmdNodeList::ST_RESPONSE stResp;
	vector<CNodeProcTB::ST_INFO> vecNode;
	string strBody;
	int nNode;
	
	clsResp.SetResponse(a_clsReq);

	if (a_clsReq.IsFlagRequest() == false) {
		g_pclsLog->ERROR("CMD, NODE_LIST not set request flag");
		a_clsReq.Print(g_pclsLog, LV_ERROR, false);
		strBody = clsNode.ErrorGen(1, "net set request flag");
		clsResp.SetFlagError();
		goto Response_;
	}

	if (clsNode.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, NODE_LIST invalied body format");
		a_clsReq.Print(g_pclsLog, LV_ERROR, false);
		strBody = clsNode.ErrorGen(1, "invalied body format");
		clsResp.SetFlagError();		
		goto Response_;
	}

	nNode = CNodeProcTB::Instance().FindNode(clsNode.m_strPkgName, 
												clsNode.m_strNodeType, vecNode);
	for (int i=0; i < nNode; i++) {
		stResp.m_strPkgName		= vecNode[i].m_strPkgName;
		stResp.m_strNodeType	= vecNode[i].m_strNodeType;
		stResp.m_nNodeNo		= vecNode[i].m_nNodeProcNo;
		stResp.m_strNodeName	= vecNode[i].m_strNodeProcName;
		stResp.m_strIp			= vecNode[i].m_strIp;
		stResp.m_strVersion		= vecNode[i].m_strNodeVersion;

		clsNode.m_lstNode.push_back(stResp);
	}
	
	strBody = clsNode.ResponseGen();

Response_:
	// 응답 메세지 전송
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLog->ERROR("CMD, 'NODE_LIST' send failed");
		return false;
	}

	#ifdef NM_DEBUG
	g_pclsLog->DEBUG("Send message");
	clsResp.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	return true;
}

bool CCommandNm::AlarmNodeStatus(CNodeProcTB::ST_INFO& a_stNode, const char* a_szStatus)
{
	CConfigNm& clsCfg = CConfigNm::Instance();
	CNodeProcTB& clsNodeProc = CNodeProcTB::Instance();
	

	CCmdStatusNodeEvent clsStatus;
	clsStatus.m_strMessage		= "node status change";
	clsStatus.m_strPkgName		= a_stNode.m_strPkgName;
	clsStatus.m_nNodeNo			= a_stNode.m_nNodeProcNo;
	clsStatus.m_strNodeName		= a_stNode.m_strNodeProcName;
	clsStatus.m_strNodeVersion	= a_stNode.m_strNodeVersion;
	clsStatus.m_strNodeType		= a_stNode.m_strNodeType;
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
	
	if (CModuleIPC::Instance().SendMesg(clsNoti) == false) {
		g_pclsLog->ERROR("CMD, 'STATUS_NODE_EVENT' send failed to ALM");
		return false;
	}
	
	return true;
}

