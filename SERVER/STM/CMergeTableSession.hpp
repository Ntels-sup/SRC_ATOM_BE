#ifndef __CMEREGTABLESESSION_HPP__
#define __CMEREGTABLESESSION_HPP__
#include <time.h>
#include <string>
#include <list>

using namespace std;

class CMergeTableSession{
    private:
        time_t m_hourStartTime;
        time_t m_hourEndTime;
        time_t m_dailyStartTime;
        time_t m_dailyEndTime;
        time_t m_dropTime;
        string m_strTableName;
        list<string> *m_lstPrimaryKeyColumn;
        list<string> *m_lstColumn;

    public:
        CMergeTableSession();
        ~CMergeTableSession();
        void SetTime(time_t a_hourStartTime, time_t a_hourEndTime, time_t a_dailyStartTime, 
                time_t a_dailyEndTime, time_t dropTime);
        void GetTime(time_t &a_hourStartTime, time_t &a_hourEndTime, time_t &a_dailyStartTime, 
                time_t &a_dailyEndTime, time_t &dropTime);
        void SetTableName(string &a_strTableName);
        string GetTableName() { return m_strTableName; };
        void SetPrimaryKeyColumn(list<string> *a_lstPrimaryKeyColumn);
        void SetColumn(list<string> *a_lstColumn);
        list<string>* GetPrimaryKeyColumnP() { return m_lstPrimaryKeyColumn; };
        list<string>* GetColumnP() { return m_lstColumn; };
};

#endif
