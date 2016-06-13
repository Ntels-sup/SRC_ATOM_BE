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

using std::list;
using std::string;

CFileLog* g_pclsLog = NULL;


void* PRAStart(void* pArg)
{
	CModule::ST_MODULE_OPTIONS stOption = *static_cast<CModule::ST_MODULE_OPTIONS*>(pArg);
	//stOption.m_szPkgName;
	//stOption.m_szNodeType;
	//stOption.m_szProcName;
	//stOption.m_nNodeId;
	//stOption.m_nProcId;
	//stOption.m_pclsModIpc;
	//stOption.m_pclsAddress;

	CModuleIPC* clsIPC = stOption.m_pclsModIpc;
	CAddress*	clsAddr = stOption.m_pclsAddress;

	string strLogPath = "/UDBS/ATOM/LOG";

	// Log init
	g_pclsLog = new CFileLog();
	if (g_pclsLog->Initialize((char*)strLogPath.c_str(),
	 									NULL, (char*)"NA_PRA", 0, LV_DEBUG) != 0) {
		fprintf(stderr, "Log initalize failed");
		return NULL;
	}


	g_pclsLog->INFO("NA PRA module starting.");


	CProtocol clsProto;
	list<CAddress::ST_ADDR> lstAddr;
	int nNodeId, nProcId;
	int nSeq;
	
	while (true) {
		//if (clsIPC->RecvMesg(stOption.m_nProcId, clsProto, -1) > 0) {		// none block mode
		//if (clsIPC->RecvMesg(stOption.m_nProcId, clsProto, 0) > 0) {		// block mode
		if (clsIPC->RecvMesg(stOption.m_nProcId, clsProto, 5) > 0) {		// time 5sec

			int nCmd = atoi(clsProto.GetCommand().c_str());
			switch (nCmd) {
				case CMD_PING :
					clsProto.Print(g_pclsLog, LV_DEBUG, true);
					break;
				default :
					g_pclsLog->ERROR("unknown command");
					clsProto.Print(g_pclsLog, LV_DEBUG, true);
			}
		}

		// Address Lookup
		if (clsAddr->LookupAtom("ATOM_NM", nNodeId, nProcId) <= 0) {
			g_pclsLog->ERROR("lookup failed");
			continue;
		}

		// Send PING command
		clsProto.Clear();
		clsProto.SetCommand(CMD_PING);
		clsProto.SetFlagRequest();
		clsProto.SetSource(stOption.m_nNodeId, stOption.m_nProcId);
		clsProto.SetDestination(nNodeId, nProcId);
		nSeq = clsProto.SetSequence();
		if (clsIPC->SendMesg(clsProto) == false) {
			g_pclsLog->ERROR("send failed");
		}
		clsProto.Print(g_pclsLog, LV_DEBUG, true);
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
