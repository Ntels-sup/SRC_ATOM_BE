#include <stdio.h>
#include <errno.h>
#include "CGlobal.hpp"
#include "CStsValue.hpp"

CStsValue::CStsValue()
{
    m_lstValue = NULL;
    m_collectStartTime = 0;
    m_collectEndTime = 0;
    m_blnAggregationFlag = false;
}

CStsValue::~CStsValue()
{
    if(m_lstValue != NULL){
        delete m_lstValue;
    }

    if(m_strFileName.size() != 0){
        DEBUG_LOG("REMOVE FILE NAME(%s)\n", m_strFileName.c_str());
        remove(m_strFileName.c_str());
    }
}

int CStsValue::InsertValue(time_t a_collectStartTime, time_t a_collectEndTime, list<int> *a_lstValue, string *a_strFileName)
{
    if(m_lstValue != NULL){
        STA_LOG(STA_ERR,"Value already exist\n");
        return STA_NOK;
    }

    m_collectStartTime = a_collectStartTime;
    m_collectEndTime = a_collectEndTime;

    m_lstValue = a_lstValue;

    if(a_strFileName != NULL){
        if(m_strFileName.size() != 0){
            remove(m_strFileName.c_str());
        }
        m_strFileName = *a_strFileName;
    }

    return STA_OK;
}

int CStsValue::UpdateValue(list<int> *a_lstValue)
{
    list<int>::iterator iter;
    list<int>::iterator updateIter;

    if(m_lstValue == NULL){
        STA_LOG(STA_ERR,"Value not exist\n");
        return STA_NOK;
    }

    if(a_lstValue->size() != m_lstValue->size()){
        STA_LOG(STA_ERR,"Invalid value count(drop original data)(ori=%d,upd=%d)\n",
                (int)m_lstValue->size(), (int)a_lstValue->size());
        delete m_lstValue;

        m_lstValue = a_lstValue;
    }

    for(iter = m_lstValue->begin(), updateIter = a_lstValue->begin();iter != m_lstValue->end();
            iter++, updateIter++){
        (*iter) += *updateIter;
        DEBUG_LOG("Update STIME=%lu, ETIME=%lu, DIFF=%lu, VAL=%d\n",m_collectStartTime, m_collectEndTime,
                m_collectEndTime - m_collectStartTime, *iter);
    }

    return STA_OK;
}

void CStsValue::UpdateCollectTime(time_t a_collectStartTime, time_t a_collectEndTime)
{
    if(a_collectStartTime < m_collectStartTime){
        m_collectStartTime = a_collectStartTime;
    }

    if(a_collectEndTime > m_collectEndTime){
        m_collectEndTime = a_collectEndTime;
    }
}

int CStsValue::CheckKey(time_t a_collectStartTime, time_t a_collectEndTime)
{
    if((m_collectStartTime <= a_collectStartTime) && (m_collectEndTime >= a_collectEndTime)){
        return STA_OK;
    }
    else if((m_collectStartTime >= a_collectStartTime) && (m_collectStartTime < a_collectEndTime)){
        return STA_OK;
    }
    else if((m_collectStartTime < a_collectEndTime) && (m_collectEndTime >= a_collectEndTime)){
        return STA_OK;
    }
    else if((m_collectStartTime > a_collectStartTime) && (m_collectEndTime < a_collectEndTime)){
        return STA_OK;
    }

    return STA_NOK;
}
