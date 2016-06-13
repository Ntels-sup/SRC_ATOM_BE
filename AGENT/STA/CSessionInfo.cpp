#include <stdio.h>
#include <string.h>
#include "STA.h"
#include "CGlobal.hpp"
#include "CSessionInfo.hpp"

CSessionInfo::CSessionInfo()
{
    m_receiveTime = 0;
    m_chReceiveData = NULL;
    m_nReceiveDataLen = 0;
}

CSessionInfo::~CSessionInfo()
{
    if(m_chReceiveData != NULL){
        delete m_chReceiveData;
    }
}

int CSessionInfo::GetReceiveData(char **a_chReceiveData, int *a_nReceiveDataLen)
{
    if(m_chReceiveData == NULL){
        STA_LOG(STA_ERR,"data not exist\n");
        return STA_NOK;
    }

    *a_chReceiveData = m_chReceiveData;
    *a_nReceiveDataLen = m_nReceiveDataLen;

    m_chReceiveData = NULL;
    m_nReceiveDataLen = 0;

    return STA_OK;
}

void CSessionInfo::SetReceiveData(char *a_chReceiveData, int a_nReceiveDataLen)
{
    m_chReceiveData = a_chReceiveData;
    m_nReceiveDataLen = a_nReceiveDataLen;
}

