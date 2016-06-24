/* vim:ts=4:sw=4
 */
/**
 * \file	NAMain.cpp
 * \brief	Node Agent 메인 function
 *			Agent thread 및 function을 초기화 하고 실행
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <map>
#include <string>

//common header
#include "CFileLog.hpp"
#include "CMesgExchSocket.hpp"
#include "CommandFormat.hpp"

//NA hader
#include "CConfigNa.hpp"
#include "CModule.hpp"
#include "CModuleManager.hpp"
#include "CCmdRegistNode.hpp"
#include "CAddress.hpp"
#include "CCommandNa.hpp"
#include "Utility.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;

static CMesgExchSocket clsMsgEx;

CFileLog* g_pclsLog = NULL;


 
void Usage(int argc, char* argv[])
{
	fprintf(stdout, "ATOM Node Agent\n");
	fprintf(stdout, "Usage) %s <options>\n", argv[0]);
	fprintf(stdout, "       options:\n");
	fprintf(stdout, "          -c <config file>      # default $(HOME)/CFG/ATOM.cfg\n");
	fprintf(stdout, "          -s <atom server IP>   # default 127.0.0.1\n");
	fprintf(stdout, "          -p <atom server Port> # default 8000\n");
	fprintf(stdout, "          -K <package name>     # ex) VOFCS\n");
	fprintf(stdout, "          -N <node type>        # ex) AP\n");
	fprintf(stdout, "          -P <process name>     # ex) ATOM_NA\n");
	fprintf(stdout, "          -d <module path>      # defult HOME/BIN\n");
	fprintf(stdout, "          -m <loading module>   # default all, ex) ATOM_NA_PRA.so\n");
	fprintf(stdout, "          -u <UUID>             # ex) 3d54d4ec-0bc3-...\n");	
	fprintf(stdout, "          -h                    # help\n");
	fprintf(stdout, "\n");
	
	return;
}

bool Options(int argc, char* argv[])
{
	int opt;

	// arguemnt
	char*	szCfgFile = NULL;
	char*	szModPath = NULL;
	char*	szUuid = NULL;
	char*	szLoadModule = NULL; 

	while ((opt = getopt(argc, argv, "c:d:m:u:h")) != -1) {
		switch (opt) {
			case 'c':				// config path
				szCfgFile = optarg;
			   	break;
			case 'd':				// module path
				szModPath = optarg;
				break;
			case 'm':				// loading module
				szLoadModule = optarg;
				break;
			case 'u':				// UUID
				szUuid = optarg;
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

	CConfigNa& clsCfg = CConfigNa::Instance();

	// ATOM config 파일을 loading
   	if (clsCfg.LoadCfgFile(szCfgFile) == false) {
		fprintf(stderr, "NA, config file loading failed\n");
		return false;
	}
	
	// VNF환경에서 EMS IP loading
	if (clsCfg.m_bIsVnfMode) {
		clsCfg.ProbeVnfProperties();
	}

	
	// manual 입력된 값으로 재설정
	if (szModPath) {
		clsCfg.m_strModPath = szModPath;
	}
	if (szUuid) {
		clsCfg.m_strUuid = szUuid;
	}
	// Module probe dynamic(.so)
	vector<string> vecLoadModule;
	if (szLoadModule) {
		StringSplit(szLoadModule, ",", vecLoadModule);
	}
	if (clsCfg.ProbeModule(vecLoadModule) == false) {
        fprintf(stderr, "NA, module loading failed\n");
		return false;
	}

	return true;
}

bool Initalize(void)
{
	CConfigNa& clsCfg = CConfigNa::Instance();

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
										NULL, (char*)"NA", 0, LV_DEBUG) != 0) {
		cerr << "Log initalize failed" << endl;
		return false;
	}

	// DB instance 생성
	if (CConfigNa::Instance().DBSetup() == false) {
        g_pclsLog->ERROR("NA, DB connect failed");
		return false;
	}

	// NA node process id
	if (clsCfg.m_nProcNo <= 0) {
		clsCfg.m_nProcNo = clsCfg.ProbeProcNo(clsCfg.m_strPkgName.c_str(), 
							clsCfg.m_strNodeType.c_str(), clsCfg.m_strProcName.c_str());
		if (clsCfg.m_nProcNo <= 0) {
        	g_pclsLog->ERROR("NA, process no get failed");
			return false;
		}
	}

	// Routing init
	CMesgRoute& clsRoute = CMesgRoute::Instance();
	clsRoute.SetLocalNodeNo(clsCfg.m_nNodeNo);
	clsRoute.SetDefaultProcNo(clsCfg.m_nProcNo);
	clsRoute.AddRoute(clsCfg.m_nProcNo, clsCfg.m_nProcNo);

	// Address init
	CAddress& clsAddr = CAddress::Instance();
    clsAddr.SetLocalPkg(clsCfg.m_strPkgName, clsCfg.m_strNodeType);
	clsAddr.SetLocalNodeNo(clsCfg.m_nNodeNo);
	clsAddr.SetNaProcNo(clsCfg.m_nProcNo);
	if (clsAddr.SetAtomAddress() == false) {
        g_pclsLog->ERROR("NA, address preload failed");
		return false;
	}

	// Thread IPC init
	CModuleIPC& clsIpc = CModuleIPC::Instance();
	clsIpc.Regist(clsCfg.m_strProcName.c_str(), clsCfg.m_nProcNo);

	// Module loading dynamic(.so)
	CModuleManager& clsModMng = CModuleManager::Instance();
	for (size_t i=0; i < clsCfg.m_vecModule.size(); i++) {
		clsModMng.AddModule(clsCfg.m_vecModule[i]);
	}

	clsCfg.ConfigPrint(g_pclsLog);
	clsRoute.PrintTable(g_pclsLog, LV_INFO);
	clsIpc.PrintTable(g_pclsLog, LV_INFO);
	clsAddr.PrintTable(g_pclsLog, LV_INFO);

	return true;
}

//가상화 환경에서 NA 기동 후 VNFM으로 부터 UUID등의 시스템 환경정보를
//얻는다.
bool VNFStandby(void)
{
	CConfigNa& clsCfg = CConfigNa::Instance();
	CModuleIPC& clsIpc = CModuleIPC::Instance();
	CModuleManager& clsModMng = CModuleManager::Instance();

	// VNFM 연동을 위한 관련 모듈 실행	
	if (clsModMng.ActiveModule("ATOM_NA_EXA") == false) {
		g_pclsLog->CRITICAL("Modeule 'ATOM_NA_EXA' running failed");
		return false;
	}

	int			nNaProcNo  = clsCfg.m_nProcNo;
	CProtocol	clsProto;
	bool		bRet = false;

	CCommandNa	clsNaCmd;
	clsNaCmd.PermitCommand(CMD_VNF_PRA_READY);
	clsNaCmd.PermitCommand(CMD_VNF_PRA_INSTALL);
	clsNaCmd.PermitCommand(CMD_VNF_PRA_START);

	bool	bLoop = true;
	
    g_pclsLog->INFO("NA, VNF initalize startup");

	do {
		// EXT Module 메세지 수신 처리
		if (clsIpc.RecvMesg(nNaProcNo, clsProto, clsCfg.m_nVnfWaitTime) > 0) {

			bRet = clsNaCmd.ProcMesgModule(clsProto, NULL);
			switch (clsNaCmd.m_nLastCmd) {
				case CMD_VNF_PRA_READY :
					if (bRet == false) {
						return false;
					}
    				g_pclsLog->INFO("NA, VNF ready complete");
					break;
				case CMD_VNF_PRA_INSTALL :
					if (bRet) {
						bLoop = false;
					} else {
						return false;
					}
    				g_pclsLog->INFO("NA, VNF install complete");
					break;
				case CMD_VNF_PRA_START :
					if (bRet) {
						bLoop = false;
					} else {
						return false;
					}
    				g_pclsLog->INFO("NA, VNF start complete");
					break;
                default:
				    g_pclsLog->WARNING("only accept VNF_READY message, command: %d",
																clsNaCmd.m_nLastCmd);
			}
		} else {
			g_pclsLog->ERROR("NA, VNF wait timeout, READY and (INSTALL or START");
			return false;
		}
		
	} while (bLoop);

    g_pclsLog->INFO("NA, VNF initalize complete");

	return true;
}

// ATOM Server connect
bool NMConnect(CMesgExchSocket& a_clsMsgEx)
{
	if (a_clsMsgEx.IsConnected()) {
		return true;
	}

	// NA connect delay
	// 최소 5초에 한번씩만 연결 시도, 연결 실패 로그 과다 생성 방지
	static time_t tLastTryTime = 0;
	if (time(NULL) - tLastTryTime <= 5) {
		return false;
	}
	tLastTryTime = time(NULL);

	CConfigNa& clsCfg = CConfigNa::Instance();

	// UUID 확인
	if (clsCfg.m_strUuid.empty()) {
		g_pclsLog->ERROR("UUID empty, please set uuid");
		return false;
	}
	
	if (a_clsMsgEx.Connect(clsCfg.m_strServIp.c_str(), clsCfg.m_nServPort) == false) {
		g_pclsLog->ERROR("ATOM Server connect failed");
		a_clsMsgEx.Close();
		return false;
	}
	
 	// 인증 정보 전송
	CCmdRegistNode clsRegist;
	clsRegist.m_strPkgName	= clsCfg.m_strPkgName; 
	clsRegist.m_strNodeType	= clsCfg.m_strNodeType;
	clsRegist.m_strProcName	= clsCfg.m_strProcName;
	clsRegist.m_strNodeIp	= clsCfg.m_strNodeIp;
	clsRegist.m_strUuid		= clsCfg.m_strUuid;
	clsRegist.m_nProcNo		= clsCfg.m_nProcNo;
	clsRegist.m_strVersion	= clsCfg.m_strVersion;

	a_clsMsgEx.Clear();
	a_clsMsgEx.SetCommand(CMD_REGIST_NODE);
	a_clsMsgEx.SetFlagRequest();
	a_clsMsgEx.SetSource(0, clsCfg.m_nProcNo);
	a_clsMsgEx.SetDestination(0, 0);
	a_clsMsgEx.SetSequence();
	a_clsMsgEx.SetPayload(clsRegist.RequestGen());

	if (a_clsMsgEx.SendMesg() == false) {
		g_pclsLog->ERROR("NA, 'CMD_REGIST_NODE' send failed");
		a_clsMsgEx.Close();
		return false;
	}

	#ifdef NA_DEBUG
	g_pclsLog->DEBUG("SEND Message");
	a_clsMsgEx.Print(g_pclsLog, LV_DEBUG, true);
	#endif

	a_clsMsgEx.Clear();
	if (a_clsMsgEx.RecvMesg(NULL, 5) == 1) {
		if (a_clsMsgEx.IsFlagError()) {
			clsRegist.ErrorParse(a_clsMsgEx.GetPayload().c_str());
			g_pclsLog->ERROR("NA, 'CMD_REGIST_MODE' error response, code: %d, text: %s",
								clsRegist.m_nErrorCode, clsRegist.m_strErrorText.c_str());
			a_clsMsgEx.Close();
			return false;
		}
		if (clsRegist.ResponseParse(a_clsMsgEx.GetPayload().c_str()) == false) {
			a_clsMsgEx.Close();
			a_clsMsgEx.Print(g_pclsLog, LV_DEBUG, true);
			return false;
		}

		// 변경된 NodeNo 정보를 재 설정
		CConfigNa& clsCfg = CConfigNa::Instance();
		clsCfg.m_nNodeNo		= clsRegist.m_nNodeNo;
		clsCfg.m_strNodeName	= clsRegist.m_strNodeName;
		
		CMesgRoute& clsRoute = CMesgRoute::Instance();
		clsRoute.SetLocalNodeNo(clsRegist.m_nNodeNo);
		clsRoute.DelRoute(clsCfg.m_nProcNo);
		clsRoute.AddRoute(clsCfg.m_nProcNo, clsCfg.m_nProcNo);
		clsRoute.PrintTable(g_pclsLog, LV_INFO);

		CAddress& clsAddr = CAddress::Instance();
		clsAddr.SetLocalNodeNo(clsRegist.m_nNodeNo);
		clsAddr.PrintTable(g_pclsLog, LV_INFO);
		
		CModuleManager& clsModMng = CModuleManager::Instance();
		clsModMng.SetOptNodeNo(clsRegist.m_nNodeNo);
		clsModMng.SetOptNodeName(clsRegist.m_strNodeName);

	} else {
		g_pclsLog->ERROR("NA, 'CMD_REGIST_NODE' receive failed");
		a_clsMsgEx.Close();
		return false;
	}

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

	// VNF 환경을 위한 초기 정보 및 NA 실행 환경 설정
	if (CConfigNa::Instance().m_bIsVnfMode) {
		if (VNFStandby() == false) {
			return 1;
		}
	}

	NMConnect(clsMsgEx);

	// 실행되지 않은 모듈을 기동 시킨다.
	CModuleManager& clsModMng = CModuleManager::Instance();
	clsModMng.ActiveModule();


	CModuleIPC&	clsIPC = CModuleIPC::Instance();
	int 		nNaProcNo = CConfigNa::Instance().m_nProcNo;
	CProtocol	clsProto;
	CCommandNa	clsNaCmd;
	bool		bIdle = true;
	int			nCnt = 0;

	while (true) {
		NMConnect(clsMsgEx);

		// Socket 메세지 수신 처리
		nCnt = clsMsgEx.RecvMesg(&clsProto, -1);
		if (nCnt == 1) {
			clsNaCmd.ProcMesgSock(clsProto, &clsMsgEx);
			bIdle = false;
		} else if (nCnt < 0) {
			clsMsgEx.Close();
		}
		
		// Thread 메세지 수신 처리
		if (clsIPC.RecvMesg(nNaProcNo, clsProto, -1) > 0) {
			clsNaCmd.ProcMesgModule(clsProto, &clsMsgEx);
			bIdle = false;
		}

        // Module 중 스레드가 아닌 function 형태의 모듈 실행
        clsModMng.ActiveModule();

		// 처리한 메세지가 없다면 sleep		
		if (bIdle) {
			usleep(3000);
		}
		bIdle = true;
	}

    // Module stop
	clsModMng.StopModule();

	return 0;
}
