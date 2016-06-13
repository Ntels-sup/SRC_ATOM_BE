/* vim:ts=4:sw=4
 */
/**
 * \file	NMMain.cpp
 * \brief	Node Manager 메인 function
 *			Node Agent 사이의 메세지 라우팅 및 NM 스레드 생성 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <csignal>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "CFileLog.hpp"
#include "CModule.hpp"
#include "CProtocol.hpp"
#include "CMesgExchSocket.hpp"
#include "CMesgExchSocketServer.hpp"
#include "Utility.hpp"

#include "CConfigNm.hpp"
#include "CDBInOut.hpp"
#include "CModuleManager.hpp"
#include "CModuleIPC.hpp"
#include "CNodeProcTB.hpp"
#include "CMesgRoute.hpp"
#include "CCommandNm.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::pair;
using std::vector;

// Global 변수
CFileLog* g_pclsLog = NULL;

// Static 변수

// function prototype
void* ThreadRouting(void* a_pArg);


void Usage(int argc, char* argv[])
{
	fprintf(stdout, "ATOM EMS Server\n");
	fprintf(stdout, "Usage) %s <options>\n", argv[0]);
	fprintf(stdout, "       options:\n");
	fprintf(stdout, "          -c <config file>      # default $(HOME)/CFG/ATOM.cfg\n");
	fprintf(stdout, "          -s <atom listen IP>   # default 0.0.0.0\n");
	fprintf(stdout, "          -p <atom listen Port> # default 8000\n");
	fprintf(stdout, "          -d <module path>      # defult HOME/BIN\n");
	fprintf(stdout, "          -m <loading module>   # default all, ex) ATOM_NM_PRA.so\n");	
	fprintf(stdout, "          -v                    # verbose mode\n");
	fprintf(stdout, "          -h                    # help\n");

	return;
}

bool Options(int argc, char* argv[])
{
	// 실행 argument 파싱
	int opt;

	char*	szCfgFile = NULL;
	char*	szListenIp = NULL;
	char*	szListenPort = NULL;
	char*	szModePath = NULL;
	char*	szLoadModule = NULL;
	bool	bVerbose = false;


	while ((opt = getopt(argc, argv, "c:s:p:d:m:vh")) != -1) {
		switch (opt) {
			case 'c':
				szCfgFile = optarg;
				break;
			case 's':
				szListenIp = optarg;
				break;
			case 'p':
				szListenPort = optarg;
				break;
			case 'd':
				szModePath = optarg;
				break;
			case 'm':
				szLoadModule = optarg;
				break;
			case 'v':
				bVerbose = true;
				break;
			case 'h':
				Usage(argc, argv);
				return false;
			default:
				cerr << "invalied options";
				Usage(argc, argv);
				return false;
		}
	}

	// config file 설정값 loading
	CConfigNm& clsCfg = CConfigNm::Instance();
   	if (clsCfg.LoadCfgFile(szCfgFile) == false) {
		return false;
	}
	
	if (szListenIp) clsCfg.m_strListenIp = szListenIp;
	if (szListenPort) clsCfg.m_nListenPort = atoi(szListenPort);
	if (szModePath) clsCfg.m_strModPath = szModePath;
	clsCfg.m_bVerbose = bVerbose;

	// Module probe dynamic(.so)
	vector<string> vecLoadModule;
	if (szLoadModule) {
		StringSplit(szLoadModule, ",", vecLoadModule);
	}
	if (clsCfg.ProbeModule(vecLoadModule) == false) {
		return false;
	}
	return true;
}

bool Initalize(void)
{
	CConfigNm& clsCfg = CConfigNm::Instance();

    // signal block set
	sigset_t stSigSet;
	sigfillset(&stSigSet);
	sigdelset(&stSigSet, SIGINT);
	sigdelset(&stSigSet, SIGSEGV);
	sigdelset(&stSigSet, SIGSTOP);
	sigdelset(&stSigSet, SIGTERM);
	sigdelset(&stSigSet, SIGKILL);
	sigprocmask(SIG_BLOCK, &stSigSet, NULL);

	// Log init
	g_pclsLog = new CFileLog();
	if (g_pclsLog->Initialize((char*)clsCfg.m_strLogPath.c_str(), 
							NULL, (char*)"NM", 0, LV_DEBUG) != 0) {
		cerr << "NM, log initalize failed" << endl;
		return false;
	}
	g_pclsLog->SetThreadLock();

	// process background running
	if (clsCfg.m_bVerbose == false) {
		daemon(1, 0);
	}

	clsCfg.DBSetup();

	// NM의 NodeNo, ProcNo 검색
	CDBInOut clsDB;
	clsCfg.m_nNodeNo = clsDB.FindNodeNo(clsCfg.m_strPkgName, clsCfg.m_strNodeType);
	if (clsCfg.m_nNodeNo <= 0) {
		g_pclsLog->ERROR("NM, not found node id");
		return false;
	}
	clsCfg.m_nProcNo = clsDB.FindProcNo(clsCfg.m_strPkgName, clsCfg.m_strNodeType,
														clsCfg.m_strProcName);
	if (clsCfg.m_nProcNo <= 0) {
		g_pclsLog->ERROR("NM, not found process id");
		return false;
	}

	// NM IPC 등록
	CModuleIPC& clsIpc = CModuleIPC::Instance();
	clsIpc.Regist(clsCfg.m_strProcName.c_str(), clsCfg.m_nProcNo);

	// NM Routing Table 등록
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	clsRoute.SetLocalNodeNo(clsCfg.m_nNodeNo);
	clsRoute.AddRouteModule(clsCfg.m_nNodeNo, clsCfg.m_nProcNo, clsCfg.m_nProcNo);

	// Address init
	CAddress& clsAddr = CAddress::Instance();
	clsAddr.SetDefaultPkg(clsCfg.m_strPkgName.c_str());
	clsAddr.SetDefaultNode(clsCfg.m_strNodeType.c_str(), clsCfg.m_nNodeNo);

	// Routing thread run
	pthread_t thid;
	if (pthread_create(&thid, NULL, ThreadRouting, NULL) != 0) {
		g_pclsLog->ERROR("NM, Routing thread failed");
		return false;
	}
	clsRoute.AddRouteModule(clsCfg.m_nNodeNo, PROCID_ATOM_NM_ROUTE, PROCID_ATOM_NM_ROUTE);
	clsIpc.Regist("ROUTE", PROCID_ATOM_NM_ROUTE);

	// Module loading dynamic(.so)
	CModuleManager& clsModMng = CModuleManager::Instance();
	for (size_t i=0; i < clsCfg.m_vecModule.size(); i++) {
		clsModMng.AddModule(clsCfg.m_vecModule[i]);
	}

	// Node, Process loading
	CNodeProcTB::Instance();

	clsCfg.ConfigPrint(g_pclsLog);
	clsRoute.PrintTable(g_pclsLog, LV_INFO);

	return true;
}

int main(int argc, char* argv[])
{
	if (Options(argc, argv) == false) {
		return 1;
	}
	if (Initalize() == false) {
		return 1;
	}

	CConfigNm& clsCfg = CConfigNm::Instance();
    
	// Server socket listen
	CMesgExchSocketServer clsServer;
	if (clsServer.Listen(clsCfg.m_strListenIp.c_str(), clsCfg.m_nListenPort) == false) {
		g_pclsLog->ERROR("NM, listen failed, IP: %s, PORT: %d",
						clsCfg.m_strListenIp.c_str(), clsCfg.m_nListenPort);
		return 1;
	}

	// 모듈을 실행
    CModuleManager& clsModMng = CModuleManager::Instance();
    clsModMng.ActiveModule();

	map<string, CModule*>::iterator mapIter;
	CProtocol clsProto;
	CModuleIPC& clsIPC = CModuleIPC::Instance();
	CSocket* pclsClient = NULL;
	CCommandNm clsCmd;
	int nCnt = 0;

	while (true) {

		// 새로운 연결 대기
		pclsClient = clsServer.Select(0, 100);		// wait 500 msec
		if (pclsClient == NULL) {
			switch (clsServer.m_enError) {
				case CSocket::SOCK_OK :
				case CSocket::SOCK_TIMEOUT :
        		case CSocket::SOCK_INTERRUPT :
					continue;
				default :
					g_pclsLog->ERROR("NM, Server socket, select failed, m_enError: %d",
										clsServer.m_enError);
					sleep(1);
					continue;
			}
		}
		
		// NM이 처리할 메세지
		
		nCnt = clsServer.RecvMesg(pclsClient, &clsProto, -1);
		if (nCnt == 1) {
			clsCmd.MesgProcess(clsProto, clsServer, pclsClient);
		} else {
			if (!clsServer.CSocketServer::m_strErrorMsg.empty()) {
				g_pclsLog->ERROR("NM, %s", clsServer.CSocketServer::m_strErrorMsg.c_str());
			}
			if (!clsServer.CProtocol::m_strErrorMsg.empty()) {
				g_pclsLog->ERROR("NM, %s", clsServer.CProtocol::m_strErrorMsg.c_str());
			}
			g_pclsLog->ERROR("NM, client socket close, ret:%d, fd: %d", 
														nCnt, pclsClient->GetSocket());
			clsServer.ClosePeer(pclsClient);
		}
		
        // Module 중 스레드가 아닌 function 형태의 모듈 실행
        clsModMng.ActiveModule();

		// Module간 메세지 수신 처리
		nCnt = clsIPC.RecvMesg(clsCfg.m_nProcNo, clsProto, -1);
		if (nCnt == 1) {
			clsCmd.MesgProcess(clsProto, clsServer, NULL);
		}
	}

	// Module stop - function 
	clsModMng.StopModule();

	return 0;
}
