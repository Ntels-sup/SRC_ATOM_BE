#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "CGlobal.hpp"
#include "CColumnInfo.hpp"

CColumnInfo::CColumnInfo()
{
    m_nSequence = 0;
    m_blnPKFlag = true;
}

int CColumnInfo::Init(const char *a_chColumnName, int a_nSequence, const char *a_chDataFormat, bool a_blnPKFlag)
{
    int nLen = 0;

    nLen = strlen(a_chColumnName);
    if(nLen >= COLUMN_NAME_LEN){
        STM_LOG(STM_ERR,"Invalid Column name length(len=%d, max=%d)\n",nLen, COLUMN_NAME_LEN);
        return STM_NOK;
    }

    m_nSequence = a_nSequence;

    m_strColumnName = a_chColumnName;

    nLen = strlen(a_chDataFormat);
    if(nLen >= DATA_FORMAT_LEN){
        STM_LOG(STM_ERR,"Invalid Column name length(len=%d, max=%d)\n",nLen, DATA_FORMAT_LEN);
        return STM_NOK;
    }

    m_strDataFormat = a_chDataFormat;

    m_blnPKFlag = a_blnPKFlag;

    return STM_OK;
}
