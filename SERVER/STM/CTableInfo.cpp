#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "CGlobal.hpp"
#include "CTableInfo.hpp"

CTableInfo::CTableInfo()
{
    m_nCollectTime = 0;
    m_nType = 0; 
    m_nLastSendTime = 0;
    m_blnCollectFlag = true;
    m_lstColumnList = new list<CColumnInfo*>;
}

CTableInfo::~CTableInfo()
{
    delete m_lstColumnList;
}

int CTableInfo::Init(const char *a_chTableName, int a_nCollectTime, int a_nType, bool a_blnCollectFlag)
{
    int nLen = 0;

    nLen = strlen(a_chTableName);
    if(nLen >= TABLE_NAME_LEN){
        STM_LOG(STM_ERR,"Invalid Table length(len=%d, max=%d)\n", nLen, TABLE_NAME_LEN);
        return STM_NOK;
    }

    m_strTableName = a_chTableName;

    m_nCollectTime = a_nCollectTime;

    if((a_nType != STM_TABLE_TYPE_STS) && 
            (a_nType != STM_TABLE_TYPE_HIST)){
        STM_LOG(STM_ERR,"Invalid Table Type(type=%d)\n",a_nType);
        return STM_NOK;
    }

    m_nType = a_nType;
    m_blnCollectFlag = a_blnCollectFlag;

    m_blnInitFlag = true;

    return STM_OK;
}

int CTableInfo::InsertColumn(const char *a_pchColumnName, int a_nSequence, const char *a_pchDataFormat, bool a_blnPKFlag)
{
    int nRet = 0;
    CColumnInfo *pcColumn;

    pcColumn = new CColumnInfo;

    nRet = pcColumn->Init(a_pchColumnName, a_nSequence, a_pchDataFormat, a_blnPKFlag);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Column init failed(name=%s, format=%s, ret=%d)\n",a_pchColumnName, a_pchDataFormat, nRet);
        return STM_NOK;
    }

    m_lstColumnList->push_back(pcColumn);

    return STM_OK;
}

int CTableInfo::CheckTableName(string &a_strTableName)
{
    if(a_strTableName.size() != m_strTableName.size()){
        return STM_NOK;
    }

    if(strncasecmp(a_strTableName.c_str(), m_strTableName.c_str(), m_strTableName.size()) == 0){
        return STM_OK;
    }

    return STM_NOK;
}

CColumnInfo* CTableInfo::SearchColumn(const char *a_chColumnName)
{
    return NULL;
}

string CTableInfo::GetColumnName(int a_nSequence)
{
    string strColumnName;
    list<CColumnInfo*>::iterator iter;
    CColumnInfo *cColumn = NULL;

    if(a_nSequence == 0){
        STM_LOG(STM_ERR,"Invalid sequence(%d)\n",a_nSequence);
        return strColumnName;
    }

    for(iter = m_lstColumnList->begin(); iter != m_lstColumnList->end();iter++){
        cColumn = *iter;

        if(cColumn->GetSequence() == a_nSequence){
            strColumnName = cColumn->GetColumnName();
            return strColumnName;
        }
    }

    strColumnName.clear();
    return strColumnName;
}

string CTableInfo::GetRcdInsertColumnStrList()
{
    unsigned int i = 0;
    string ColumnName;
    string ColumnList;

    ColumnList.append("PRC_DATE, NODE_NAME, DST_FLAG");

    ColumnName.clear();

    for(i=0;i<m_lstColumnList->size();i++){
        ColumnName = GetColumnName(i+1);
        if(ColumnName.size() == 0){
            return ColumnList;
        }
        ColumnList.append(", ");
        ColumnList.append(ColumnName);
        ColumnName.clear();
    }

    return ColumnList;
}

list<string> *CTableInfo::GetPrimaryKeyColumnList()
{
    list<string> *lstPrimaryKey = NULL;
    string strColumnName;
    list<CColumnInfo*>::iterator iter;
    CColumnInfo *cColumn = NULL;

    for(iter = m_lstColumnList->begin(); iter != m_lstColumnList->end();iter++){
        cColumn = *iter;

        if(cColumn->GetPkFlag() == true){
            strColumnName = cColumn->GetColumnName();
            if(lstPrimaryKey == NULL){
                lstPrimaryKey = new list<string>;
            }
            lstPrimaryKey->push_back(strColumnName);
        }
    }

    return lstPrimaryKey;
}

list<string> *CTableInfo::GetValueColumnList()
{
    list<string> *lstValue = NULL;
    string strColumnName;
    list<CColumnInfo*>::iterator iter;
    CColumnInfo *cColumn = NULL;

    for(iter = m_lstColumnList->begin(); iter != m_lstColumnList->end();iter++){
        cColumn = *iter;

        if(cColumn->GetPkFlag() == false){
            strColumnName = cColumn->GetColumnName();
            if(lstValue == NULL){
                lstValue = new list<string>;
            }
            lstValue->push_back(strColumnName);
        }
    }

    return lstValue;
}

