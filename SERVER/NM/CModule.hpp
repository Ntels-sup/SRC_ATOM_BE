/* vim:ts=4:sw=4
 */
/**
 * \file	CModule.hpp
 * \brief	Node Agent module 관리 class
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */

#ifndef CMODULE_HPP_
#define CMODULE_HPP_

#include <string>

#include "DB.hpp"
#include "CommonCode.hpp"

#include "CModuleIPC.hpp"
#include "CAddress.hpp"


struct MODULE_OPERATION_
{
	char	szProcName[DB_PROC_NAME_SIZE];
	bool	bThreadRun;					// Thread 로 동작 여부
	bool	(*Init)	(void);				// 초기화 function pointer
	void*	(*Process) (void* a_pArg);	// data 처리 function pointer,
									  	// thread 경우 start function 
	void	(*CleanUp) (void);			// 종료 function pointer
};
#define MODULE_OP MODULE_OPERATION_ module_op

class CModule
{
public:
	struct ST_MODULE_OPTIONS {
		char		m_szPkgName[DB_PKG_NAME_SIZE];
		char		m_szNodeType[DB_NODE_TYPE_SIZE];
		char		m_szProcName[DB_PROC_NAME_SIZE];
		char		m_szNodeName[DB_NODE_NAME_SIZE];
		char		m_szNodeVersion[DB_PKG_VER_SIZE];
		char		m_szCfgFile[160];
		int			m_nNodeNo;
		int			m_nProcNo;
		CModuleIPC*	m_pclsModIpc;
		CAddress*	m_pclsAddress;
		DB*			m_pclsDB;
	} m_stOption;
	
public:
	CModule();
	~CModule();
	
	bool	Loading(const std::string& a_strModule);
	void	UnLoading(void);
	const char* GetModuleName(void);

	bool	Initalize(void);
	bool	Process(void);
	bool	CleanUp(void);
    
    bool    IsThread(void) { 
				bool ret = (m_pstModuleOp) ? m_pstModuleOp->bThreadRun : false;
				return ret;
			}
    bool    IsThreadRunning(void) { return m_bThreadRunning; }

private:
	std::string			strModuleFName;			// module 파일
	void*				m_pHandle;				// module handle
	MODULE_OPERATION_*	m_pstModuleOp;			// module operation pointer
	
	bool				m_bIsInit;				// 초기화 실행 여부
	bool				m_bIsCleanUp;			// clean 실행 여부

	pthread_t			m_nThreadId;			// 스레드id
	bool				m_bThreadRunning;		// 현재 스레드로 동작 중인가?

	time_t				m_tLastProcTime;		// 마지막 실행 시간, 스레드는 실행 시간
	int					m_nCycleTime;			// 실행 주기(초), 향후 기능 보완 예정
};

#endif // CMODULE_HPP_
