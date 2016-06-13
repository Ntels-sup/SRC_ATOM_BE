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

#include "CommonCode.hpp"
#include "CModuleIPC.hpp"


struct MODULE_OPERATION_
{
	char	szProcName[DB_PROC_NAME_SIZE];
	bool	bThreadRun;				// Thread 로 동작 여부
	bool	(*Init)	(void);			// 초기화 function pointer
	void*	(*Process) (void* a_pArg);// data 처리 function pointer,
									  // thread 경우 start function 
	void	(*CleanUp) (void);		// 종료 function pointer
};
#define MODULE_OP MODULE_OPERATION_ module_op

class CModule
{
public:
	struct ST_MODULE_OPTIONS {
		char		m_szPkgName[80];
		char		m_szNodeType[80];
		char		m_szProcName[80];
		int			m_nNodeNo;
		int			m_nProcNo;
		CModuleIPC*	m_pclsModIpc;
	};
	
public:
	CModule();
	~CModule();
	
	bool	Loading(std::string& a_strModule);
	void	UnLoading(void);
	void	SetOption(ST_MODULE_OPTIONS& a_stOpt);
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
	
	ST_MODULE_OPTIONS	m_stOption;				// module option

	bool				m_bIsInit;				// 초기화 실행 여부
	bool				m_bIsCleanUp;			// clean 실행 여부

	pthread_t			m_nThreadId;
	bool				m_bThreadRunning;

	time_t				m_tLastProcTime;		// 마지막 실행 시간, 스레드는 실행 시간
};

#endif // CMODULE_HPP_
