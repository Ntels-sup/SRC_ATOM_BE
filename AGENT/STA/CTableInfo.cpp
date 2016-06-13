#include <stdio.h>
#include <stdlib.h>
#include "STA.h"
#include "CGlobal.hpp"
#include "CStsValueList.hpp"
#include "CTableInfo.hpp"
#include "CCollectValue.hpp"

CTableInfo::CTableInfo(string a_strTableName)
{
    m_strTableName.append(a_strTableName);
    m_lstStsValueList = new list<CStsValueList*>;
}

CTableInfo::~CTableInfo()
{
    int nRet = 0;
    nRet = DropAll();
    if(nRet != STA_OK){
        STA_LOG(STA_ERR,"Table list drop failed(ret=%d)\n",nRet);
    }

    delete m_lstStsValueList;
}

void CTableInfo::SetTableName(string a_strTableName)
{
    m_strTableName.clear();
    m_strTableName.append(a_strTableName);
}

int CTableInfo::Aggregation(time_t a_endTime, string a_strDir)
{
    int nRet = 0;
    int nIndex = 0;
    list<CStsValueList*>::iterator iter;
    CStsValueList *cValueList = NULL;	

    DEBUG_LOG("AGGREGATION START\n");
    for(iter = m_lstStsValueList->begin() ; iter != m_lstStsValueList->end(); iter++){
        cValueList = *iter;

        nRet = cValueList->Aggregation(a_endTime, a_strDir, m_strTableName, nIndex++);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Aggregation failed(time=%lu, tableName=%s)\n",a_endTime, m_strTableName.c_str());
            continue;
        }
    }

    return STA_OK;
}

int CTableInfo::GetValueCount()
{
    if(m_lstStsValueList == NULL){
        return 0;
    }
    else {
        return m_lstStsValueList->size();
    }
}

int CTableInfo::InsertValue(time_t a_collectStartTime, time_t a_collectEndTime, 
				list<string> *a_lstPrimaryKey, list<int> *a_lstValue, string *a_strFileName)
{
    int nRet = STA_NOK;
    list<CStsValueList*>::iterator iter;
    CStsValueList *cStsValueList = NULL;

    nRet = STA_NOK;

    for(iter = m_lstStsValueList->begin(); iter != m_lstStsValueList->end();iter++){
        cStsValueList = *iter;

        nRet = cStsValueList->CheckKey(a_lstPrimaryKey);
        if(nRet == STA_OK){
            nRet = cStsValueList->InsertValue(a_collectStartTime, a_collectEndTime, a_lstValue, a_strFileName);
            if(nRet != STA_OK){
                STA_LOG(STA_ERR,"sts value insert failed(ret=%d)\n",nRet);
                return nRet;
            }

            delete a_lstPrimaryKey;

            return STA_OK;
        }
    }

    cStsValueList = new CStsValueList();
    cStsValueList->SetPrimaryKey(a_lstPrimaryKey);
    cStsValueList->InsertValue(a_collectStartTime, a_collectEndTime, a_lstValue, a_strFileName);

    m_lstStsValueList->push_back(cStsValueList);

    return STA_OK;
}

list<CCollectValue*> *CTableInfo::GetCollectValue(time_t a_nStartTime, int a_nCollectTime)
{
    list<CCollectValue*>* lstCollectValue = NULL;
    CCollectValue *cCollectValue = NULL;
    list<CStsValueList*>::iterator iter;
    CStsValueList *cValueList = NULL;	
    CStsValue *cValue = NULL;

#if 0
    iter = m_lstStsValueList->begin();
    while(iter != m_lstStsValueList->end()){
        cValueList = *iter;

        cValue = cValueList->GetCollectValue(a_nStartTime, a_nCollectTime);
        if(cValue == NULL){
			iter++;
            continue;
        }
        cCollectValue = new CCollectValue();
        cCollectValue->SetPrimaryKey(cValueList->GetPrimaryKey());
        cCollectValue->SetValue(cValue->GetValue());

        delete cValue;

        if(lstCollectValue == NULL){
            lstCollectValue = new list<CCollectValue*>;
        }
        lstCollectValue->push_back(cCollectValue);

        if(cValueList->GetValueCount() == 0){
            m_lstStsValueList->erase(iter);
            delete cValueList;
        }
        else {
            iter++;
        }
    }
#endif

#if 1
    for(iter = m_lstStsValueList->begin() ; iter != m_lstStsValueList->end(); iter++){
        cValueList = *iter;

        cValue = cValueList->GetCollectValue(a_nStartTime, a_nCollectTime);
        if(cValue == NULL){
            continue;
        }
        cCollectValue = new CCollectValue();
        cCollectValue->SetPrimaryKey(cValueList->GetPrimaryKey());
        cCollectValue->SetValue(cValue->GetValue());

        delete cValue;

        if(lstCollectValue == NULL){
            lstCollectValue = new list<CCollectValue*>;
        }
        lstCollectValue->push_back(cCollectValue);

        if(cValueList->GetValueCount() == 0){
            m_lstStsValueList->erase(iter--);
            delete cValueList;
        }
    }
#endif

    return lstCollectValue;
}

int CTableInfo::DropStsValue(time_t a_dropTime)
{
    int nRet = 0;
    list<CStsValueList*>::iterator iter;
    CStsValueList *cValueList = NULL;	

#if 0
    iter = m_lstStsValueList->begin();
    while(iter != m_lstStsValueList->end()){
        cValueList = *iter;

        nRet = cValueList->DropStsValue(a_dropTime);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Sts value drop failed(ret=%d)\n",nRet);
            continue;
        }

        if(cValueList->GetValueCount() == 0){
            m_lstStsValueList->erase(iter);
            delete cValueList;
        }
        else {
            iter++;
        }
    }
#endif

#if 1
    for(iter = m_lstStsValueList->begin() ; iter != m_lstStsValueList->end(); iter++){
        cValueList = *iter;

        nRet = cValueList->DropStsValue(a_dropTime);
        if(nRet != STA_OK){
            STA_LOG(STA_ERR,"Sts value drop failed(ret=%d)\n",nRet);
            continue;
        }

        if(cValueList->GetValueCount() == 0){
            m_lstStsValueList->erase(iter--);
            delete cValueList;
        }
    }
#endif

    return STA_OK;
}

int CTableInfo::DropAll()
{
    //CStsValueList *cValueList = NULL;	

    while(m_lstStsValueList->size()){
        delete m_lstStsValueList->front();
        m_lstStsValueList->pop_front();
    }

    return STA_OK;
}
