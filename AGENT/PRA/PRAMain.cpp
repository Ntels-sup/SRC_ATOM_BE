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

#include "CConfigPra.hpp"
#include "CCommandPra.hpp"
#include "CProcess.hpp"
#include "CProcessManager.hpp"
#include "CAppQueue.hpp"

CFileLog* g_pclsLogPra = NULL;


void* PRAStart(void* pArg)
{
	CModule::ST_MODULE_OPTIONS stOption = *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);

	// Config Init
	CConfigPra& clsCfg = CConfigPra::Instance();
	clsCfg.LoadCfgFile(stOption.m_szCfgFile);

	clsCfg.m_strPkgName = stOption.m_szPkgName;
	clsCfg.m_strNodeType = stOption.m_szNodeType;
	clsCfg.m_strNodeName = stOption.m_szNodeName;
	clsCfg.m_strProcName = stOption.m_szProcName;
	clsCfg.m_nProcNo = stOption.m_nProcNo;
	clsCfg.m_pclsDB = stOption.m_pclsDB;
	
	// Log init
	g_pclsLogPra = new CFileLog();
	if (g_pclsLogPra->Initialize((char*)clsCfg.m_strLogPath.c_str(),
	 									NULL, (char*)"PRA", 0, LV_DEBUG) != 0) {
		fprintf(stderr, "Log initalize failed");
		return NULL;
	}
	clsCfg.ConfigPrint();


	g_pclsLogPra->INFO("PRA module starting.");


	CModuleIPC& clsIPC = CModuleIPC::Instance();
	CAppQueue& clsAppQ = CAppQueue::Instance();
	if (clsAppQ.OpenQ() == false) {
		return NULL;
	}

	bool bIdle = true;
	int	nCnt = 0;
	
	// process 정보를 읽도록 메세지 self 메세지 전송
	CProtocol clsProto;
	clsProto.SetCommand(CMD_PRA_RECONFIG);
	clsProto.SetFlagNotify();
	clsProto.SetSource(0, clsCfg.m_nProcNo);
	clsProto.SetDestination(0, clsCfg.m_nProcNo);
	clsProto.SetSequence();
	if (clsIPC.SendMesg(clsProto) == false) {
		g_pclsLogPra->ERROR("PRA reconfig send failed");
	}

	CCommandPra clsPraCmd;
	CProcessManager clsPM;


	while (true) {
		
		// Module 메세지 수신 처리
		nCnt = clsIPC.RecvMesg(stOption.m_nProcNo, clsProto, -1);
		if (nCnt == 1) {
			clsPraCmd.ProcMesgModule(clsProto, clsPM);
			bIdle = false;
		}
		
		// App 메세지 수신 처리
		nCnt = clsAppQ.RecvCmd(clsProto);
		if (nCnt == 1) {
			clsPraCmd.ProcMesgApp(clsProto, clsPM);
			bIdle = false;
		}
		
		clsPM.CheckExit();
		
		// TODO
		// manual 기동 프로세스 확인
		
		// 처리한 메세지가 없다면 sleep		
		if (bIdle) {
			usleep(3000);
		}
		bIdle = true;
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
