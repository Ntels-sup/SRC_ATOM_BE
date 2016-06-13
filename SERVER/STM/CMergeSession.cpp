#include "CMergeSession.hpp"

CMergeSession::CMergeSession(int a_nType, string a_strPackageName, string a_strDBName, string a_strDBIp, int a_nDBPort,
				string a_strDBUser, string a_strDBPassword)
{
    m_nType = a_nType;
    m_strPackageName = a_strPackageName;
    m_strDBName = a_strDBName;
    m_strDBIp = a_strDBIp;
    m_nDBPort = a_nDBPort;
    m_strDBUser = a_strDBUser;
    m_strDBPassword = a_strDBPassword;
}

CMergeSession::~CMergeSession()
{
    CMergeTableSession *cTableSession = NULL;

    while(m_lstMergeTableList.size() != 0){
        cTableSession = m_lstMergeTableList.front();
        m_lstMergeTableList.pop_front();
        delete cTableSession;
    }
}

void CMergeSession::GetDBInfo(string &a_strDBName, string &a_strDBIp, int &a_nDBPort, string &a_strDBUser,
								string &a_strDBPassword, string &a_strPackageName)
{
    a_strPackageName = m_strPackageName;
    a_strDBName = m_strDBName;
    a_strDBIp = m_strDBIp;
    a_nDBPort = m_nDBPort;
    a_strDBUser = m_strDBUser;
    a_strDBPassword = m_strDBPassword;
}

void CMergeSession::SetTableSession(CMergeTableSession *cTableSession)
{
    m_lstMergeTableList.push_back(cTableSession);
}

CMergeTableSession *CMergeSession::GetFirstTableSession()
{
    CMergeTableSession *cTableSession = NULL;

    if(m_lstMergeTableList.size() == 0){
        return NULL;
    }

    cTableSession = m_lstMergeTableList.front();
    m_lstMergeTableList.pop_front();

    return cTableSession;
}

