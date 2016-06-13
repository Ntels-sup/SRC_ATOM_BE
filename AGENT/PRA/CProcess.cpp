/* vim:ts=4:sw=4
 */
/**
 * \file	CProcess.cpp
 * \brief	
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#include <sys/types.h> 
#include <sys/wait.h>

#include "CFileLog.hpp"
#include "Utility.hpp"

#include "CProcess.hpp"

using std::string;
using std::vector;

extern CFileLog* g_pclsLogPra;


CProcess::CProcess()
{
	ClearRunInfo();

	m_nTermSignal = SIGTERM;		// process termiate signal

	return;
}

CProcess::~CProcess()
{
	ClearRunInfo();
	return;
}

bool CProcess::Execute(void)
{
	if (m_stRunInfo.m_nPid > 0) {
		g_pclsLogPra->WARNING("already running");
		return true;
	}

	// exec binary check
	if (access(m_stSetup.m_strExeBin.c_str(), R_OK | X_OK) < 0) {
		g_pclsLogPra->ERROR("not found binary or execute permissions failed");
		m_stRunInfo.m_enStatus = CProcStatus::NOEXEC;
		return false;
	}

	ClearRunInfo();

	// 프로세스 실행 전 argument, envionment, bind 정보 생성
	SetupAttributes();

	pid_t pid = fork();
	if (pid < 0) {										// fork failed
		g_pclsLogPra->CRITICAL("fork failed, process: %s", m_stSetup.m_strExeBin.c_str());
		m_stRunInfo.m_enStatus = CProcStatus::NOEXEC;
		return false;
	}

	if (pid > 0) {										// parent process
		m_stRunInfo.m_nPid = pid;
		m_stRunInfo.m_nPpid = getppid();
		m_stRunInfo.m_nUid = getuid();
		m_stRunInfo.m_tStartTime = time(NULL);
		m_stRunInfo.m_enStatus = CProcStatus::RUNNING;

	} else if (pid == 0) {								// child process 
		// physical CPU binding
		if (m_stRunInfo.m_pstBindCpu != NULL) {
			if (numa_sched_setaffinity(0, m_stRunInfo.m_pstBindCpu) != 0) {
				g_pclsLogPra->WARNING("cpu bind error");
			}
		}

		// MEMORY numa node binding
		if (m_stRunInfo.m_pstBindMem != NULL) {
			numa_set_bind_policy(1);
			numa_set_membind(m_stRunInfo.m_pstBindMem);
			numa_set_bind_policy(0);
		}
		
		// 부모의 열려있는 file descriptor를 close 한다.
		CloseFd();

		char** exargv = ArgumentToArry();
		char** exenv = EnvironmentToArry();

		/*
		if (execve(m_stSetup.m_strExeBin.c_str(), exargv, exenv) < 0) {
			g_pclsLogPra->CRITICAL("execve failed, pname: %s, errno: %d(%s)", 
										m_stSetup.m_strExeBin.c_str(), errno,
										strerror(errno));
		}
		*/

		for (int i=0; exenv[i] != NULL; i++) {
			putenv(exenv[i]);
		}
		if (execvp(m_stSetup.m_strExeBin.c_str(), exargv) < 0) {
			g_pclsLogPra->CRITICAL("execve failed, pname: %s, errno: %d(%s)", 
										m_stSetup.m_strExeBin.c_str(), errno,
										strerror(errno));
		}

		exit(99);
	}

	return true;
}

bool CProcess::ExecuteManual(pid_t a_nPid)
{
	// TODO
	return true;
}

bool CProcess::Terminate(void)
{
	if (m_stRunInfo.m_nPid <= 0) {
		g_pclsLogPra->WARNING("not running process");
		return false;
	}
	
	kill(m_stRunInfo.m_nPid, m_nTermSignal);

	return true;
}

bool CProcess::Kill(void)
{
	if (m_stRunInfo.m_nPid <= 0) {
		g_pclsLogPra->WARNING("not running process");
		return true;
	}
	
	kill(m_stRunInfo.m_nPid, SIGSTOP);
	
	return true;
}

CProcStatus::EN_STATUS CProcess::ExitCheck(void)
{
	if (m_stRunInfo.m_nPid <= 0) {
		// 프로세스가 실행중이지 않다.
		return m_stRunInfo.m_enStatus;
	}
	
	int nStat;
	pid_t nPid = waitpid(m_stRunInfo.m_nPid, &nStat, WNOHANG | WUNTRACED);
	if (nPid < 0) {
		g_pclsLogPra->WARNING("watepid failed, errno: %d", errno);
		return CProcStatus::NONE;
	} else if (nPid == 0) {
		// 프로세스의 상태 변화가 없다.
		return m_stRunInfo.m_enStatus;
	}

	ClearRunInfo(true);
	
	if (WIFEXITED(nStat)) {
		m_stRunInfo.m_enStatus = CProcStatus::STOPPED;
		m_stRunInfo.m_nExitCode = WEXITSTATUS(nStat);

		g_pclsLogPra->INFO("process exited, pid: %d, pname: %s, exitcd: %d",
					nPid, m_stSetup.m_strProcName.c_str(), m_stRunInfo.m_nExitCode);

	} else if (WIFSIGNALED(nStat)) {
		int signo = WTERMSIG(nStat);
		if (signo == m_nTermSignal) {
			m_stRunInfo.m_enStatus = CProcStatus::STOPPED;
			m_stRunInfo.m_nExitCode = WEXITSTATUS(nStat);

			g_pclsLogPra->INFO("process exited by stop signal(%d), pid: %d, pname: %s",
								signo, nPid, m_stSetup.m_strProcName.c_str());
		} else {
			if (m_stRunInfo.m_enStatus != CProcStatus::HANGUP) {
				m_stRunInfo.m_enStatus = CProcStatus::ABNORMAL;
			}
			g_pclsLogPra->WARNING("process exited by signal(%d), pid: %d, pname: %s",
								signo, nPid, m_stSetup.m_strProcName.c_str());

			if (WCOREDUMP(nStat)) {
				g_pclsLogPra->WARNING("coredump created");
			}
		}
		
	} else if (WIFSTOPPED(nStat)) {
		m_stRunInfo.m_enStatus = CProcStatus::SUSPEND;

		g_pclsLogPra->WARNING("process suspend by signal(%d)", WSTOPSIG(nStat));
	}
	
	return m_stRunInfo.m_enStatus;
}

bool CProcess::CoreDumpFilter(EN_COREFILTER a_enFilter)
{
	if (m_stRunInfo.m_nPid <= 0) {
		return false;
	}

	int fd = -1;
	char szCoreFilter[100];
	sprintf(szCoreFilter,"/proc/%d/coredump_filter", m_stRunInfo.m_nPid);

	if ((fd = open(szCoreFilter, O_WRONLY)) >= 0) {
		switch (a_enFilter) {
			case LV0_ANONY_PRIVATE :
				if (write(fd, "1", 1) != 1) {
					g_pclsLogPra->ERROR("core filter write failure, %s", szCoreFilter);
					close(fd);
					return false;
				}
				break;
			case LV1_ANONY_SHARED :
				if (write(fd, "2", 1) != 1) {
					g_pclsLogPra->ERROR("core filter write failure, %s", szCoreFilter);
					close(fd);
					return false;
				}
				break;
			case LV2_FB_PRIVATE :
				if (write(fd, "4", 1) != 1) {
					g_pclsLogPra->ERROR("core filter write failure, %s", szCoreFilter);
					close(fd);
					return false;
				}
				break;
			case LV3_FB_SHARD :
				if (write(fd, "8", 1) != 1) {
					g_pclsLogPra->ERROR("core filter write failure, %s", szCoreFilter);
					close(fd);
					return false;
				}
				break;
			default :
				g_pclsLogPra->ERROR("unknown core filter, code: %d", a_enFilter);
				close(fd);
				return false;
		}
		close(fd);

	} else {
		g_pclsLogPra->ERROR("core filter failed, errono:%d, errmsg:%s",
													errno, strerror(errno));
		return false;
	}

	return true;
}

bool CProcess::SendSignal(int a_nSigno)
{
	if (m_stRunInfo.m_nPid <= 0) {
		return false;
	}
	
	if (kill(m_stRunInfo.m_nPid, a_nSigno) < 0) {
		return false;
	}
	
	return true;
}

bool CProcess::IsRunning(void)
{
	char filename[180];
	snprintf(filename, sizeof(filename), "/proc/%d", m_stRunInfo.m_nPid);
	
	if (access(filename, R_OK) < 0) {
		return false;
	}

	return true;
}

void CProcess::SetupAttributes(void)
{
	// arguemnt을 space 기준으로 분리하여 vector 삽입
	m_stRunInfo.m_vecArg.clear();
	StringSplit(m_stSetup.m_strExeArg, " ", m_stRunInfo.m_vecArg);

	// envrionment를 space 기준으로 분리하여 vector 삽입
	m_stRunInfo.m_vecEnv.clear();
	StringSplit(m_stSetup.m_strExeEnv, " ", m_stRunInfo.m_vecEnv);

	//TODO
	// core 와 socket을 bitmask를 mearge 해야 한다.
	#if 0
	bitmask* pBitMask = numa_parse_cpustring(char *string);
	bitmask* pBitMask = numa_parse_nodestring(char *string);
	#endif

	// TODO
	// memory bind
	
	return;
}

char** CProcess::ArgumentToArry(void)
{
	// +2은 process name, NULL pointer 추가 위함.
	char** arry = new char*[m_stRunInfo.m_vecArg.size() + 2];

	arry[0] = (char*)m_stSetup.m_strExeBin.c_str();
	for (size_t i=0; i < m_stRunInfo.m_vecArg.size(); i++) {
		arry[i+1] = (char*)m_stRunInfo.m_vecArg[i].c_str();
	}
	arry[m_stRunInfo.m_vecArg.size() + 1] = NULL;
	
	return arry;
}

char** CProcess::EnvironmentToArry(void)
{
	// +1은 NULL pointer 추가 위함.
	char** arry = new char*[m_stRunInfo.m_vecEnv.size() + 1];		

	for (size_t i=0; i < m_stRunInfo.m_vecEnv.size(); i++) {
		arry[i] = (char*)m_stRunInfo.m_vecEnv[i].c_str();
	}
	arry[m_stRunInfo.m_vecEnv.size()] = NULL;
	
	return arry;
}

bitmask* CProcess::BindStringParse(string& strBind, char a_cType)
{
	char* szNodeCpu = strdup(strBind.c_str());

	// 현재 시슷템에서 사용할 수 있는 CPU core 개수, Memory Slot 개수 얻는다.
	int nCnt = 0;
	if (a_cType == 'N') {							// node
		nCnt = numa_num_configured_nodes();
	} else if (a_cType == 'C') {   				//cpu physical id
		nCnt = numa_num_configured_cpus();
	} else {
		return NULL;
	}

	bitmask* pBitMask = numa_bitmask_alloc(nCnt);
	numa_bitmask_clearall(pBitMask);

	char* next = NULL;
	char* token = strtok_r(szNodeCpu, ",", &next);

	while (token != NULL) {
		if (nCnt > atoi(token)) {
			numa_bitmask_setbit(pBitMask, atoi(token));
		}
		token = next;
		token = strtok_r(token, ",", &next);
	}
	free(szNodeCpu);

	return pBitMask;
}

void CProcess::CloseFd(void)
{
	for (int nfd = 3; nfd < 1024; nfd++) {
		close(nfd);
	}
	
	return;
}

void CProcess::ClearRunInfo(bool a_bIsExit)
{
	m_stRunInfo.m_nPid = -1;
	m_stRunInfo.m_nPpid = -1;
	m_stRunInfo.m_nUid = -1;
	if (a_bIsExit == false) {
		m_stRunInfo.m_tStartTime = time(NULL);
	}
	m_stRunInfo.m_tStopTime = time(NULL);
	m_stRunInfo.m_enStatus = CProcStatus::STOPPED;
	if (a_bIsExit && m_stRunInfo.m_pstBindCpu != NULL) {
		numa_bitmask_free(m_stRunInfo.m_pstBindCpu);
	}
	m_stRunInfo.m_pstBindCpu = NULL;
	if (a_bIsExit && m_stRunInfo.m_pstBindMem != NULL) {
		numa_bitmask_free(m_stRunInfo.m_pstBindMem);
	}
	m_stRunInfo.m_pstBindMem = NULL;
	m_stRunInfo.m_vecArg.clear();
	m_stRunInfo.m_vecEnv.clear();
	m_stRunInfo.m_nExitCode = 0;

	return;
}
