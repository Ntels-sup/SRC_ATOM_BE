#ifndef __CMEREGSESSION_HPP__
#define __CMEREGSESSION_HPP__
#include <time.h>
#include <string>
#include <list>

#include "CMergeTableSession.hpp"

using namespace std;

class CMergeSession{
    private:
        int m_nType; 
        string m_strDBName;
        string m_strDBIp;
        int m_nDBPort;
        string m_strDBUser;
        string m_strDBPassword;
        string m_strPackageName;

        list<CMergeTableSession*> m_lstMergeTableList;

    public:
        CMergeSession(int a_nType, string a_strPackageName, string a_strDBName, string a_strDBIp, int m_nDBPort,
                string a_strDBUser, string a_strDBPassword);
        ~CMergeSession();
        int GetType() { return m_nType; };
        void SetTableSession(CMergeTableSession *cTableSession);
        void GetDBInfo( string &a_strDBName, string &a_strDBIp, int &a_nDBPort, string &a_strDBUser,
                string &a_strDBPassword, string &a_strPackageName);
        CMergeTableSession *GetFirstTableSession();
};

#endif
