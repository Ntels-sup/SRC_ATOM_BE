#ifndef __CTCAINFO_HPP__
#define __CTCAINFO_HPP__
#include <string>
#include <list>
#include "STM.h"
#include "CTCARule.hpp"

using namespace std;

class CTCAInfo{
    private :
        string m_strPackageName;
        string m_strTCAName;
        string m_strColumnName;
        string m_strTableName;
        bool m_blnActiveFlag; 
        list<CTCARule*> *m_lstRuleList;

    public : 
        CTCAInfo();
        ~CTCAInfo();
        int Init(string &a_strTableName, string &a_strColumnName,  string &a_strTCAName, 
                string &a_strPackageName, bool m_blnActiveFlag);
        string GetPackageName() { return m_strPackageName; };
        string GetTableName() { return m_strTableName; };
        string GetColumnName() { return m_strColumnName; };

        int CheckPackageName(string &a_strPackageName);
        int CheckTableName(string &a_strTableName);
        int CheckColumnName(string &a_strColumnName);
        int InsertRule(CTCARule *a_cTCARule);
        int CheckRule(int a_nValue,  list<int> *a_lstAlarmList);
};

#endif
