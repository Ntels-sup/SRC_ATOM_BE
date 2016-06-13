#ifndef __CTABLELIST_HPP__
#define __CTABLELIST_HPP__

#include <map>
#include <string>
#include "CTableInfo.hpp"

using namespace std;

class CTableList{
    private:
        map<string, CTableInfo*> m_mapTableMap;

        time_t GetNumberTime(const char *chSrc, unsigned int nLen);
        int GetNumber(const char *chSrc, unsigned int nLen);
        int ParseFileName(string a_strFileName, string &a_strTableName, time_t &a_startTime, time_t &a_endTime, 
                int &a_nSequence);
    public:
        CTableList();
        ~CTableList();
        int InsertTableInfo(CTableInfo *a_cTable);
        CTableInfo* SearchTableInfo(string a_strTableName);
        int DeleteTableInfo(string a_strTableName);
        int Aggregation(int a_endTime, string a_strDir);
        int LoadTableData(string a_strFileName, list<string> **a_lstPrimaryKey, list<int> **a_lstValue);
        int LoadTableFromFile(string a_strDir);
};

#endif
