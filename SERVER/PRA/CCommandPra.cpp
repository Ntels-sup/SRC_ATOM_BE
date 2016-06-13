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
#include "../NM/CMesgRoute.hpp"
#include "../NM/CModuleIPC.hpp"
#include "../NM/CAddress.hpp"

#include "CConfigPra.hpp"
#include "CCmdBase.hpp"
#include "CCommandPra.hpp"
#include "CProcStatus.hpp"

using std::string;
using std::list;
using std::pair;

extern CFileLog* g_pclsLogPra;


bool CCommandPra::ProcMesgModule(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	#ifdef PRA_DEBUG
	g_pclsLogPra->DEBUG("Recvive message form Module");
	a_clsRecvP.Print(g_pclsLogPra, LV_DEBUG, true);
	#endif
	
	int nCmd = atoi(a_clsRecvP.GetCommand().c_str());
	switch (nCmd) {
		case CMD_PING :
			CmdPing(a_clsRecvP);
			break;
		case CMD_PRA_RECONFIG :
			CmdReconfig(a_clsRecvP, a_clsPM);
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

	CModuleIPC::Instance().SendMesg(clsResp);
	
	return;
}

void CCommandPra::CmdReconfig(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	CProtocol clsResp;

	CCmdBase clsBase;
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD, 'PRA_RECONFIG', check notify flag");
		return;
	}

	list<CProcessManager::ST_APPINFO> lstProcess;
	CConfigPra::Instance().LoadProcess(lstProcess);

	// 현재 프로세스의 비활성화 flag 설정
	a_clsPM.Disable();
	
	for (auto iter = lstProcess.begin(); iter != lstProcess.end(); ++iter) {
		a_clsPM.Add(*iter);
	}

	return;
}

/*
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

	GetAppStatus(a_clsPM, clsBase);
	strBody = clsBase.ResponseGen();

JumpSend:
	CProtocol clsResp;
	clsResp.SetResponse(a_clsRecvP);
	if (bIsError) {
		clsResp.SetFlagError();
	}
	clsResp.SetPayload(strBody);

	CModuleIPC::Instance().SendMesg(clsResp);
	
	return;
}

void CCommandPra::CmdAppStart(CProtocol& a_clsRecvP, CProcessManager& a_clsPM)
{
	if (a_clsRecvP.IsFlagNotify() == false) {
		g_pclsLogPra->WARNING("CMD 'APP_START', not set request flag");
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
*/

void CCommandPra::GetAppStatus(CProcessManager& a_clsPM, CCmdBase& a_clsBase)
{
	list<CProcessManager::ST_APPRUNINFO> lstInfo;

	if (a_clsBase.m_bIsAll) {
		// 전체 프로세스의 상태 정보를 요청
		a_clsPM.GetStatus(lstInfo, -1);
	} else {
		// 지정 프로세스의 상태 정보를 요청
		list<pair<int, string> >::iterator iter = a_clsBase.m_lstTarget.begin();
		for (; iter != a_clsBase.m_lstTarget.end(); ++iter) {
			a_clsPM.GetStatus(lstInfo, iter->first);
		}
	}

	CCmdBase::ST_RESPONSE stResp;
	struct tm ltm;
	char ltmbuf[40] = {0x00,};

	for (auto iter = lstInfo.begin(); iter != lstInfo.end(); ++iter) {
		stResp.m_nProcNo 		= iter->m_stInfo.m_nProcNo;
		stResp.m_strProcName	= iter->m_stInfo.m_strProcName;
		stResp.m_strStatus		= CProcStatus::StatusToString(iter->m_stRun.m_enStatus);
		if (iter->m_stRun.m_tStartTime > 0) {
			localtime_r(&iter->m_stRun.m_tStartTime, &ltm);
			snprintf(ltmbuf, sizeof(ltmbuf), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
			stResp.m_strStartDate	= ltmbuf;
		}
		if (iter->m_stRun.m_tStopTime > 0) {
			localtime_r(&iter->m_stRun.m_tStopTime, &ltm);
			snprintf(ltmbuf, sizeof(ltmbuf), "%04d-%02d-%02d %02d:%02d:%02d", 
									ltm.tm_year + 1900, ltm.tm_mon+1, ltm.tm_mday,
									ltm.tm_hour, ltm.tm_min, ltm.tm_sec);		
			stResp.m_strStopDate	= ltmbuf;
		}
		stResp.m_strVersion		= iter->m_stRun.m_strVersion;

		a_clsBase.m_lstResponse.push_back(stResp);
	}

	a_clsBase.m_strWorstStatus = CProcStatus::StatusToString(a_clsPM.GetStatusWorst());

	return;
}
