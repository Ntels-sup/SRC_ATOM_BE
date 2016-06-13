#ifndef __CTABLEINFO_HPP__
#define __CTABLEINFO_HPP__
#include <list>
#include <time.h>
#include "CColumnInfo.hpp"

using namespace std;

class CTableInfo{
    private:
        bool m_blnInitFlag;
        string m_strTableName;
        int m_nCollectTime;
        time_t m_nLastSendTime;
        int m_nType; 
        bool m_blnCollectFlag;
        list<CColumnInfo*> *m_lstColumnList;

    public:
        CTableInfo();
        ~CTableInfo();
        time_t GetLastSendTime() { return m_nLastSendTime; };
        int GetType() { return m_nType; };
        list<CColumnInfo*> *GetColumnLst() { return m_lstColumnList; };
        list<string> *GetPrimaryKeyColumnList();
        list<string> *GetValueColumnList();
        void SetLastSendTime(time_t a_nLastSendTime) { m_nLastSendTime = a_nLastSendTime; };
        int Init(const char *a_chTableName, int a_nCollectTime, int a_nType, bool a_blnCollectFlag);
        int InsertColumn(const char *a_chColumnName, int a_nSequence, const char *a_chDataForamt, bool a_blnPKFlag);
        int GetCollectTime() { return m_nCollectTime; };
        bool GetCollectFlag() { return m_blnCollectFlag; };
        string GetTableName() { return m_strTableName; };
        string GetColumnName(int a_nSequcnece);
        int CheckTableName(string &a_strTableName);
        CColumnInfo* SearchColumn(const char *a_chColumnName);
        string GetRcdInsertColumnStrList();
};

#endif

