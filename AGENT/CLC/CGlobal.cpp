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

int CGlobal::GetPathLen(const char *a_szDir, unsigned int *a_nLastCur)
{
	unsigned int i = 0;
	unsigned int nDirLen = 0;
	unsigned int nLastCur = 0;

	nDirLen = strlen(a_szDir);

	for(i=0;i<nDirLen;i++){
		if(a_szDir[i] == '/'){
			nLastCur = i;
		}
	}

	(*a_nLastCur) = nLastCur;

	return CLC_OK;
}

int CGlobal::ForceDir(const char *a_szDir)
{
	unsigned int nDirLen = 0;
	char    buff[1024];
	char   *p_dirc  = buff;


	nDirLen = strlen(a_szDir);
	if(nDirLen >= sizeof(buff)){
		return CLC_NOK;
	}

	memcpy(buff, a_szDir, nDirLen);

	buff[nDirLen]   = '\0';

	while( *p_dirc){
		if ( '/' == *p_dirc){
			*p_dirc = '\0';
			if ( 0 != access( buff, F_OK)){
				mkdir( buff, 0777);
			}
			*p_dirc = '/';
		}
		p_dirc++;
	}
	if ( 0 != access( buff, F_OK)){
		mkdir( buff, 0777);
	}
	return CLC_OK;
}

