#ifndef __CCOLUMNINFO_HPP__
#define __CCOLUMNINFO_HPP__

#include <string>
#include "STM.h"

using namespace std;

class CColumnInfo{
    private:
        string m_strColumnName;
        string m_strDataFormat;
        bool m_blnPKFlag;
        int m_nSequence;
    public:
        CColumnInfo();
        ~CColumnInfo() {};

        int Init(const char *a_chColumnName, int a_nSequence, const char *a_chDataForamt, bool a_blnPKFlag);
        string GetColumnName() { return m_strColumnName; };
        bool GetPkFlag() { return m_blnPKFlag; };
        int GetSequence() { return m_nSequence; };
};

#endif
