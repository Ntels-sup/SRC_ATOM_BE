/* vim:ts=4:sw=4
 */
/**
 * \file	CCommandPra.cpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include "CommandFormat.hpp"
#include "CFileLog.hpp"
#include "CMesgRoute.hpp"
#include "CModuleIPC.hpp"
#include "CAddress.hpp"
#include "CCliReq.hpp"
#include "CCliRsp.hpp"

#include "CConfigPra.hpp"
#include "CCmdBase.hpp"
#include "CCmdAppInit.hpp"
#include "CCmdBatchStart.hpp"
#include "CCmdAppLogLevel.hpp"
#include "CCmdAppCtl.hpp"
#include "CCommandPra.hpp"
#include "CProcStatus.hpp"
#include "CAppQueue.hpp"

using std::string;
using std::list;
using std::pair;

extern CFileLog* g_pclsLogPra;


bool CCommandPra::ProcMesgModule(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CConfigPra& clsCfg = CConfigPra::Instance();

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Recvive message form Module");
	a_clsRecvP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	int nDstNode = 0;
	int nDstProc = 0; 
	a_clsRecvP.GetDestination(nDstNode, nDstProc);

	// PRA가 처리해 할 명령인가?
	if (nDstProc == clsCfg.m_nProcNo) {
		return Command(a_clsRecvP, a_clsPM);
	}

	// 메세지를 전송할 APP를 찾는다.
	string strProcName;
	if (a_clsPM.GetProcName(nDstProc, &strProcName) != NULL) {
		CAppQueue& clsQ = CAppQueue::Instance();
		if (clsQ.SendCmd(a_clsRecvP, strProcName.c_str()) == false) {
			g_pclsLogPra->ERROR("message send faile, to app");
			a_clsRecvP.Print(g_pclsLogPra, LV_INFO);
			return false;
		}
	} else {
		g_pclsLogPra->ERROR("not found target app process");
		a_clsRecvP.Print(g_pclsLogPra, LV_INFO);
	}

	return true;
}

bool CCommandPra::ProcMesgApp(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CConfigPra& clsCfg = CConfigPra::Instance();

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Recvive message form App");
	a_clsRecvP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	int nDstNode = 0;
	int nDstProc = 0; 
	a_clsRecvP.GetDestination(nDstNode, nDstProc);
	
	// TODO
	// APP에서 ATOM node, proc no을 알수 없어 0으로 지정한 경우
	// command에 따라 ATOM node, procno을 재지정 해준다.
	// 이럴 경우 APP에서 추가 명령이 발생할 때 마다 수정이 필요하므로
	// 향후 개선이 필요하다.
	if (nDstNode == 0 && nDstProc == 0) {
		int nCmd = atoi(a_clsRecvP.GetCommand().c_str());
		switch (nCmd) {
			case CMD_NODE_LIST :
				CAddress::Instance().LookupAtom("ATOM_NM", nDstNode, nDstProc);
				a_clsRecvP.SetDestination(nDstNode, nDstProc);
				break;
		}
	}

	// PRA가 처리해 할 명령인가?
	if (nDstProc == clsCfg.m_nProcNo) {
		return Command(a_clsRecvP, a_clsPM);
	}

	// agent에 메세지 전송
	CModuleIPC& clsIPC = CModuleIPC::Instance();
	if (clsIPC.SendMesg(a_clsRecvP) == false) {
		g_pclsLogPra->ERROR("CMD, mode ipc send failed");
		a_clsRecvP.Print(g_pclsLogPra, LV_INFO);
		return false;
	}

	// APP의 Alive 주기적 검사를 줄이기 위해 APP에서 메세지가 왔다면
	// 이를 APP가 정상적으로 Alive 중인것으로 판단.
	int nSrcNode = 0;
	int nSrcProc = 0;
	a_clsRecvP.GetSource(nSrcNode, nSrcProc);
	if (nSrcProc > 0) {
		a_clsPM.AliveOk(nSrcProc);
	}

	return true;
}

void CCommandPra::GenAppStatus(CProcessManager& a_clsPM, CCmdBase& a_clsBase)
{
	list<CProcessManager::ST_APPRUNINFO> lstInfo;

	if (a_clsBase.m_bIsAll) {
		// 전체 프로세스의 상태 정보를 요청
		a_clsPM.GetStatus(lstInfo, -1);
		GenNaStatus(a_clsBase);
	} else {
		// 지정 프로세스의 상태 정보를 요청
		list<pair<int, string> >::iterator iter = a_clsBase.m_lstTarget.begin();
		for (; iter != a_clsBase.m_lstTarget.end(); ++iter) {
			if (iter->second.compare("NA") == 0) {
				GenNaStatus(a_clsBase);
				continue;
			}
			a_clsPM.GetStatus(lstInfo, iter->first);
		}
	}

	CCmdBase::ST_PROCESS stProc;
	struct tm ltm;
	char ltmbuf[40] = {0x00,};
	
	map<int, CCmdBase::ST_RESPONSE>::iterator miter;
	
	for (auto liter = lstInfo.begin(); liter != lstInfo.end(); ++liter) {
		stProc.m_nProcNo 		= liter->m_stInfo.m_nProcNo;
		stProc.m_strProcName	= liter->m_stInfo.m_strProcName;
		stProc.m_strStatus		= CProcStatus::StatusToString(liter->m_stRun.m_enStatus);
		if (liter->m_stRun.m_tStartTime > 0) {
			localtime_r(&liter->m_stRun.m_tStartTime, &ltm);
			snprintf(ltmbuf, sizeof(ltmbuf), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
			stProc.m_strStartDate	= ltmbuf;
		}
		if (liter->m_stRun.m_tStopTime > 0) {
			localtime_r(&liter->m_stRun.m_tStopTime, &ltm);
			snprintf(ltmbuf, sizeof(ltmbuf), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		
			stProc.m_strStopDate	= ltmbuf;
		}
		stProc.m_strVersion		= liter->m_stRun.m_strVersion;
		
		miter = a_clsBase.m_mapResponse.find(liter->m_stInfo.m_nSvcNo);
		if (miter == a_clsBase.m_mapResponse.end()) {
			miter = a_clsBase.m_mapResponse.insert(a_clsBase.m_mapResponse.end(),
							map<int, CCmdBase::ST_RESPONSE>::value_type(
								liter->m_stInfo.m_nSvcNo, CCmdBase::ST_RESPONSE()));
			miter->second.m_strSvcName = liter->m_stInfo.m_strSvcName;
			miter->second.m_strWorstStatus = CProcStatus::StatusToString(a_clsPM.GetStatusWorst());
		}
		miter->second.m_vecProcess.push_back(stProc);
	}

	a_clsBase.m_strWorstStatus = CProcStatus::StatusToString(a_clsPM.GetStatusWorst());

	return;
}

void CCommandPra::GenNaStatus(CCmdBase& a_clsBase)
{
	CConfigPra& clsCfg = CConfigPra::Instance();

	// service 정보 
	map<int, CCmdBase::ST_RESPONSE>::iterator miter;
	miter = a_clsBase.m_mapResponse.insert(a_clsBase.m_mapResponse.end(),
							map<int, CCmdBase::ST_RESPONSE>::value_type(
								clsCfg.m_strNaSvcNo, CCmdBase::ST_RESPONSE()));
	miter->second.m_strSvcName = clsCfg.m_strNaSvcName;
	miter->second.m_strWorstStatus = "RUNNING";

	// NA process no 정보
	int nNodeNo = 0;
	int nProcNo = 0;
	CAddress::Instance().LookupAtom("ATOM_NA", nNodeNo, nProcNo);

	char ltmbuf[40] = {0x00,};
	struct tm ltm;
	localtime_r(&clsCfg.m_tNaStartTime, &ltm);
	snprintf(ltmbuf, sizeof(ltmbuf), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		

	CCmdBase::ST_PROCESS stProc;
	stProc.m_nProcNo 		= nProcNo;
	stProc.m_strProcName	= "NA";
	stProc.m_strStatus		= "RUNNING";
	stProc.m_strStartDate	= ltmbuf;
	//stProc.m_strStopDate;
	stProc.m_strVersion		= clsCfg.m_strNaVersion;

	miter->second.m_vecProcess.push_back(stProc);

	return;
}


bool CCommandPra::Command(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	int nCmd = atoi(a_clsRecvP.GetCommand().c_str());
	switch (nCmd) {
		case CMD_PING :
			CmdPing(a_clsRecvP);
			break;
		case CMD_PRA_RECONFIG :
			CmdAppReconfig(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_STATUS :
			CmdAppStatus(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_START :
			CmdAppStart(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_STOP :
			CmdAppStop(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_KILL :
			CmdAppKill(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_INIT :
			CmdAppInit(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_SUSPEND :
			CmdAppSuspend(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_RESUME :
			CmdAppResume(a_clsRecvP, a_clsPM);
			break;
		case CMD_APP_LOGLEVEL :
			CmdAppLogLevel(a_clsRecvP, a_clsPM);
			break;
		case CMD_STS_APP :
			CmdStaticApp(a_clsRecvP, a_clsPM);
			break;
		case CMD_BATCH_START :
			CmdBatchStart(a_clsRecvP, a_clsPM);
			break;
		case CMD_CLI_COMMAND :
			CmdCli(a_clsRecvP, a_clsPM);
			break;
		default :
			g_pclsLogPra->ERROR("CMD, not support command");
			a_clsRecvP.Print(g_pclsLogPra, LV_INFO, true);
			return false;
	}
	
	return true;
}

void CCommandPra::CmdPing(CProtocol& a_clsRecvP)
{
	CProtocol clsResp;
	clsResp.SetResponse(a_clsRecvP);

	CCmdBase clsBase;
	string strBody;
	if (a_clsRecvP.IsFlagRequest()) {
		strBody = clsBase.ErrorGen(0, "ok i am alive");
	} else { 
		strBody = clsBase.ErrorGen(0, "not set requst flag");
		clsResp.SetFlagError();
		g_pclsLogPra->ERROR("MSGP, ping, not set request flag");
	}
	
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsResp.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsResp.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;
}

void CCommandPra::CmdAppReconfig(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CProtocol clsResp;

	CCmdBase clsBase;
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD, 'PRA_RECONFIG', check notify flag");
		return;
	}

	CConfigPra& clsCfg = CConfigPra::Instance();
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	CAddress& clsAddr = CAddress::Instance();

	list<CProcessManager::ST_APPINFO> lstProcess;
	clsCfg.LoadProcess(lstProcess);

	// 현재 프로세스의 비활성화 flag 설정
	a_clsPM.Disable();
	
	for (auto iter = lstProcess.begin(); iter != lstProcess.end(); ++iter) {
		a_clsPM.Add(*iter);
		if (iter->m_bIsExec) {
			a_clsPM.Run(iter->m_nProcNo);
		}
		clsAddr.AddApp(iter->m_strPkgName.c_str(), iter->m_strNodeType.c_str(),
						iter->m_strProcName.c_str(), iter->m_nProcNo);
		clsRoute.AddRoute(iter->m_nProcNo, clsCfg.m_nProcNo);
	}
	clsRoute.PrintTable(g_pclsLogPra, LV_INFO);

	return;
}

void CCommandPra::CmdAppStatus(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CCmdBase clsBase;
	bool bIsError = false;
	string strBody;


	if (a_clsRecvP.IsFlagRequest() == false) {
		strBody = clsBase.ErrorGen(1, "not set request flag");
		bIsError = true;
		goto JumpSend;
	}

	if (clsBase.RequestParse(a_clsRecvP.GetPayload().c_str()) == false) {
		strBody = clsBase.ErrorGen(1, "invalied body format");
		bIsError = true;
		goto JumpSend;
	}

	GenAppStatus(a_clsPM, clsBase);
	strBody = clsBase.ResponseGen();

JumpSend:
	CProtocol clsResp;
	clsResp.SetResponse(a_clsRecvP);
	if (bIsError) {
		clsResp.SetFlagError();
	}
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsResp.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsResp.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;
}

void CCommandPra::CmdAppStart(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD 'APP_START', not set notify flag");
	}

	CCmdBase clsBase;
	if (clsBase.NotifyParse(a_clsRecvP.GetPayload().c_str()) == false) {
		g_pclsLogPra->ERROR("CMD 'APP_START', invalied body format");
		return;
	}

	if (clsBase.m_bIsAll) {
		// 전체 프로세스 실행 요청
		a_clsPM.Run(-1);
	} else {
		// 지정 프로세스의 실행 요청
		list<pair<int, string> >::iterator iter = clsBase.m_lstTarget.begin();
		for (; iter != clsBase.m_lstTarget.end(); ++iter) {
			a_clsPM.Run(iter->first);
		}
	}

	return;
}

void CCommandPra::CmdAppStop(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD 'APP_STOP', not set notify flag");
	}

	CCmdBase clsBase;
	if (clsBase.NotifyParse(a_clsRecvP.GetPayload().c_str()) == false) {
		g_pclsLogPra->ERROR("CMD 'APP_STOP', invalied body format");
		return;
	}

	if (clsBase.m_bIsAll) {
		// 전체 프로세스 stop 요청
		a_clsPM.Stop();
	} else {
		// 지정 프로세스의 stop 요청
		auto iter = clsBase.m_lstTarget.begin();
		for (; iter != clsBase.m_lstTarget.end(); ++iter) {
			a_clsPM.Stop(iter->first);
		}
	}

	return;
}

void CCommandPra::CmdAppKill(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD 'APP_KILL', not set notify flag");
	}

	CCmdBase clsBase;
	if (clsBase.NotifyParse(a_clsRecvP.GetPayload().c_str()) == false) {
		g_pclsLogPra->ERROR("CMD 'APP_KILL', invalied body format");
		return;
	}

	if (clsBase.m_bIsAll) {
		// 전체 프로세스 강제 종료
		a_clsPM.StopForceibly();
	} else {
		// 지정 프로세스의 강제 종료
		auto iter = clsBase.m_lstTarget.begin();
		for (; iter != clsBase.m_lstTarget.end(); ++iter) {
			a_clsPM.StopForceibly(iter->first);
		}
	}

	return;
}

void CCommandPra::CmdAppInit(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CConfigPra& clsCfg = CConfigPra::Instance();
	CAppQueue& clsQ = CAppQueue::Instance();
	
	CCmdAppInit clsInit;				// Request body parsing
	CCmdAppInit::ST_RESPONSE stResponse;
	string strBody;
	bool bIsError = false;
			
	CCmdAppCtl clsCtl;					// APP에 보낼 body 메세지
	CProtocol clsSendP;					// APP에 보낼 message


	if (a_clsRecvP.IsFlagRequest() == false) {
		strBody = clsInit.ErrorGen(1, "CMD 'APP_INIT', not set request flag");
		bIsError = true;
		goto JumpSend;
	}

	if (clsInit.RequestParse(a_clsRecvP.GetPayload().c_str()) == false) {
		strBody = clsInit.ErrorGen(1, "CMD 'APP_INIT', invalied body format");
		bIsError = true;
		goto JumpSend;
	}
	
	// 프로세스 보낼 메세지 구성
	clsCtl.m_strAction = "INIT";
	clsSendP.SetCommand(CMD_APP_CTL);
	clsSendP.SetFlagNotify();
	clsSendP.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsSendP.SetSequence();
	clsSendP.SetPayload(clsCtl.RequestGen());

	if (clsInit.m_bIsAll) {
		// 전체 프로세스 init 요청
		vector<CProcessManager::ST_APPINFO> vecInfo;
		a_clsPM.GetAppInfo(vecInfo, CProcStatus::RUNNING);
		for (size_t i=0; i < vecInfo.size(); ++i) {
			clsSendP.SetDestination(clsCfg.m_nNodeNo, vecInfo[i].m_nProcNo);
			
			stResponse.m_nProcNo = vecInfo[i].m_nProcNo;
			stResponse.m_strProcName = vecInfo[i].m_strProcName;
			stResponse.m_bSuccess = true;
			
			if (clsQ.SendCmd(clsSendP, vecInfo[i].m_strProcName.c_str()) == false) {
				g_pclsLogPra->ERROR("APP init send failed");
				stResponse.m_bSuccess = false;
			}
			
			clsInit.m_lstResponse.push_back(stResponse);
		}

	} else {
		// 지정 프로세스의 init 요청
		list<pair<int, string> >::iterator iter = clsInit.m_lstTarget.begin();
		for (; iter != clsInit.m_lstTarget.end(); ++iter) {
			clsSendP.SetDestination(clsCfg.m_nNodeNo, iter->first);
			
			stResponse.m_nProcNo = iter->first;
			stResponse.m_strProcName = iter->second;
			stResponse.m_bSuccess = true;
			
			if (clsQ.SendCmd(clsSendP, iter->second.c_str()) == false) {
				g_pclsLogPra->ERROR("APP init send failed");
				stResponse.m_bSuccess = false;
			}

			clsInit.m_lstResponse.push_back(stResponse);
		}
	}
	strBody = clsInit.ResponseGen();

JumpSend:
	CProtocol clsResp;
	clsResp.SetResponse(a_clsRecvP);
	if (bIsError) {
		clsResp.SetFlagError();
	}
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsResp.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsResp.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;
}

void CCommandPra::CmdAppSuspend(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	//TODO
	return;
}

void CCommandPra::CmdAppResume(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	//TODO
	return;
}

void CCommandPra::CmdAppLogLevel(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CConfigPra& clsCfg = CConfigPra::Instance();
	CAppQueue& clsQ = CAppQueue::Instance();
	
	CCmdAppLogLevel clsLog;				// Notify parsing
	string strBody;

	CCmdAppCtl clsCtl;					// APP에 보낼 body 메세지
	CProtocol clsSendP;					// APP에 보낼 message


	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD, 'LOG_LEVEL' not set notify flag");
	}

	if (clsLog.NotifyParse(a_clsRecvP.GetPayload().c_str()) == false) {
		g_pclsLogPra->ERROR("CMD, 'LOG_LEVEL' invalied body format");
		return;
	}
	
	// 프로세스 보낼 메세지 구성
	clsCtl.m_strAction = "LOG_LEVEL";
	clsCtl.m_nOption = clsLog.m_nLogLevel;

	clsSendP.SetCommand(CMD_APP_CTL);
	clsSendP.SetFlagRequest();
	clsSendP.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsSendP.SetSequence();
	clsSendP.SetPayload(clsCtl.RequestGen());

	if (clsLog.m_bIsAll) {
		// 전체 프로세스 loglevel 변경 요청
		vector<CProcessManager::ST_APPINFO> vecInfo;
		a_clsPM.GetAppInfo(vecInfo, CProcStatus::RUNNING);
		for (size_t i=0; i < vecInfo.size(); ++i) {
			clsSendP.SetDestination(clsCfg.m_nNodeNo, vecInfo[i].m_nProcNo);
			
			if (clsQ.SendCmd(clsSendP, vecInfo[i].m_strProcName.c_str()) == false) {
				g_pclsLogPra->ERROR("APP loglevel send failed");
			}
		}

	} else {
		// 지정 프로세스의 loglevel 변경 요청
		list<pair<int, string> >::iterator iter = clsLog.m_lstTarget.begin();
		for (; iter != clsLog.m_lstTarget.end(); ++iter) {
			clsSendP.SetDestination(clsCfg.m_nNodeNo, iter->first);
			
			if (clsQ.SendCmd(clsSendP, iter->second.c_str()) == false) {
				g_pclsLogPra->ERROR("APP loglevel send failed");
			}
		}
	}

	return;
}

void CCommandPra::CmdStaticApp(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	if (a_clsRecvP.IsFlagBroadcast() == false) {
		g_pclsLogPra->WARNING("statics not set broadcast");
	}
	
	vector<CProcessManager::ST_APPINFO> vecInfo;
	a_clsPM.GetAppInfo(vecInfo, CProcStatus::RUNNING);
	
	CConfigPra& clsCfg = CConfigPra::Instance();
	CAppQueue& clsQ = CAppQueue::Instance();
	
	for (size_t i=0; i < vecInfo.size(); i++) {
		a_clsRecvP.SetDestination(clsCfg.m_nNodeNo, vecInfo[i].m_nProcNo);
		if (clsQ.SendCmd(a_clsRecvP, vecInfo[i].m_strProcName.c_str()) == false) {
			g_pclsLogPra->ERROR("statics send failed, pname: %s", 
												vecInfo[i].m_strProcName.c_str());
		}
	}
	
	return;
}

void CCommandPra::CmdBatchStart(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CCmdBatchStart clsBatch;
	bool bIsError = false;
	string strBody;
	CProcStatus::EN_STATUS nStatus = CProcStatus::NONE;


	if (a_clsRecvP.IsFlagRequest() == false) {
		strBody = clsBatch.ErrorGen(1, "not set request flag");
		bIsError = true;
		goto JumpSend;
	}

	if (clsBatch.RequestParse(a_clsRecvP.GetPayload().c_str()) == false) {
		strBody = clsBatch.ErrorGen(1, "invalied body format");
		bIsError = true;
		goto JumpSend;
	}

	// 배치 프로세스 실행
	a_clsPM.Run(clsBatch.m_nProcNo);

	// batch exit wait
	usleep(100);
	a_clsPM.CheckExit(clsBatch.m_nProcNo);


	nStatus = a_clsPM.GetStatus(clsBatch.m_nProcNo);
	switch (nStatus) {
		case CProcStatus::RUNNING :
			clsBatch.m_strStatus = "pending";
			break;
		case CProcStatus::STOPPED :
			clsBatch.m_strStatus = "normal";
			clsBatch.m_nExitCd = a_clsPM.GetExitCode(clsBatch.m_nProcNo);
			break;
		default :
			clsBatch.m_strStatus = "failed";
	}
	strBody = clsBatch.ResponseGen();

JumpSend:
	CProtocol clsResp;
	clsResp.SetResponse(a_clsRecvP);
	if (bIsError) {
		clsResp.SetFlagError();
	}
	clsResp.SetPayload(strBody);

	if (CModuleIPC::Instance().SendMesg(clsResp) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsResp.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsResp.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif

	return;
}

void CCommandPra::CmdCli(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CCliReq clsCliReq;
	
	string strBody = a_clsRecvP.GetPayload();
	if (clsCliReq.DecodeMessage(strBody) != CCliApi::RESULT_OK) {
		g_pclsLogPra->ERROR("CMD 'CLI' invalied body message");
		return;
	}

	string strCliCmd = clsCliReq.GetCmdName();
	if (strCliCmd.compare("DISP-PRC") == 0) {
		return CmdCli_DispProc(a_clsRecvP, clsCliReq, a_clsPM);
	} else if (strCliCmd.compare("INIT-PRC") == 0) {
		return CmdCli_InitProc(a_clsRecvP, clsCliReq, a_clsPM);
	} else if (strCliCmd.compare("START-PRC") == 0) {
		return CmdCli_StartProc(a_clsRecvP, clsCliReq, a_clsPM);
	} else if (strCliCmd.compare("STOP-PRC") == 0) {
		return CmdCli_StopProc(a_clsRecvP, clsCliReq, a_clsPM);
	} else {
		g_pclsLogPra->WARNING("CMD 'CLI', not support command");
		return;
	}

	return;
}

void CCommandPra::CmdCli_DispProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq,
													CProcessManager& a_clsPM)
{
	list<CProcessManager::ST_APPRUNINFO> lstInfo;
	a_clsPM.GetStatus(lstInfo, -1);
	string strWorst = CProcStatus::StatusToString(a_clsPM.GetStatusWorst());

	struct tm ltm;
	char szStartTime[40] = {0x00,};
	char szStopTime[40] = {0x00,};

	CCliRsp	clsCliRsp;
	clsCliRsp.SetSessionId(clsCliReq.GetSessionId());
	clsCliRsp.SetResultCode(1, "SUCCESS");
	clsCliRsp.SetText      ("");
	clsCliRsp.NPrintf(1024, "worst status: %s\n", strWorst.c_str());
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	clsCliRsp.SetTextAppend("       pid             procname     status           start time            stop time    version\n");
	clsCliRsp.SetTextAppend("---------- -------------------- ---------- -------------------- -------------------- ----------\n");

	for (auto iter = lstInfo.begin(); iter != lstInfo.end(); ++iter) {
		if (iter->m_stRun.m_tStartTime > 0) {
			localtime_r(&iter->m_stRun.m_tStartTime, &ltm);
			snprintf(szStartTime, sizeof(szStartTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
		}
		if (iter->m_stRun.m_tStopTime > 0) {
			localtime_r(&iter->m_stRun.m_tStopTime, &ltm);
			snprintf(szStopTime, sizeof(szStopTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		
		}
		
		clsCliRsp.NPrintf(1024, "%10d %-20s %10s %20s %20s %10s\n",
								iter->m_stRun.m_nPid,
								iter->m_stInfo.m_strProcName.c_str(),
								CProcStatus::StatusToString(iter->m_stRun.m_enStatus),
								szStartTime,
								szStopTime,
								iter->m_stRun.m_strVersion.c_str());
	}
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	
	string strBody;
	clsCliRsp.EncodeMessage(strBody);

	// 응답 메세지 생성
	CProtocol clsSendP;
	clsSendP.SetResponse(a_clsRecvP);
	clsSendP.SetPayload(strBody);
	
	if (CModuleIPC::Instance().SendMesg(clsSendP) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsSendP.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsSendP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;
}

void CCommandPra::CmdCli_StartProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq,
													CProcessManager& a_clsPM)
{
	vector<int> vecTarget;
	int nProcNo = -1;
	
	for (uint32_t i=0; i < clsCliReq.GetCount(); i++) {
		// Argument
		if (clsCliReq[i].GetName().compare("SERVICE_NAME") == 0) {
			// TODO
			// sevice name process invoke
			continue;
			
		} else if (clsCliReq[i].GetName().compare("PROCESS_NAME") == 0) {
			// Parameter
			for (uint32_t j=0; j < clsCliReq[i].GetCount(); j++) {
				nProcNo = a_clsPM.FindProcNo(clsCliReq[i][j].GetString());
				if (nProcNo <= 0) {
					g_pclsLogPra->ERROR("CMD 'CLI' uknown process no, pname: %s",
											clsCliReq[i][j].GetString().c_str());
					continue;
				}
				a_clsPM.Run(nProcNo);
				
				vecTarget.push_back(nProcNo);
			}
		}
	}
	
	list<CProcessManager::ST_APPRUNINFO> lstInfo;
	list<CProcessManager::ST_APPRUNINFO>::iterator liter;

	struct tm ltm;
	char szStartTime[40] = {0x00,};
	char szStopTime[40] = {0x00,};

	CCliRsp	clsCliRsp;
	clsCliRsp.SetSessionId(clsCliReq.GetSessionId());
	clsCliRsp.SetResultCode(1, "SUCCESS");
	clsCliRsp.SetText      ("");
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	clsCliRsp.SetTextAppend("       pid             procname     status           start time            stop time    version\n");
	clsCliRsp.SetTextAppend("---------- -------------------- ---------- -------------------- -------------------- ----------\n");

	for (size_t i=0; i < vecTarget.size(); i++) {

		if (a_clsPM.GetStatus(lstInfo, vecTarget[i]) != 1) {
			g_pclsLogPra->ERROR("CMD 'CLI' status not found, pno: %d", vecTarget[i]);
			continue;
		}
		liter = lstInfo.begin();
	
		if (liter->m_stRun.m_tStartTime > 0) {
			localtime_r(&liter->m_stRun.m_tStartTime, &ltm);
			snprintf(szStartTime, sizeof(szStartTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
		}
		if (liter->m_stRun.m_tStopTime > 0) {
			localtime_r(&liter->m_stRun.m_tStopTime, &ltm);
			snprintf(szStopTime, sizeof(szStopTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		
		}
		
		clsCliRsp.NPrintf(1024, "%10d %-20s %10s %20s %20s %10s\n",
								liter->m_stRun.m_nPid,
								liter->m_stInfo.m_strProcName.c_str(),
								CProcStatus::StatusToString(liter->m_stRun.m_enStatus),
								szStartTime,
								szStopTime,
								liter->m_stRun.m_strVersion.c_str());
	}
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	
	string strBody;
	clsCliRsp.EncodeMessage(strBody);

	// 응답 메세지 생성
	CProtocol clsSendP;
	clsSendP.SetResponse(a_clsRecvP);
	clsSendP.SetPayload(strBody);
	
	if (CModuleIPC::Instance().SendMesg(clsSendP) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsSendP.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsSendP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;	
}

void CCommandPra::CmdCli_StopProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq,
													CProcessManager& a_clsPM)
{
	vector<int> vecTarget;
	int nProcNo = -1;
	
	for (uint32_t i=0; i < clsCliReq.GetCount(); i++) {
		// Argument
		if (clsCliReq[i].GetName().compare("SERVICE_NAME") == 0) {
			// TODO
			// sevice name process invoke
			continue;
			
		} else if (clsCliReq[i].GetName().compare("PROCESS_NAME") == 0) {
			// Parameter
			for (uint32_t j=0; j < clsCliReq[i].GetCount(); j++) {
				nProcNo = a_clsPM.FindProcNo(clsCliReq[i][j].GetString());
				if (nProcNo <= 0) {
					g_pclsLogPra->ERROR("CMD 'CLI' uknown process no, pname: %s",
											clsCliReq[i][j].GetString().c_str());
					continue;
				}
				a_clsPM.Stop(nProcNo);
				
				vecTarget.push_back(nProcNo);
			}
		}
	}
	
	list<CProcessManager::ST_APPRUNINFO> lstInfo;
	list<CProcessManager::ST_APPRUNINFO>::iterator liter;

	struct tm ltm;
	char szStartTime[40] = {0x00,};
	char szStopTime[40] = {0x00,};

	CCliRsp	clsCliRsp;
	clsCliRsp.SetSessionId(clsCliReq.GetSessionId());
	clsCliRsp.SetResultCode(1, "SUCCESS");
	clsCliRsp.SetText      ("");
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	clsCliRsp.SetTextAppend("       pid             procname     status           start time            stop time    version\n");
	clsCliRsp.SetTextAppend("---------- -------------------- ---------- -------------------- -------------------- ----------\n");

	for (size_t i=0; i < vecTarget.size(); i++) {

		if (a_clsPM.GetStatus(lstInfo, vecTarget[i]) != 1) {
			g_pclsLogPra->ERROR("CMD 'CLI' status not found, pno: %d", vecTarget[i]);
			continue;
		}
		liter = lstInfo.begin();

		if (liter->m_stRun.m_tStartTime > 0) {
			localtime_r(&liter->m_stRun.m_tStartTime, &ltm);
			snprintf(szStartTime, sizeof(szStartTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
		}
		if (liter->m_stRun.m_tStopTime > 0) {
			localtime_r(&liter->m_stRun.m_tStopTime, &ltm);
			snprintf(szStopTime, sizeof(szStopTime), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		
		}
		
		clsCliRsp.NPrintf(1024, "%10d %-20s %10s %20s %20s %10s\n",
								liter->m_stRun.m_nPid,
								liter->m_stInfo.m_strProcName.c_str(),
								CProcStatus::StatusToString(liter->m_stRun.m_enStatus),
								szStartTime,
								szStopTime,
								liter->m_stRun.m_strVersion.c_str());
	}
	clsCliRsp.SetTextAppend("===============================================================================================\n");
	
	string strBody;
	clsCliRsp.EncodeMessage(strBody);

	// 응답 메세지 생성
	CProtocol clsSendP;
	clsSendP.SetResponse(a_clsRecvP);
	clsSendP.SetPayload(strBody);
	
	if (CModuleIPC::Instance().SendMesg(clsSendP) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsSendP.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsSendP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;	
}

void CCommandPra::CmdCli_InitProc(CProtocol& a_clsRecvP, CCliReq& clsCliReq,
													CProcessManager& a_clsPM)
{
	CConfigPra& clsCfg = CConfigPra::Instance();

	// 프로세스 보낼 메세지 구성
	CCmdAppCtl clsCtl;					// APP에 보낼 body 메세지
	clsCtl.m_strAction = "INIT";

	CProtocol clsSendP;					// APP에 보낼 message
	clsSendP.SetCommand(CMD_APP_CTL);
	clsSendP.SetFlagNotify();
	clsSendP.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsSendP.SetSequence();
	clsSendP.SetPayload(clsCtl.RequestGen());
	
	// CLI response 메세지 구성
	CCliRsp	clsCliRsp;
	clsCliRsp.SetSessionId(clsCliReq.GetSessionId());
	clsCliRsp.SetResultCode(1, "SUCCESS");
	clsCliRsp.SetText      ("");
	clsCliRsp.SetTextAppend("================================\n");
	clsCliRsp.SetTextAppend("    procno   procname    success\n");
	clsCliRsp.SetTextAppend("---------- ---------- ----------\n");
	
	CAppQueue& clsQ = CAppQueue::Instance();
	string strProcName;
	int nProcNo = -1;
	
	for (uint32_t i=0; i < clsCliReq.GetCount(); i++) {
		// Argument
		if (clsCliReq[i].GetName().compare("SERVICE_NAME") == 0) {
			// TODO
			// sevice name process invoke
			continue;
			
		} else if (clsCliReq[i].GetName().compare("PROCESS_NAME") == 0) {
			// Parameter
			for (uint32_t j=0; j < clsCliReq[i].GetCount(); j++) {
				strProcName = clsCliReq[i][j].GetString();
				
				nProcNo = a_clsPM.FindProcNo(strProcName);
				if (nProcNo <= 0) {
					g_pclsLogPra->ERROR("CMD 'CLI' uknown process no, pname: %s",
															strProcName.c_str());
					clsCliRsp.NPrintf(1024, "%10d %10s %10s\n",
						   					-1, strProcName.c_str(), "FAILED");
					continue;
				}

				// 프로세스의 init 요청
				clsSendP.SetDestination(clsCfg.m_nNodeNo, nProcNo);
				if (clsQ.SendCmd(clsSendP, clsCliReq[i][j].GetString().c_str()) == false) {
					g_pclsLogPra->ERROR("APP init send failed");
					clsCliRsp.NPrintf(1024, "%10d %10s %10s\n",
						   					-1, strProcName.c_str(), "FAILED");
					continue;
				}

				clsCliRsp.NPrintf(1024, "%10d %10s %10s\n", 
										nProcNo, strProcName.c_str(), "OK");
			}
		}
	}
	clsCliRsp.SetTextAppend("================================\n");
	
	string strBody;
	clsCliRsp.EncodeMessage(strBody);

	// 응답 메세지 생성
	clsSendP.SetResponse(a_clsRecvP);
	clsSendP.SetPayload(strBody);
	
	if (CModuleIPC::Instance().SendMesg(clsSendP) == false) {
		g_pclsLogPra->ERROR("CMD, send message failed, %s", 
								CModuleIPC::Instance().m_strErrorMsg.c_str());
		clsSendP.Print(g_pclsLogPra, LV_ERROR, true);
	}

	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Send message");
	clsSendP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	return;	
}
