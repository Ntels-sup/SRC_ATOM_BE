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

int CGlobal::ForceDir(const char *a_szDir)
{
	unsigned int nDirLen = 0;
	char    buff[1024];
	char   *p_dirc  = buff;


	nDirLen = strlen(a_szDir);
	if(nDirLen >= sizeof(buff)){
		return CLA_NOK;
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
	return CLA_OK;
}
