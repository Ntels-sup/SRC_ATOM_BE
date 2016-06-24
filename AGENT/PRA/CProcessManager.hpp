/* vim:ts=4:sw=4
 */
/**
 * \file	CProcessManager.hpp
 * \brief	APP 프로세스 관리
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CPROCESSMANAGER_HPP_
#define CPROCESSMANAGER_HPP_

#include <sys/types.h>
#include <sys/wait.h>
#include <list>
#include <map>
#include <vector>

#include "CProtocol.hpp"
#include "CLQManager.hpp"
#include "EventAPI.hpp"

#include "CProcess.hpp"


class CProcessManager
{
public:
	struct ST_APPINFO {
        int         m_nProcNo;
        std::string m_strPkgName;
        std::string m_strProcName;
        std::string m_strNodeType;
		int			m_nSvcNo;
        std::string m_strSvcName;
        bool        m_bIsExec;

        std::string m_strExeBin;
        std::string m_strExeArg;
        std::string m_strExeEnv;

        int         m_nStartOrder;
        int         m_nStopOrder;

        std::string m_strCpuBind;
        std::string m_strMmrBind;

        int         m_nNormalStopCd;
        int         m_nRestartCnt;
        char        m_bIsBatch;

        bool        m_bIsUse;
	};

	struct ST_APPRUNINFO {
		ST_APPINFO				m_stInfo;
		CProcess::ST_RUNINFO	m_stRun;
	};

	int			m_nRestartClearTime;		// 재실행 횟수 초기화 시간
	int			m_nHangupTime;				// Hangup 기준 시간
	int			m_nAlivePeriod;				// Alive check 주기


public:
	CProcessManager();
	~CProcessManager();
	
	bool		Add(ST_APPINFO& a_stProc);
	void		Run(int a_nProcNo = -1);
	void		Stop(int a_nProcNo = -1);
	void		StopForceibly(int a_nProcNo = -1);
	void		Disable(int a_nProcNo = -1);
	bool		IsRunning(int a_nProcNo);
	void		AliveOk(int a_nProcNo);

	void		CheckAlive(void);
	void		CheckExit(int a_nProcNo = -1);
	void		CheckManualRun(void);

	const char*	GetProcName(int a_nProcNo, std::string* a_pstrProcName);

	int			GetAppInfo(std::vector<CProcessManager::ST_APPINFO>& a_vecInfo,
						int a_nProcNo = -1);
	int			GetAppInfo(std::vector<CProcessManager::ST_APPINFO>& a_vecInfo,
						CProcStatus::EN_STATUS a_nStatus = CProcStatus::NONE);
	int			GetAppInfoStopOrder(std::vector<CProcessManager::ST_APPINFO>& a_vecInfo,
						CProcStatus::EN_STATUS a_nStatus = CProcStatus::NONE);

	int			GetRunInfo(std::vector<CProcess::ST_RUNINFO>& a_vecInfo,
															int a_nProcNo = -1);
	int			GetRunInfo(std::vector<CProcess::ST_RUNINFO>& a_vecInfo, 
						CProcStatus::EN_STATUS a_nStatus = CProcStatus::NONE);

	CProcStatus::EN_STATUS	GetStatus(int a_nProcNo);
	int						GetStatus(std::list<ST_APPRUNINFO>& a_lstStat,
													int a_nProcNo = -1);
	int						GetStatus(std::list<ST_APPRUNINFO>& a_lstStat,
								CProcStatus::EN_STATUS a_nStatus = CProcStatus::NONE);
	CProcStatus::EN_STATUS	GetStatusWorst(void);

	int			GetExitCode(int a_nProcNo);
	int			FindProcNo(const std::string& a_strProcName);


private:
	struct ST_PROCESS {
		ST_APPINFO	m_stAppInfo;
		CProcess*	m_pclsApp;
		int			m_nRunCount;			// 프로레스 실행 횟수
		time_t		m_nAliveTime;			// 마지막 Alive 확인 시간
	};
	typedef std::map<int, ST_PROCESS> proc_t;
	proc_t	m_mapProcess;					// key: procno
	
	CProcStatus::EN_STATUS	m_enAppLastStatus;		// 프로세스 마지막 상태
	time_t					m_tAppLastStatusTime;	// 프로세스 미자막 상태 확인 시간

	EventAPI	m_clsAlm;
	
	
private:
	void		SetAtomEnvArg(CProcessManager::ST_APPINFO& a_stSetup);
	
	static bool	StartCmp(const proc_t::iterator L, const proc_t::iterator R);
	static bool	StopCmp(const proc_t::iterator L, const proc_t::iterator R);
	static bool	StatusCmp(const proc_t::iterator L, const proc_t::iterator R);

	void		OrderStart(std::vector<proc_t::iterator>& a_vecStart);
	void		OrderStop(std::vector<proc_t::iterator>& a_vecStop);
	void		OrderStatus(std::vector<proc_t::iterator>& a_vecStatus);
	
	void		ExtracStatus(std::vector<CProcStatus::EN_STATUS>& a_vecStatus);
	proc_t::iterator	FindByPid(pid_t a_nPid);
};

#endif
