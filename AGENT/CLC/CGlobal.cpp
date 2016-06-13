#include <unistd.h>
#include "CLC.h"
#include "CProtocol.hpp"
#include "CGlobal.hpp"

CGlobal *CGlobal::m_cInstance = NULL;

CGlobal::CGlobal()
{
	m_cLog = new CFileLog();

	//std::string strPath(getenv("HOME"));
	//strPath += "/LOG";

	//m_cLog->Initialize((char*)strPath.c_str(),"ATOM",(char*)"CLC", -1, LV_ERROR);

	m_cLog->SetThreadLock();

	//m_cTimerHandler = new CTimerHandler();

	//m_cClaInterface = new CClaInterface();
	m_cClaInterface = new CClaInterface();
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
