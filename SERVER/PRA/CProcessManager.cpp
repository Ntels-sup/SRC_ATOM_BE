/* vim:ts=4:sw=4
 */
/**
 * \file	CProcessManager.cpp
 * \brief	APP 프로세스 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <algorithm>

// common header
#include "CFileLog.hpp"
#include "CommandFormat.hpp"

// PRA header
#include "CConfigPra.hpp"
#include "CProcStatus.hpp"
#include "CProcessManager.hpp"
#include "CCmdAppCtl.hpp"

using std::pair;
using std::list;
using std::sort;
using std::vector;
using std::string;
using std::make_pair;

extern CFileLog* g_pclsLogPra;


CProcessManager::CProcessManager()
{
	m_nRestartClearTime = 60;			// 자동 재실행 횟수 초기화 시간
	m_nHangupTime = 60;					// Hangup 기준 시간
	m_nAlivePeriod = 30;				// Alive check 주기	

	return;
}

CProcessManager::~CProcessManager()
{
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		delete iter->second.m_pclsApp;
	}

	return;
}

bool CProcessManager::Add(ST_APPINFO& a_stAppInfo)
{
	// ATOM에서 기본 제공하는 argument, envionment 설
	SetAtomEnvArg(a_stAppInfo);

	auto iter = m_mapProcess.find(a_stAppInfo.m_nProcNo);
	if (iter == m_mapProcess.end()) {
		pair<int, ST_PROCESS> v(a_stAppInfo.m_nProcNo, ST_PROCESS());
		pair<proc_t::iterator, bool> ins = m_mapProcess.insert(v);
		if (ins.second == false) {
			g_pclsLogPra->ERROR("PM, map insert failed, process: %s", 
												a_stAppInfo.m_strProcName.c_str());
			return false;
		}

		ins.first->second.m_stAppInfo = a_stAppInfo;
		ins.first->second.m_pclsApp = new CProcess();
		ins.first->second.m_nRunCount = 0;
		ins.first->second.m_nAliveTime = time(NULL);

		iter = ins.first;
	}

	iter->second.m_pclsApp->m_stSetup.m_strProcName = a_stAppInfo.m_strProcName;
	iter->second.m_pclsApp->m_stSetup.m_strExeBin = a_stAppInfo.m_strExeBin;
	iter->second.m_pclsApp->m_stSetup.m_strExeArg = a_stAppInfo.m_strExeArg;
	iter->second.m_pclsApp->m_stSetup.m_strExeEnv = a_stAppInfo.m_strExeEnv;
	iter->second.m_pclsApp->m_stSetup.m_strCpuBind = a_stAppInfo.m_strCpuBind;
	iter->second.m_pclsApp->m_stSetup.m_strMmrBind = a_stAppInfo.m_strMmrBind;

	return true;
}

// 등록된 프로세스 실행
void CProcessManager::Run(int a_nProcNo)
{
	vector<proc_t::iterator> P;
	OrderStart(P);

	// REPACK
	// 특정 프로세스 만을 대상으로 하는 경우에도 for loop를 타야 한다.

	for (size_t i=0; i < P.size(); ++i) {
		if (a_nProcNo > 0 && a_nProcNo != P[i]->second.m_stAppInfo.m_nProcNo) {
			continue;
		}
		
		if (P[i]->second.m_pclsApp->IsRunning()) {
			// 중복 실행 방지
			continue;
		}
		
		// 프로세스 재실행 횟수 초기화
		// 만약 재실행 횟수 3이라고 했을때
		// 1달에 1번 정도 비정상 종료하여 재실행 했다면 3달 후에는 프로세스 재실행
		// 되지않는다.
		// 따라서 실행 특정 시간 경과하면 재 실행 횟수를 초기화 해준다.
		if (P[i]->second.m_pclsApp->m_stRunInfo.m_tStartTime + m_nRestartClearTime < time(NULL)) {
			P[i]->second.m_nRunCount = 0;
		}

		// 재실행 횟수 체크
		if (P[i]->second.m_nRunCount >= P[i]->second.m_stAppInfo.m_nRestartCnt) {
			g_pclsLogPra->WARNING("PM, process execute count over, pno: %d, pname: %s, limit: %d",
										P[i]->second.m_stAppInfo.m_nProcNo,
										P[i]->second.m_stAppInfo.m_strProcName.c_str(),
										P[i]->second.m_stAppInfo.m_nRestartCnt);
			if (a_nProcNo > 0) {
				break;
			} else {
				continue;
			}
		}

		if (P[i]->second.m_stAppInfo.m_bIsUse && P[i]->second.m_stAppInfo.m_bIsExec) {

			if (P[i]->second.m_pclsApp->Execute()) {
				P[i]->second.m_pclsApp->CoreDumpFilter(CProcess::LV0_ANONY_PRIVATE);
				g_pclsLogPra->INFO("PM, process execute ok, pname: %s",
										P[i]->second.m_stAppInfo.m_strProcName.c_str());
			} else {
				g_pclsLogPra->INFO("PM, process execute failed, pname: %s",
										P[i]->second.m_stAppInfo.m_strProcName.c_str());
			}

			P[i]->second.m_nRunCount++;
		}

		// 특정 프로세스를 실행 했다면 loop를 빠져 나간다.
		if (a_nProcNo > 0) {
			break;
		}
	}
	
	return;
}

void CProcessManager::Stop(int a_nProcNo)
{
	if (a_nProcNo > 0) {
		auto iter = m_mapProcess.find(a_nProcNo);
		if (iter != m_mapProcess.end()) {
			iter->second.m_pclsApp->Terminate();
		}
		return;
	} else if (a_nProcNo == 0) {
		return;
	}
	
	// 종료 순서대로 프로세스 정렬
	vector<proc_t::iterator> vecStop;
	OrderStop(vecStop);

	auto iter = vecStop.begin();
	for (size_t i=0; i < vecStop.size(); i++) {
		vecStop[i]->second.m_pclsApp->Terminate();
	}
	
	return;
}

void CProcessManager::StopForceibly(int a_nProcNo)
{
	if (a_nProcNo > 0) {
		auto iter = m_mapProcess.find(a_nProcNo);
		if (iter != m_mapProcess.end()) {
			iter->second.m_pclsApp->Kill();
		}
		return;
	} else if (a_nProcNo == 0) {
		return;
	}

	// 종료 순서대로 프로세스 정렬
	vector<proc_t::iterator> vecStop;
	OrderStop(vecStop);
	
	auto iter = vecStop.begin();
	for (size_t i=0; i < vecStop.size(); i++) {
		vecStop[i]->second.m_pclsApp->Kill();
	}

	return;
}

// 프로세스를 사용하지 않도록 설정
void CProcessManager::Disable(int a_nProcNo)
{
	// 특정 프로세스 지정한 경우.
	// 프로세스 실행 여부와 상관없이 use 플래그를 disable 한다.
	// disable 된 프로세스는 비정상 종료 되어야 자동 실행 되지 않는다. 
	if (a_nProcNo > 0) {
		auto iter = m_mapProcess.find(a_nProcNo);
		if (iter != m_mapProcess.end()) {
			iter->second.m_stAppInfo.m_bIsUse = false;
		}
		return;
	} else if (a_nProcNo == 0) {
		return;
	}
	
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); iter++) {
		iter->second.m_stAppInfo.m_bIsUse = false;	
	}
	
	return;
}

bool CProcessManager::IsRunning(int a_nProcNo)
{
	auto iter = m_mapProcess.find(a_nProcNo);
	if (iter == m_mapProcess.end()) {
		return false;
	}
	
	if (iter->second.m_pclsApp->m_stRunInfo.m_enStatus != CProcStatus::RUNNING) {
		return false;
	}

	return true;
}

void CProcessManager::AliveOk(int a_nProcNo)
{
	auto iter = m_mapProcess.find(a_nProcNo);
	if (iter != m_mapProcess.end()) {
		iter->second.m_nAliveTime = time(NULL);
	}
}

void CProcessManager::CheckAlive(void)
{
	CConfigPra& clsCfg = CConfigPra::Instance();
	
	// Alive check를 위한 Ping 메세지 생성
	CProtocol clsProto;
	clsProto.SetCommand(CMD_PING);
	clsProto.SetFlagRequest();
	clsProto.SetSource(clsCfg.m_nNodeNo, clsCfg.m_nProcNo);
	clsProto.SetPayload(NULL, 0);
	
	time_t curtime = time(NULL);

	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); iter++) {

		if (iter->second.m_pclsApp->GetStatus() != CProcStatus::RUNNING) {
			continue;
		}
		
		// Alive 체크
		// 조건1) 마지막 alive 시간 + hangup 시간 이내 인가?
		// 조건2) alive 체크 주기 인가?
		if (curtime <= (iter->second.m_nAliveTime + m_nHangupTime)) {
			if (curtime >= (iter->second.m_nAliveTime + m_nAlivePeriod)) {
				clsProto.SetDestination(clsCfg.m_nNodeNo, iter->first);
				clsProto.SetSequence();
				
				auto miter = m_mapProcess.find(iter->first);
				if (miter == m_mapProcess.end()) {
					continue;
				}
				
				//TODO
				//send ping message
			}
			continue;
		}

		// HANGUP 처리
		g_pclsLogPra->CRITICAL("PM, process hangup, pno: %d, pname: %s",
							iter->first, 
							iter->second.m_stAppInfo.m_strProcName.c_str());
		iter->second.m_pclsApp->SendSignal(SIGSEGV);
		iter->second.m_pclsApp->SetStatus(CProcStatus::HANGUP);
	}
	
	return;
}

void CProcessManager::CheckExit(int a_nProcNo)
{
	pid_t nChild = 0;

	if (a_nProcNo > 0) {
		auto iter = m_mapProcess.find(a_nProcNo);
		if (iter != m_mapProcess.end()) {
			g_pclsLogPra->ERROR("PM, not found process, procno: %d", a_nProcNo);
			return;
		}
		nChild = iter->second.m_pclsApp->GetPid();
		if (nChild <= 0) {
			return;
		}
	}

	proc_t::iterator miter; 
	CProcStatus::EN_STATUS enStat;
	siginfo_t tExitInfo;

	while (true) {
		tExitInfo.si_pid = 0;

		if (nChild > 0) {
			nChild = waitid(P_PID, nChild, &tExitInfo, WEXITED | WNOHANG | WNOWAIT);
		} else {
			nChild = waitid(P_ALL, 0, &tExitInfo, WEXITED | WNOHANG | WNOWAIT);
		}

		if (nChild < 0) {
			if (errno == ECHILD) {
				break;
			}
			g_pclsLogPra->CRITICAL("PM, waitid failed, error: %d(%s)", 
														errno, strerror(errno));
			break;
		}
		if (tExitInfo.si_pid == 0) {
			break;
		}

		miter = FindByPid(tExitInfo.si_pid);
		if (miter == m_mapProcess.end()) {
			nChild = waitid(P_PID, tExitInfo.si_pid, NULL, WEXITED | WNOHANG);
			g_pclsLogPra->CRITICAL("PM, not found process, pid: %d",
				   					tExitInfo.si_pid, errno, strerror(errno));
			break;
		}
		
		enStat = miter->second.m_pclsApp->ExitCheck();
		if (enStat == CProcStatus::ABNORMAL) {
			Run(miter->second.m_stAppInfo.m_nProcNo);
		}
	}
	
	return;
}

void CProcessManager::CheckManualRun(void)
{
	//TODO
	return;
}

const char* CProcessManager::GetProcName(int a_nProcNo, string* a_pstrProcName)
{
	auto iter = m_mapProcess.find(a_nProcNo);
	if (iter == m_mapProcess.end()) {
		return NULL;
	}
	
	if (a_pstrProcName != NULL) {
		*a_pstrProcName = iter->second.m_stAppInfo.m_strProcName;
		return a_pstrProcName->c_str();
	}

	return iter->second.m_stAppInfo.m_strProcName.c_str();
}

CProcStatus::EN_STATUS CProcessManager::GetStatus(int a_nProcNo)
{
	CProcStatus::EN_STATUS nStatus = CProcStatus::NONE;
	
	auto iter = m_mapProcess.find(a_nProcNo);
	if (iter != m_mapProcess.end()) {
		nStatus = iter->second.m_pclsApp->GetStatus();
	}

	return nStatus;
}

int CProcessManager::GetAppInfo(vector<ST_APPINFO>& a_vecInfo, int a_nProcNo)
{
	int nCnt = 0;
	a_vecInfo.reserve(m_mapProcess.size());
	
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		if (iter->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		if (a_nProcNo > 0) {
			if (iter->second.m_stAppInfo.m_nProcNo != a_nProcNo) {
				continue;
			}
		}
		
		a_vecInfo.push_back(iter->second.m_stAppInfo);
		nCnt++;
	}

	return nCnt;
}

int CProcessManager::GetAppInfo(vector<ST_APPINFO>& a_vecInfo, 
											CProcStatus::EN_STATUS a_nStatus)
{
	int nCnt = 0;
	a_vecInfo.reserve(m_mapProcess.size());
	
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		if (iter->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		if (a_nStatus != CProcStatus::NONE) {
			if (iter->second.m_pclsApp->GetStatus() != a_nStatus) {
				continue;
			}
		}
		
		a_vecInfo.push_back(iter->second.m_stAppInfo);
		nCnt++;
	}

	return nCnt;
}

int CProcessManager::GetAppInfoStopOrder(vector<ST_APPINFO>& a_vecInfo,
												CProcStatus::EN_STATUS a_nStatus)
{
	vector<proc_t::iterator> P;
	OrderStop(P);

	a_vecInfo.reserve(m_mapProcess.size());
	
	for (size_t i=0; i < P.size(); i++) {
		if (P[i]->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		if (a_nStatus != CProcStatus::NONE) {
			if (P[i]->second.m_pclsApp->GetStatus() != a_nStatus) {
				continue;
			}
		}
		
		a_vecInfo.push_back(P[i]->second.m_stAppInfo);
	}

	return (int)a_vecInfo.size();
}

int	CProcessManager::GetRunInfo(vector<CProcess::ST_RUNINFO>& a_vecInfo, int a_nProcNo)
{
	int nCnt = 0;
	a_vecInfo.reserve(m_mapProcess.size());
	
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		if (iter->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		if (a_nProcNo > 0) {
			if (iter->second.m_stAppInfo.m_nProcNo != a_nProcNo) {
				continue;
			}
		}
		
		a_vecInfo.push_back(iter->second.m_pclsApp->m_stRunInfo);
		nCnt++;
	}

	return nCnt;
}

int CProcessManager::GetRunInfo(vector<CProcess::ST_RUNINFO>& a_vecInfo, 
											CProcStatus::EN_STATUS a_nStatus)
{
	int nCnt = 0;
	a_vecInfo.reserve(m_mapProcess.size());
	
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		if (iter->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		if (a_nStatus != CProcStatus::NONE) {
			if (iter->second.m_pclsApp->GetStatus() != a_nStatus) {
				continue;
			}
		}
		
		a_vecInfo.push_back(iter->second.m_pclsApp->m_stRunInfo);
		nCnt++;
	}

	return nCnt;
}

int CProcessManager::GetStatus(list<ST_APPRUNINFO>& a_lstInfo, int a_nProcNo)
{
	vector<proc_t::iterator> P;
	OrderStatus(P);

	list<ST_APPRUNINFO>::iterator liter;


	for (size_t i=0; i < P.size(); ++i) {
		if (a_nProcNo > 0 && a_nProcNo != P[i]->second.m_stAppInfo.m_nProcNo) {
			continue;
		}
		
		if (P[i]->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		
		liter = a_lstInfo.insert(a_lstInfo.end(), ST_APPRUNINFO());
		liter->m_stInfo = P[i]->second.m_stAppInfo;
		liter->m_stRun = P[i]->second.m_pclsApp->m_stRunInfo;

		// 특정 프로세스의 상태 정보를 얻었다면 loop를 빠져 나간다.
		if (a_nProcNo > 0) {
			break;
		}
	}

	return (int)a_lstInfo.size();
}

// 실행 중인 프로세스 중에서 특정 상태인 프로세스 목록을 얻는다.
// 상태를 지정하지 않으면(-1) 가장 안좋은(worst) 프로세스 찾는다.
int CProcessManager::GetStatus(list<ST_APPRUNINFO>& a_lstInfo,
												CProcStatus::EN_STATUS a_nStatus)
{
	// 최악의 상태를 찾는다.
	if (a_nStatus == CProcStatus::NONE) {
		vector<CProcStatus::EN_STATUS> vecStatus;
		ExtracStatus(vecStatus);

		CProcStatus clsStatus;
		a_nStatus = clsStatus.FindWorstStatus(vecStatus);
	}

	vector<proc_t::iterator> P;
	OrderStatus(P);

	list<ST_APPRUNINFO>::iterator liter;

	for (size_t i=0; i < P.size(); ++i) {
		if (P[i]->second.m_stAppInfo.m_bIsBatch) {
			continue;
		}
		if (P[i]->second.m_pclsApp->GetStatus() == a_nStatus) {
			liter = a_lstInfo.insert(a_lstInfo.end(), ST_APPRUNINFO());
			liter->m_stInfo = P[i]->second.m_stAppInfo;
			liter->m_stRun = P[i]->second.m_pclsApp->m_stRunInfo;
		} else {
			// 상태 종류별로 정렬되어 있기 때문에 상태가 다른 프로세스가 확인되면
			// 불필요한 loop를 멈춘다.
			if (!a_lstInfo.empty()) {
				break;
			}
		}
	}

	return (int)a_lstInfo.size();
}

// 등록된 프로세스 중에서 가장 안좋은(worst) 상태를 얻는다.
CProcStatus::EN_STATUS CProcessManager::GetStatusWorst(void)
{
	vector<CProcStatus::EN_STATUS> vecStatus;
	ExtracStatus(vecStatus);

	// 최악의 상태를 찾는다.
	CProcStatus clsStatus;
	return clsStatus.FindWorstStatus(vecStatus);
}

int CProcessManager::GetExitCode(int a_nProcNo)
{
	auto iter = m_mapProcess.find(a_nProcNo);
	if (iter == m_mapProcess.end()) {
		return -1;
	}

	return iter->second.m_pclsApp->m_stRunInfo.m_nExitCode;
}

int	CProcessManager::FindProcNo(const std::string& a_strProcName)
{
	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		if (iter->second.m_stAppInfo.m_strProcName == a_strProcName) {
			return iter->first;
		}
	}

	return -1;
}

void CProcessManager::SetAtomEnvArg(ST_APPINFO& a_stAppInfo)
{
	// 프로세스 환경변수에 NodeNo, ProcNo 추가
	char szBuf[80] = {0x00,};
	snprintf(szBuf, sizeof(szBuf), " ATOM_NODENO=%d ATOM_PROCNO=%d ATOM_CFG_HOME=%s",
					CConfigPra::Instance().m_nNodeNo, 
					a_stAppInfo.m_nProcNo,
					CConfigPra::Instance().m_strCfgFile.c_str());
	a_stAppInfo.m_strExeEnv += szBuf;
	
	// 프로세스 기본 argument 설정
	// <pkg name> <svc name> <node type> <node name> <process name>
	snprintf(szBuf, sizeof(szBuf), "%s %s %s %s %s ",
									a_stAppInfo.m_strPkgName.c_str(),
									a_stAppInfo.m_strSvcName.c_str(),
									a_stAppInfo.m_strNodeType.c_str(),
									CConfigPra::Instance().m_strNodeName.c_str(),
									a_stAppInfo.m_strProcName.c_str());
	
	string strBack = a_stAppInfo.m_strExeArg;
	a_stAppInfo.m_strExeArg = szBuf;
	a_stAppInfo.m_strExeArg += strBack;
	
	return;
}

bool CProcessManager::StartCmp(const proc_t::iterator L, const proc_t::iterator R)
{
	return (L->second.m_stAppInfo.m_nStartOrder < R->second.m_stAppInfo.m_nStartOrder);
}
void CProcessManager::OrderStart(vector<proc_t::iterator>& a_vecStart)
{
	a_vecStart.reserve(m_mapProcess.size());

	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		a_vecStart.push_back(iter);
	}

	sort(a_vecStart.begin(), a_vecStart.end(), CProcessManager::StartCmp);

	return;
}

bool CProcessManager::StopCmp(const proc_t::iterator L, const proc_t::iterator R)
{
	return (L->second.m_stAppInfo.m_nStopOrder < R->second.m_stAppInfo.m_nStopOrder);
}
void CProcessManager::OrderStop(vector<proc_t::iterator>& a_vecStop)
{
	a_vecStop.reserve(m_mapProcess.size());

	auto iter = m_mapProcess.begin();
	for (; iter != m_mapProcess.end(); ++iter) {
		a_vecStop.push_back(iter);
	}

	sort(a_vecStop.begin(), a_vecStop.end(), CProcessManager::StopCmp);

	return;
}

bool CProcessManager::StatusCmp(const proc_t::iterator L, const proc_t::iterator R)
{
	return (L->second.m_pclsApp->GetStatus() < R->second.m_pclsApp->GetStatus());
}
void CProcessManager::OrderStatus(vector<proc_t::iterator>& a_vecStatus)
{
	a_vecStatus.reserve(m_mapProcess.size());

	for (auto iter = m_mapProcess.begin(); iter != m_mapProcess.end(); ++iter) {
		a_vecStatus.push_back(iter);
	}

	sort(a_vecStatus.begin(), a_vecStatus.end(), CProcessManager::StatusCmp);

	return;
}

void CProcessManager::ExtracStatus(std::vector<CProcStatus::EN_STATUS>& a_vecStatus)
{
	a_vecStatus.reserve(m_mapProcess.size());
	
	for (auto iter = m_mapProcess.begin(); iter != m_mapProcess.end(); ++iter) {
		a_vecStatus.push_back(iter->second.m_pclsApp->GetStatus());
	}
	
	return;
}

CProcessManager::proc_t::iterator CProcessManager::FindByPid(pid_t a_nPid)
{
	for (auto iter = m_mapProcess.begin(); iter != m_mapProcess.end(); ++iter) {
		if (a_nPid == iter->second.m_pclsApp->GetPid()) {
			return iter;
		}
	}
	
	return m_mapProcess.end();
}
