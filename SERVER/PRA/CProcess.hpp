/* vim:ts=4:sw=4
 */
/**
 * \file	CProcess.hpp
 * \brief	
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CPROCESS_HPP_
#define CPROCESS_HPP_

#include <sys/types.h>
#include <unistd.h>
#include <numa.h>
#include <string>
#include <vector>

#include "CProcStatus.hpp"


class CProcess
{
public:
	// 프로세스 실행 설정 정보
	struct ST_SETUP {
		std::string m_strProcName;
		std::string	m_strExeBin;
		std::string	m_strExeArg;
		std::string	m_strExeEnv;
		std::string	m_strCpuBind;
		std::string	m_strMmrBind;
	} m_stSetup;

	// 프로세스 실행 상태 정보
	struct ST_RUNINFO {
		pid_t		m_nPid;
		pid_t		m_nPpid;
		uid_t		m_nUid;
		time_t		m_tStartTime;
		time_t		m_tStopTime;
		CProcStatus::EN_STATUS	m_enStatus;
		bitmask*	m_pstBindCpu;
		bitmask*	m_pstBindMem;
		std::vector<std::string>	m_vecArg;
		std::vector<std::string>	m_vecEnv;
		std::string	m_strVersion;
		int			m_nExitCode;
	};

	// core dump filter 정의, core manpage 참고
	enum EN_COREFILTER {
		LV0_ANONY_PRIVATE,			// Dump anonymous private mappings.
		LV1_ANONY_SHARED,			// Dump anonymous shared mappings
		LV2_FB_PRIVATE,				// Dump file-backed private mappings.
		LV3_FB_SHARD				// Dump file-backed shared mappings.
	};

public:
	CProcess();
	~CProcess();

	bool		Execute(void);
	bool		ExecuteManual(pid_t a_nPid);
	bool		Terminate(void);
	bool		Kill(void);
	CProcStatus::EN_STATUS ExitCheck(void);

	bool		CoreDumpFilter(EN_COREFILTER a_enFilter);
	bool		SendSignal(int a_nSigno);
	
	void		GetRunInfo(ST_RUNINFO& a_stRun) { a_stRun = m_stRunInfo; }
	CProcStatus::EN_STATUS	GetStatus(void) { return m_stRunInfo.m_enStatus; }
	pid_t		GetPid(void) { return m_stRunInfo.m_nPid; }
	pid_t		GetPpid(void) { return m_stRunInfo.m_nPpid; }
	uid_t		GetUid(void) { return m_stRunInfo.m_nUid; }
	int			GetExitCode(void) { return m_stRunInfo.m_nExitCode; }

	void		SetStatus(CProcStatus::EN_STATUS a_nStatus) {
					m_stRunInfo.m_enStatus = a_nStatus;
				}

	bool		IsRunning(void);

	friend class CProcessManager;

private:
	ST_RUNINFO	m_stRunInfo;
	int			m_nTermSignal;

private:
	void		SetupAttributes(void);
	char**		ArgumentToArry(void);
	char**		EnvironmentToArry(void);
	bitmask*	BindStringParse(std::string& strBind, char a_cType);
	void		CloseFd(void);
	void		ClearRunInfo(bool a_bIsExit = false);
};

#endif
