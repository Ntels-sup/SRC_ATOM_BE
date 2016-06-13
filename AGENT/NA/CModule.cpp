/* vim:ts=4:sw=4
 */
/**
 * \file	CModule.cpp
 * \brief	Node Agent module 관리 class
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <string>

#include "CModule.hpp"

using std::string;

extern CFileLog* g_pclsLog;


CModule::CModule(void)
{
	m_pHandle = NULL;
	m_pstModuleOp = NULL;
    m_nThreadId = 0;
	m_bThreadRunning = false;
	m_bIsInit = false;
	m_bIsCleanUp = false;
	m_tLastProcTime = 0;
	m_nCycleTime = 0;
	
	memset(&m_stOption, 0x00, sizeof(m_stOption));

	return;
}

CModule::~CModule(void)
{
	if (m_pHandle != NULL) {
		UnLoading();
	}
	return;
}

bool CModule::Loading(const string& a_strModule)
{
	// 기존에 loading된 모듈일 경우 unloading 처리
	// 중복으로 loading 요청이 있을 경우 마지막 loading을 유효한 것으로 처리하기 위함.
	if (m_pHandle != NULL) {
		UnLoading();
	}

	m_pHandle = dlopen(a_strModule.c_str(), RTLD_LAZY);
	if (m_pHandle == NULL) {
		g_pclsLog->ERROR("MOD, dlopen failed [%s]", dlerror());
		return false;
	}
	
	dlerror();    // Clear any existing error

	// Module operation check
	m_pstModuleOp = (MODULE_OPERATION_*)dlsym(m_pHandle, "module_op");
	if (m_pstModuleOp == NULL) {
		g_pclsLog->ERROR("MOD, not found module operation");
		goto Failed;
	}
	if (strlen(m_pstModuleOp->szProcName) == 0) {
		g_pclsLog->ERROR("MOD, not set procname");
		goto Failed;
	}
	//m_pstModuleOp->bThreadRun;
	//m_pstModuleOp->Init;
	if (m_pstModuleOp->Process == NULL) {
		g_pclsLog->ERROR("MOD, not set process");
		goto Failed;		
	}
	//m_pstModuleOp->CleanUp;

	return true;


Failed:
	UnLoading();
	return false;
}

void CModule::UnLoading(void)
{
    CleanUp();
    
	if (m_pHandle != NULL) {
		dlclose(m_pHandle);
	}

	m_pHandle = NULL;
	m_pstModuleOp = NULL;

	return;
}

const char* CModule::GetModuleName(void)
{
	if (m_pstModuleOp == NULL) {
		return NULL;
	}
	
	return m_pstModuleOp->szProcName;
}

bool CModule::Initalize(void)
{
	// 중복 초기화 방지 
	if (m_bIsInit) {
		return true;
	}

	if (m_pstModuleOp != NULL && m_pstModuleOp->Init != NULL) {
		if (m_pstModuleOp->Init() == false) {
			UnLoading();
			return false;
		}
	}
	
	return true;
}

bool CModule::Process(void)
{
	if (m_pstModuleOp == NULL) {
        return false;
    }
    if (m_pstModuleOp->Process == NULL) {
        return false;
    }
    
    if (m_pstModuleOp->bThreadRun) {
		if (m_bThreadRunning == false) {
        	if (pthread_create(&m_nThreadId, NULL, m_pstModuleOp->Process, 
															&m_stOption) < 0) {
            	return false;
			}
			m_tLastProcTime = time(NULL);
        }
        m_bThreadRunning = true;
		
    } else {										// thread가 아닌 function
		time_t curtime = time(NULL);
		if (curtime - m_tLastProcTime >= m_nCycleTime) {	// 최소 1초에 한번 실행 
        	m_pstModuleOp->Process(&m_stOption);
			m_tLastProcTime = curtime;
		}
    }

	return true;
}

bool CModule::CleanUp(void)
{
	// 중복 cleanup 방지 
	if (m_bIsCleanUp) {
		return true;
	}

	if (m_pstModuleOp != NULL && m_pstModuleOp->CleanUp != NULL) {
		m_pstModuleOp->CleanUp();
	}

	return true;
}
