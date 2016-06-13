#include <unistd.h>
#include "CLA.h"
#include "CProtocol.hpp"
#include "CGlobal.hpp"

CGlobal *CGlobal::m_cInstance = NULL;

CGlobal::CGlobal()
{
	m_cLog = new CFileLog();

	//m_cLog->SetThreadLock();
}

CGlobal::~CGlobal()
{
}

CFileLog *CGlobal::GetLogP()
{
    return m_cLog;
}

CGlobal* CGlobal::GetInstance()
{
    if(m_cInstance == NULL){
        m_cInstance = new CGlobal();
    }

    return m_cInstance;
}

