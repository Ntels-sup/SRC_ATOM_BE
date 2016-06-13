#ifndef __CPACKAGEINFO_HPP__
#define __CPACKAGEINFO_HPP__
#include <map>
#include "MariaDB.hpp"
#include "STM.h"
#include "CTableInfo.hpp"
#include "CNodeInfo.hpp"

using namespace std;

class CPackageInfo{
    private:
        string m_strDBName;
        string m_strDBIp;
        int m_nDBPort;
        string m_strDBUser;
        string m_strDBPassword;
        DB *m_cDb;
        string m_strPackageName;
        list<CTableInfo*> *m_lstTableInfo;
        list<CNodeInfo*> *m_lstNodeInfo;
        bool m_blnCollectFlag;
        time_t m_lastDailyCollectTime;
        int m_nDailyCollectTime;
        time_t m_lastHourlyCollectTime;
        int m_nHourlyCollectTime;
        int m_nDropTime;
        int m_nCheckTime;
        int m_nHistDropTime;

    public:
        CPackageInfo();
        ~CPackageInfo();
        int Init(const char *a_szPackageName, const char *a_szDBName, const char *a_szDBIp, int a_nDBPort,
                const char *a_szDBUser, const char *a_szDBPassword);
        int InitConfig(int a_nDailyCollectTime, time_t a_lastDailyCollectTime, 
                int a_nHourlyCollectTime, time_t a_lastHourlyCollectTime, 
                int a_nDropTime, int a_nCheckTime, int a_nHistDropTime);
        int InsertTable(CTableInfo *cTable);
        int InsertNode(CNodeInfo *cNode);
        string GetPackageName() { return m_strPackageName; };
        int ResendCollectMessage();
        CTableInfo* GetTable(string &a_strTableName);
        int SendCollectMessage(string &a_strTableName,
                time_t a_nStartTime, unsigned int a_nCollectTime, bool a_blnDstFlag, bool a_blnActFlag);
        int CheckName(string &a_strPackageName);
        int CheckName(const char *a_chPackageName, unsigned int a_chPackageNameLen);
        int InsertQuery(string a_strQuery);
        string GetRcdInsertColumnStrList(string a_strTableName);
        int SqlQuery(string a_strQuery);
        int Run();
        int CheckMarge(time_t curTm);
};
#endif
