#ifndef __CTCARULE_HPP__
#define __CTCARULE_HPP__

#include <list>

#include "STM.h"

using namespace std;

class CTCARule{
    private :
        int m_nSeverity;
        char m_chLeftSign[3];
        int m_nLeftVal;
        char m_chRightSign[3];
        int m_nRightVal;

        int CheckSign(char *a_chSign);
    public :
        CTCARule();
        ~CTCARule();
        int CheckRule(int a_nValue, int *a_nAlarmId);
        int Init(int a_nSeverity, char *a_chLeftSign, int a_nLeftVal,
                char *a_chRightSign, int a_nRightVal);
        int CheckLeftValue(int a_nValue);
        int CheckRightValue(int a_nValue);
};

#endif
