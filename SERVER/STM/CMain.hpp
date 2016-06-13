#ifndef __CMAIN_HPP__
#define __CMAIN_HPP__
#include <list>

#include "CPackageInfo.hpp"
#include "CAlarmInfo.hpp"
#include "CNodeInfo.hpp"

using namespace std;

class CMain{
    private:
        DB *m_cDb;
        CStmConfig *m_cStmConfig;
        string m_strDbName;
        string m_strDbIp;
        int m_nDbPort;
        string m_strDbUser;
        string m_strDbPasswd;
        CResourceInfo *m_cResInfo;
        CAlarmInfo *m_cAlarmInfo;
        CLowerInterface *m_cLowerInter;
        list<CPackageInfo*> *m_lstPackageInfo;
        list<CNodeInfo*> *m_lstNodeInfo;

    public:
        CMain(int &nRet);
        ~CMain();
        int Init();
        int LoadPackage();
        int LoadNode();
        int LoadTable(CPackageInfo *a_cPackageInfo);
        int PackageRun();
        int ResourceHandler(time_t a_curTm);
        int NodeSendHeartBeat();
        int TCACheck(CSessionInfo *pcFindSession, CNodeInfo *a_pcNode);
        CPackageInfo *SearchPackage(const char *a_chPackageName, int a_nPacakgeNameLen);
        CPackageInfo *SearchPackage(string &a_strPackageName);
        CNodeInfo *SearchNode(int a_nNodeNo);
        int MargeHandler(time_t a_curTm);
        int ReceiveHandler();
        int ResourceHandler(time_t a_curTm, bool a_blnDstFlag);
        int UpdateStsValue(CSessionInfo *a_cSession, CNodeInfo *a_cNodeInfo);
        int UpdateColumnEnumIndex(string a_strTableName, string a_strPackageName, 
                string a_strColumnName, string a_strEnumValue);
        int UpdateResStsValue(CSessionInfo *a_cSession, CNodeInfo *a_cNodeInfo);
        int Run();
};

#endif
