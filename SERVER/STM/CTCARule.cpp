#include <stdio.h>
#include <string.h>
#include "CGlobal.hpp"
#include "CTCARule.hpp"

CTCARule::CTCARule()
{
}

CTCARule::~CTCARule()
{
}

int CTCARule::CheckSign(char *a_chSign)
{
    if(strcmp(a_chSign,"==") == 0){
        return STM_OK;
    }
    else if(strcmp(a_chSign,"<=") == 0){
        return STM_OK;
    }
    else if(strcmp(a_chSign,">=") == 0){
        return STM_OK;
    }
    else if(strcmp(a_chSign,"<") == 0){
        return STM_OK;
    }
    else if(strcmp(a_chSign,">") == 0){
        return STM_OK;
    }
    else if(strcmp(a_chSign,"!!") == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CTCARule::CheckLeftValue(int a_nValue)
{
    /* check left */
    if(strcasecmp(m_chLeftSign,"==") == 0){
        if(m_nLeftVal == a_nValue){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chLeftSign,"<=") == 0){
        if(m_nLeftVal <= a_nValue){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chLeftSign,"<") == 0){
        if(m_nLeftVal < a_nValue){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chLeftSign,">") == 0){
        if(m_nLeftVal > a_nValue){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chLeftSign,"!!") == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CTCARule::CheckRightValue(int a_nValue)
{
    /* check left */
    if(strcasecmp(m_chRightSign,"==") == 0){
        if(a_nValue == m_nRightVal){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chRightSign,"<=") == 0){
        if(a_nValue <= m_nRightVal){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chRightSign,"<") == 0){
        if(a_nValue < m_nRightVal){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chRightSign,">") == 0){
        if(a_nValue > m_nRightVal){
            return STM_OK;
        }
    }
    else if(strcasecmp(m_chLeftSign,"!!") == 0){
        return STM_OK;
    }

    return STM_NOK;
}

int CTCARule::CheckRule(int a_nValue, int *a_nAlarmId)
{
    int nRet = STM_OK;

    nRet = CheckLeftValue(a_nValue);
    if(nRet != STM_OK){
        return STM_NOK;
    }

    nRet = CheckRightValue(a_nValue);
    if(nRet != STM_OK){
        return STM_NOK;
    }

    /* ALARM */
    STM_LOG(STM_ERR,"TCA Occure(%d)\n", a_nValue);

    *a_nAlarmId = m_nSeverity;

    return STM_OK;
}

int CTCARule::Init(int a_nSeverity, char *a_chLeftSign, int a_nLeftVal, char *a_chRightSign, int a_nRightVal)
{
    int nRet = 0;
    m_nSeverity = a_nSeverity;

    nRet = CheckSign(a_chLeftSign);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Invalid sign(%s)\n",a_chLeftSign);
        return STM_NOK;
    }

    memcpy(m_chLeftSign, a_chLeftSign, 2);

    nRet = CheckSign(a_chRightSign);
    if(nRet != STM_OK){
        STM_LOG(STM_ERR,"Invalid sign(%s)\n",a_chRightSign);
        return STM_NOK;
    }

    memcpy(m_chRightSign, a_chRightSign, 2);

    m_nLeftVal = a_nLeftVal;
    m_nRightVal = a_nRightVal;

    return STM_OK;
}
