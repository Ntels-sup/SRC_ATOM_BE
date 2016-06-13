#include "CMergeTableSession.hpp"

CMergeTableSession::CMergeTableSession()
{
    m_hourStartTime = 0;
    m_hourEndTime = 0;
    m_dailyStartTime = 0;
    m_dailyEndTime = 0;
    m_lstPrimaryKeyColumn = NULL;
    m_lstColumn = NULL;
}

CMergeTableSession::~CMergeTableSession()
{
    if(m_lstPrimaryKeyColumn != NULL){
        delete m_lstPrimaryKeyColumn;
    }

    if(m_lstColumn != NULL){
        delete m_lstColumn;
    }
}

void CMergeTableSession::SetTime(time_t a_hourStartTime, time_t a_hourEndTime, time_t a_dailyStartTime, 
				time_t a_dailyEndTime, time_t a_dropTime)
{
    m_hourStartTime = a_hourStartTime;
    m_hourEndTime = a_hourEndTime;
    m_dailyStartTime = a_dailyStartTime;
    m_dailyEndTime = a_dailyEndTime;
    m_dropTime = a_dropTime;
}

void CMergeTableSession::GetTime(time_t &a_hourStartTime, time_t &a_hourEndTime, time_t &a_dailyStartTime, 
				time_t &a_dailyEndTime, time_t &a_dropTime)
{
    a_hourStartTime = m_hourStartTime;
    a_hourEndTime = m_hourEndTime;
    a_dailyStartTime = m_dailyStartTime;
    a_dailyEndTime = m_dailyEndTime;
    a_dropTime = m_dropTime;
}

void CMergeTableSession::SetTableName(string &a_strTableName)
{
    m_strTableName = a_strTableName;
}

void CMergeTableSession::SetPrimaryKeyColumn(list<string> *a_lstPrimaryKeyColumn)
{
    m_lstPrimaryKeyColumn = a_lstPrimaryKeyColumn;
}

void CMergeTableSession::SetColumn(list<string> *a_lstColumn)
{
    m_lstColumn = a_lstColumn;
}

