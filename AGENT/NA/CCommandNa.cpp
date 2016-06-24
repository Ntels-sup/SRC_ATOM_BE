
/* vim:ts=4:sw=4
 */
/**
 * \file	CCommandNa.cpp
 * \brief	
 *
 * $Author: junls $
 * $Date: $
 * $Id: $
 */

#include "CommandFormat.hpp"

#include "CConfigNa.hpp"
#include "CMesgRoute.hpp"
#include "CAddress.hpp"
#include "CModuleIPC.hpp"
#include "CModuleManager.hpp"
#include "CPkgManage.hpp"
#include "CCmdBase.hpp"
#include "CCommandNa.hpp"
#include "CCmdVnfReady.hpp"
#include "CCmdVnfInstall.hpp"
#include "CCmdVnfStart.hpp"
#include "CCmdVnfStop.hpp"
#include "CCmdAppStart.hpp"
#include "CCmdNodeStatus.hpp"

using std::string;

extern CFileLog* g_pclsLog;


bool CCommandNa::ProcMesgSock(CProtocol& a_clsProto, CMesgExchSocket* a_pclsMsgEx)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

	#ifdef NA_DEBUG
	g_pclsLog->DEBUG("Recvive Socket message");
	a_clsProto.Print(g_pclsLog, LV_DEBUG, true);
	#endif
	
	int nDstNode = 0;
	int nDstProc = 0; 
	a_clsProto.GetDestination(nDstNode, nDstProc);

	// local nodeno 와 다르면 오류
	if (nDstNode != clsCfg.m_nNodeNo) {
		g_pclsLog->ERROR("CMD, not my nodeno");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return false;
	}
	// NA가 처리해 할 명령인가?
	if (nDstNode == clsCfg.m_nNodeNo && nDstProc == clsCfg.m_nProcNo) {
		return Command(a_clsProto, a_pclsMsgEx);
	}

	// 명령을 전송할 agent를 찾기 위해 agent 경로를 찾는다. 
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	int	nToProc = 0;
	if (clsRoute.Lookup(nDstNode, nDstProc, nToProc) == false) {
		g_pclsLog->ERROR("CMD, routing failed");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return false;
	}
	
	// agent에 명령 전송
	CModuleIPC& clsIPC = CModuleIPC::Instance();
	if (clsIPC.SendMesg(a_clsProto) == false) {
		g_pclsLog->ERROR("CMD, mode ipc send failed");
		a_clsProto.Print(g_pclsLog, LV_INFO);
		return false;
	}
	
	return true;
}

bool CCommandNa::ProcMesgModule(CProtocol& clsProto, CMesgExchSocket* a_pclsMsgEx)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

	#ifdef NA_DEBUG
	g_pclsLog->DEBUG("Receive Module message");
	clsProto.Print(g_pclsLog, LV_DEBUG, true);
	#endif
	
	int nDstNode = 0;
	int nDstProc = 0; 
	clsProto.GetDestination(nDstNode, nDstProc);

	// NA가 처리해 할 명령인가?
	if (nDstNode == 0 || nDstNode == clsCfg.m_nNodeNo) {
		if (nDstProc == clsCfg.m_nProcNo) {
			return Command(clsProto, NULL);
		} else {
			g_pclsLog->ERROR("CMD, unknow route message");
			clsProto.Print(g_pclsLog, LV_DEBUG, true);
			return false;
		}
	}

	// local nodeno 와 다르면 NM으로 전송
	// NA 초기 기동시 ATOM 연결 안되어 nodeno 0을 부여하여 모듈을 실행한다.
	// 이후 ATOM 연결 되어 nodeno을 부여 받았을 경우 nodeno가 0이 아니다.
	// Source nodeno 를 확인하여 0이면 재설정 해준다. procno는 재설정 없다.
	// 
	int nNodeNo = 0, nProcNo = 0;
	clsProto.GetSource(nNodeNo, nProcNo);
	if (nNodeNo == 0) {
		clsProto.SetSource(clsCfg.m_nNodeNo, nProcNo);
	}
	if (a_pclsMsgEx->SendMesg(clsProto) == false) {
		g_pclsLog->ERROR("CMD, messag send failed to NM, Error %d:%s",
							a_pclsMsgEx->CSocket::m_enError, 
							a_pclsMsgEx->CSocket::m_strErrorMsg.c_str());
		return false;
	}

	return true;
}

bool CCommandNa::Command(CProtocol& a_clsProto, CMesgExchSocket* a_pclsExSock)
{
	m_nLastCmd = atoi(a_clsProto.GetCommand().c_str());

	if (!m_setPermitCmd.empty()) {
		if (m_setPermitCmd.find(m_nLastCmd) == m_setPermitCmd.end()) {
			g_pclsLog->ERROR("CMD, not permit command");
			a_clsProto.Print(g_pclsLog, LV_INFO);
			m_nLastCmd = 0;
			
			return false;
		}
	}

	switch (m_nLastCmd) {
		case CMD_PING :
			return CmdPing(a_clsProto, a_pclsExSock);
		case CMD_VNF_PRA_READY :
			return CmdVnfReady(a_clsProto);
		case CMD_VNF_PRA_INSTALL :
			return CmdVnfInstall(a_clsProto);
		case CMD_VNF_PRA_START :
			return CmdVnfStart(a_clsProto);
		case CMD_VNF_PRA_STOP :
			return CmdVnfStop(a_clsProto);
		default :
			g_pclsLog->ERROR("CMD, not support command");
			a_clsProto.Print(g_pclsLog, LV_INFO);
	}

	return false;
}

bool CCommandNa::CmdPing(CProtocol& a_clsReq, CMesgExchSocket* a_pclsExSock)
{
	CProtocol clsResp;
	CCmdBase clsBase;
	string strBody;
	bool bIsError = false;

	if (a_clsReq.IsFlagRequest()) {
		strBody = clsBase.ErrorGen(0, "ok i am alive");
	} else {
		strBody = clsBase.ErrorGen(1, "not set requst flag");
		bIsError = true;

		g_pclsLog->ERROR("MSGP, ping, not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
	}

	clsResp.SetResponse(a_clsReq);
	if (bIsError) {
		clsResp.SetFlagError();
	}
	clsResp.SetPayload(strBody);

	a_pclsExSock->SendMesg(clsResp);

	if (bIsError) {
		return false;
	}
	return true;
}

bool CCommandNa::CmdVnfReady(CProtocol& a_clsReq)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CCmdVnfReady clsVnf;
	CProtocol clsResp;
	string strBody;
	bool bIsError = false;

	if (a_clsReq.IsFlagRequest() == false) {
		g_pclsLog->ERROR("CMD, 'VNF_READY', not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);

		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "net set request flag";
		bIsError = true;
		goto Response_;
	}

	if (clsVnf.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, 'VNF_READY', invalied body format");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "invalied body format";
		bIsError = true;
		goto Response_;
	}
	
	clsCfg.m_strPkgName	 = clsVnf.m_strPkgName;
	clsCfg.m_strNodeType = clsVnf.m_strNodeType;
	clsCfg.m_strNodeIp	 = clsVnf.m_strIp;

	// TODO
	// 아래 Q 생성과, Module 실행을 여기서 하는게 맞는가?
	// VNF 환경이 아닐 경우 Main에서 또 해야 하는데...
	if (CModuleManager::Instance().ActiveModule("ATOM_NA_PRA") == false) {
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "Process Manager invoke failed";
		bIsError = true;
		goto Response_;
	}

	clsVnf.m_bResult = true;
	clsVnf.m_strReason = " ";

Response_:
	clsResp.SetResponse(a_clsReq);
	clsResp.SetPayload(clsVnf.ResponseGen());

	CModuleIPC::Instance().SendMesg(clsResp);

	if (bIsError) {
		return false;
	}
	return true;
}

bool CCommandNa::CmdVnfInstall(CProtocol& a_clsReq)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CAddress& clsAddr = CAddress::Instance();
	CPkgManage clsPkg(clsCfg.m_strPkgMngPath.c_str());
	CCmdVnfInstall clsVnf;
	CCmdAppStart clsApp;
	CProtocol clsResp;
	CProtocol clsProto;	
	string strBody;
	bool bIsError = false;
	int nNodeNo = 0; 
	int nProcNo = 0;

	if (a_clsReq.IsFlagRequest() == false) {
		g_pclsLog->ERROR("CMD, 'PKG_INSTALL', not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);

		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "net set request flag";
		bIsError = true;
		goto Response_;
	}

	if (clsVnf.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, 'PKG_INSTALL', invalied body format");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "invalied body format";
		bIsError = true;
		goto Response_;
	}

	clsCfg.m_strUuid	 = clsVnf.m_strUuid;

	g_pclsLog->INFO("CMD, PKG Download, URL: %s", clsVnf.m_strDownPath.c_str());

	// 패키지 Download & Install
	if (clsPkg.Fetch(clsVnf.m_strDownPath, clsVnf.m_strChecksum, "atom", "atom") == false) {
		g_pclsLog->CRITICAL("CMD, %s", clsPkg.m_strErrorMsg.c_str());
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = clsPkg.m_strErrorMsg;
		bIsError = true;
		goto Response_;
	}
	if (clsPkg.UnPack() == false) {
		g_pclsLog->CRITICAL("CMD, %s", clsPkg.m_strErrorMsg.c_str());
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = clsPkg.m_strErrorMsg;
		bIsError = true;
		goto Response_;
	}
	if (clsPkg.Install() == false) {
		g_pclsLog->CRITICAL("CMD, %s", clsPkg.m_strErrorMsg.c_str());
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = clsPkg.m_strErrorMsg;
		bIsError = true;
		goto Response_;
	}
	
	// PRA 에 프로세스 start 명령 전송
	clsApp.m_bAll = true;
	clsAddr.LookupAtom("ATOM_NA_PRA", nNodeNo, nProcNo);
	
	clsProto.SetCommand(CMD_APP_START);
	clsProto.SetFlagNotify();
	clsProto.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsProto.SetDestination(nNodeNo, nProcNo);
	clsProto.SetSequence();
	clsProto.SetPayload(clsApp.RequestGen());
	CModuleIPC::Instance().SendMesg(clsProto);

	bIsError = false;
/*
	if (clsIPC.RecvMesg(clsCfg.m_nProcNo, clsProto, 5) > 0) {
		nCmd = atoi(clsProto.GetCommand().c_str());
		if (nCmd == CMD_APP_START) {
			if (clsApp.ResponseParse(clsProto.GetPayload().c_str())) {
				if (clsApp.m_strWorstStatus.compare("RUNNING") == 0) {
					bIsError = false;
				} else {
					g_pclsLog->ERROR("porcess running failed");
				}
			} else {
				g_pclsLog->ERROR("start failed");
			}
		} else {
		    g_pclsLog->ERROR("unknown command: %d", nCmd);
		}
	} else {
		g_pclsLog->ERROR("PRA wait timeout, START response");
	}
*/
	
	if (bIsError) {
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "install failed";
	} else {
		clsVnf.m_bResult = true;
		clsVnf.m_strReason = " ";
	}

Response_:
	clsResp.SetResponse(a_clsReq);
	clsResp.SetPayload(clsVnf.ResponseGen());

	CModuleIPC::Instance().SendMesg(clsResp);

	if (bIsError) {
		return false;
	}
	return true;
}

bool CCommandNa::CmdVnfStart(CProtocol& a_clsReq)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CAddress& clsAddr = CAddress::Instance();
	CCmdVnfStart clsVnf;
	CCmdAppStart clsApp;
	CProtocol clsResp;
	CProtocol clsProto;
	string strBody;
	bool bIsError = false;
	int nNodeNo = 0;
   	int nProcNo = 0;
	
	
	if (a_clsReq.IsFlagRequest() == false) {
		g_pclsLog->ERROR("CMD, 'VNF_START', not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "net set request flag";
		bIsError = true;
		goto Response_;
	}

	if (clsVnf.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, 'VNF_START', invalied body format");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "invalied body format";
		bIsError = true;
		goto Response_;
	}

	clsCfg.m_strUuid	 = clsVnf.m_strUuid;

	// PRA 에 프로세스 start 명령 전송
	// TODO
	// - 로직 개선이 필요, 비동기 메세제 구조와 맞지 않다.
	clsApp.m_bAll = true;
	clsAddr.LookupAtom("ATOM_NA_PRA", nNodeNo, nProcNo);
	
	clsProto.SetCommand(CMD_APP_START);
	clsProto.SetFlagNotify();
	clsProto.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsProto.SetDestination(nNodeNo, nProcNo);
	clsProto.SetSequence();
	clsProto.SetPayload(clsApp.RequestGen());
	CModuleIPC::Instance().SendMesg(clsProto);

	bIsError = false;
/*
	if (clsIPC.RecvMesg(clsCfg.m_nProcNo, clsProto, 5) > 0) {
		nCmd = atoi(clsProto.GetCommand().c_str());
		if (nCmd == CMD_APP_START) {
			if (clsApp.ResponseParse(clsProto.GetPayload().c_str())) {
				if (clsApp.m_strWorstStatus.compare("RUNNING") == 0) {
					bIsError = false;
				} else {
					g_pclsLog->ERROR("porcess running failed");
				}
			} else {
				g_pclsLog->ERROR("start failed");
			}
		} else {
		    g_pclsLog->ERROR("unknown command: %d", nCmd);
		}
	} else {
		g_pclsLog->ERROR("PRA wait timeout, START response");
	}
*/
	
	if (bIsError) {
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "start failed";
	} else {
		clsVnf.m_bResult = true;
		clsVnf.m_strReason = " ";
	}

Response_:
	clsResp.SetResponse(a_clsReq);
	clsResp.SetPayload(clsVnf.ResponseGen());

	CModuleIPC::Instance().SendMesg(clsResp);

	if (bIsError) {
		return false;
	}
	return true;
}

bool CCommandNa::CmdVnfStop(CProtocol& a_clsReq)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CAddress& clsAddr = CAddress::Instance();

	CCmdVnfStop clsVnf;					// VNF STOP 메세지 body parsing
	CCmdAppStart clsApp;				// PRA START 메세지 body 생성
	CCmdNodeStatus clsNodeStat;			// Node 상태 body 생

	CProtocol clsResp;					// EXA 음답
	CProtocol clsProto;
	string strBody;
	bool bIsError = false;
	int nNodeNo = 0;
   	int nProcNo = 0;


	if (a_clsReq.IsFlagRequest() == false) {
		g_pclsLog->ERROR("CMD, 'VNF_STOP', not set request flag");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);

		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "net set request flag";
		bIsError = true;
		goto Response_;
	}

	if (clsVnf.RequestParse(a_clsReq.GetPayload().c_str()) == false) {
		g_pclsLog->ERROR("CMD, 'VNF_STOP', invalied body format");
		a_clsReq.Print(g_pclsLog, LV_DEBUG, true);
		
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "invalied body format";
		bIsError = true;
		goto Response_;
	}

	// PRA 프로세스 중지
	// PRA 에 프로세스 stop 명령 전송
	clsApp.m_bAll = true;
	clsAddr.LookupAtom("ATOM_NA_PRA", nNodeNo, nProcNo);
	
	clsProto.SetCommand(CMD_APP_STOP);
	clsProto.SetFlagNotify();
	clsProto.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsProto.SetDestination(nNodeNo, nProcNo);
	clsProto.SetSequence();
	clsProto.SetPayload(clsApp.RequestGen());
	CModuleIPC::Instance().SendMesg(clsProto);

	bIsError = false;
/*
	if (clsIPC.RecvMesg(clsCfg.m_nProcNo, clsProto, 5) > 0) {
		nCmd = atoi(clsProto.GetCommand().c_str());
		if (nCmd == CMD_APP_START) {
			if (clsApp.ResponseParse(clsProto.GetPayload().c_str())) {
				if (clsApp.m_strWorstStatus.compare("RUNNING") == 0) {
					bIsError = false;
				} else {
					g_pclsLog->ERROR("porcess running failed");
				}
			} else {
				g_pclsLog->ERROR("start failed");
			}
		} else {
		    g_pclsLog->ERROR("unknown command: %d", nCmd);
		}
	} else {
		g_pclsLog->ERROR("PRA wait timeout, STOP response");
	}
*/
	
	if (bIsError) {
		clsVnf.m_bResult = false;
		clsVnf.m_strReason = "stop failed";
	} else {
		clsVnf.m_bResult = true;
		clsVnf.m_strReason = " ";
		
		// Node 상태(ScaleIn) 전송
		if (clsVnf.m_bIsScaleIn) {
			clsNodeStat.m_strUuid		= clsVnf.m_strUuid;
			clsNodeStat.m_strStatus		= "SCALEIN";
			clsNodeStat.m_strPkgName	= clsCfg.m_strPkgName;
			clsNodeStat.m_strNodeType	= clsCfg.m_strNodeType;
			clsNodeStat.m_nNodeNo		= clsCfg.m_nNodeNo;
			clsNodeStat.m_strNodeName	= clsCfg.m_strNodeName;
			clsNodeStat.m_strIp			= clsCfg.m_strNodeIp;
			clsNodeStat.m_strVersion	= clsCfg.m_strVersion;

			if (clsAddr.LookupAtom("ATOM_NM", nNodeNo, nProcNo) == 1) {
				clsProto.Clear();
				clsProto.SetCommand(CMD_NODE_STATUS);
				clsProto.SetFlagNotify();
				clsProto.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
				clsProto.SetDestination(nNodeNo, nProcNo);
				clsProto.SetSequence();
				clsProto.SetPayload(clsNodeStat.NotifyGen());
				if (CModuleIPC::Instance().SendMesg(clsProto) == false) {
					g_pclsLog->ERROR("CMD, NODE_STATUS send failed to NM, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
				}
		
			} else {
				g_pclsLog->ERROR("CMD, NM address lookup failed");
			}
			
		} //end if

	} //end if

Response_:
	clsResp.SetResponse(a_clsReq);
	clsResp.SetPayload(clsVnf.ResponseGen());

	CModuleIPC::Instance().SendMesg(clsResp);

	if (bIsError) {
		return false;
	}
	return true;
}
