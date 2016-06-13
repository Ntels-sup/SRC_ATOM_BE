#include <unistd.h>
#include <stdio.h>
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

