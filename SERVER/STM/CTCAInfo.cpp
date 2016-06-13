#include <stdio.h>
#include <string.h>
#include "CTCAInfo.hpp"

CTCAInfo::CTCAInfo()
{
    m_blnActiveFlag = true;
    m_lstRuleList = new list<CTCARule*>;
}

CTCAInfo::~CTCAInfo()
{
    list<CTCARule*>::iterator iter;
    //CTCARule *cFindRule = NULL;

#if 0
    for(iter = m_lstRuleList->begin(); iter != m_lstRuleList->end(); iter++){
        cFindRule = *iter;

        m_lstRuleList->erase(iter);

        delete cFindRule;
    }
#else 
    while(m_lstRuleList->size()){
        delete m_lstRuleList->front();
        m_lstRuleList->pop_front();
    }
#endif

    delete m_lstRuleList;
}

int CTCAInfo::Init( string &a_strTableName, string &a_strColumnName, string &a_strTCAName,
				string &a_strPackageName, bool m_blnActiveFlag)
{
    m_strTableName = a_strTableName;

    m_strColumnName = a_strColumnName;

    m_strTCAName = a_strTCAName;

    m_strPackageName = a_strPackageName;

    m_blnActiveFlag = m_blnActiveFlag;

    return STM_OK;
}

int CTCAInfo::InsertRule(CTCARule *a_cTCARule)
{
    m_lstRuleList->push_back(a_cTCARule);

    return STM_OK;
}

int CTCAInfo::CheckRule(int a_nValue,  list<int> *a_lstAlarmList)
{
    int nRet = STM_NOK;
    int nAlarmId = 0;
    CTCARule *cFindRule = NULL;
    list<CTCARule*>::iterator iter;

    for(iter = m_lstRuleList->begin(); iter != m_lstRuleList->end();iter++){
        cFindRule = *iter;

        nRet = cFindRule->CheckRule(a_nValue, &nAlarmId);
        if(nRet == STM_OK){
            /* send to alarm */
            a_lstAlarmList->push_back(nAlarmId);
        }
    } 

    return nRet;
}

int CTCAInfo::CheckPackageName(string &a_strPackageName)
{
    if(strcasecmp(m_strPackageName.c_str(), a_strPackageName.c_str()) == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CTCAInfo::CheckTableName(string &a_strTableName)
{
    if(strcasecmp(m_strTableName.c_str(), a_strTableName.c_str()) == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CTCAInfo::CheckColumnName(string &a_strColumnName)
{
    if(strcasecmp(m_strColumnName.c_str(), a_strColumnName.c_str()) == 0){
        return STM_OK;
    }

    return STM_NOK;
}
