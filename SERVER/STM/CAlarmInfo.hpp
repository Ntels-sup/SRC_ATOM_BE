#ifndef __CALARMINFO_HPP__
#define __CALARMINFO_HPP__

#include <time.h>
#include <string>
#include <list>

using namespace std;

class CAlarmInfo{
    private:
        int m_nHourlyCollectTime;
        int m_nDailyCollectTime;
        time_t m_lastHourlyCollectTime;
        time_t m_lastDailyCollectTime;
        int m_nCheckTime;
        int m_nDropTime;

        /* Resource stat database */
        string m_strDbName;
        string m_strDbIp;
        int m_nDbPort;
        string m_strDbUser;
        string m_strDbPasswd;

    public:
        CAlarmInfo();
        ~CAlarmInfo();
        int SetCollectTime(time_t a_nHourlyCollectTime, time_t a_nDailyCollectTime, 
                time_t a_nCheckTime, time_t a_nDropTime);
        int SetDbInfo(string &a_strDbName, string &a_strDbIp,
                int a_nDbPort, string &a_strDbUser, string &a_strDbPasswd);
        int CheckMarge(time_t curTm);
};

#endif
