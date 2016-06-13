#ifndef __CMERAGE_HPP__
#define __CMERAGE_HPP__

#include <string>
#include <list>

#include "MariaDB.hpp"
#include "STM.h"
#include "CThreadQueue.hpp"
#include "CMergeSession.hpp"

using namespace std;

class CMerge{
    private:
        DB *m_cDb;
        CThreadQueue<CMergeSession*> *m_cRcvQueue;
        string GetUpdateColumnStr(list<string> &a_lstData);
        string GetColumnStr(list<string> &a_lstPrimaryKey);
        string GetSelectValueColumnStr(list<string> &a_lstValue);
        int TableProcess(int a_nType, CMergeTableSession *a_cTableSession);
        int ConvertTimestamp(time_t a_curTm, string &a_strTime);
        int DropProcess(time_t a_dropTime, string &a_strTableName);
        int AlmQueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval);
        int RscQueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval);
        int QueryProcess(time_t a_startTime, time_t a_endTime, int a_nInterval,
                string &a_strTableName,
                string &a_strPrimaryKeyList, string &a_strValueList,
                string &a_strSelectValueList, string &a_strUpdateValueList,
                string &a_strUpdatePrimaryKeyList);

    public:
        CMerge(CThreadQueue<CMergeSession*> *a_cRcvQueue);
        ~CMerge();

        //int Run(CMergeSession *cSession);
        int Run();
};

#endif
