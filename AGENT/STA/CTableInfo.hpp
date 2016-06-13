#ifndef __CTABLEINFO_HPP__
#define __CTABLEINFO_HPP__
#include <list>
#include <time.h>

#include "CStsValue.hpp"
#include "CStsValueList.hpp"
#include "CCollectValue.hpp"

class CTableInfo{
    private:
        string m_strTableName;
        list<CStsValueList*> *m_lstStsValueList;

        time_t GetNumberTime(const char *chSrc, unsigned int nLen);
        int ParseFileName(string a_strFileName, string &a_strTableName, time_t a_startTime, time_t a_endTime);
    public: 
        CTableInfo(string m_strTableName);
        ~CTableInfo();
        void SetTableName(string a_strTableName);
        int GetValueCount();
        string GetTableName() { return m_strTableName; };
        int InsertValue(time_t a_collectStartTime, time_t a_collectEndTime, 
                list<string> *a_lstPrimaryKey, list<int> *a_lstValue, string *a_strFileName);
        int DropStsValue(time_t a_dropTime);
        list<CCollectValue*> *GetCollectValue(time_t a_nStartTime, int a_nCollectTime);
        //				int Aggregation(time_t a_endTime);
        int Aggregation(time_t a_endTime, string a_strDir);
        int DropAll();
        int LoadTableFromFile(string a_strDir);
};

#endif

