#include <unistd.h>
#include <stdio.h>
#include "STA.h"
#include "CGlobal.hpp"

CGlobal *CGlobal::cInstance = NULL;

CGlobal::CGlobal()
{
    cLog = new CFileLog();

#if 0
    std::string     strPath(getenv("HOME"));
    strPath += "/LOG";

    cLog->Initialize((char*)strPath.c_str(),"ATOM",(char*)"STA", -1, LV_ERROR);

    cLog->SetThreadLock();
#endif
}

CGlobal::~CGlobal()
{

}

CFileLog *CGlobal::GetLogP()
{
    return cLog;
}

CGlobal* CGlobal::GetInstance()
{
    if(cInstance == NULL){
        cInstance = new CGlobal();
    }

    return cInstance;
}

int CGlobal::ForceDir(const char *a_szDir)
{
	unsigned int nDirLen = 0;
	char    buff[1024];
	char   *p_dirc  = buff;


	nDirLen = strlen(a_szDir);
	if(nDirLen >= sizeof(buff)){
		return STA_NOK;
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
	return STA_OK;
}

