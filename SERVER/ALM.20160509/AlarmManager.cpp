
#include <cstring>
#include <poll.h>

#include "DebugMacro.hpp"

#include "AlarmCFG.hpp"
#include "AlarmManager.hpp"

#include "MariaDB.hpp"


#include "CFileLog.hpp"

extern  CFileLog *      gAlmLog;
extern  AlarmCFG        gCFG;

AlarmManager::
AlarmManager()
{
    db_     = NULL;
}

AlarmManager::
~AlarmManager()
{
    if(db_ != NULL)
    {
        delete db_;
        db_         = NULL;
    }
}

bool AlarmManager::
Init()
{
    if(gCFG.Init() != true)
    {
        gAlmLog->ERROR("%-24s| Init - Config fail",
            "AlarmManager");
        return false;
    }

    if(db_ == NULL)
    {
        db_ = new (std::nothrow) MariaDB();

        if(db_ == NULL)
        {
            gAlmLog->ERROR("%-24s| Init - new operation fail For DB",
                "AlarmManager");
            return false;
        }

        // TO DO : Config 로 전환해야 합니다.
        if(db_->Connect(gCFG.DBInfo.ip_.c_str(),
                        gCFG.DBInfo.port_,
                        gCFG.DBInfo.user_.c_str(),
                        gCFG.DBInfo.pw_.c_str(),
                        gCFG.DBInfo.name_.c_str()) != true)
        {
            gAlmLog->ERROR("%-24s| Init - db connect fail [%d:%s]",
                "AlarmManager",
                db_->GetError(),
                db_->GetErrorMsg());

            delete db_;
            db_ = NULL;

            return false;
        }

        if(gCFG.SetProfile(db_) != true)
        {
            delete db_;
            db_     = NULL;
            return false;
        }
    }

    if(sequence_by_node_.Init(db_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - sequence by node fail",
            "AlarmManager");
        return false;
    }

    if(nm_if_.Init() != true)
    {
        gAlmLog->ERROR("%-24s| Init - node manager interface fail",
            "AlarmManager");
        return false;
    }

    if(event_factory_.Init(db_) != true)
    {
        gAlmLog->ERROR("%-24s| Init - event factory fail",
            "AlarmManager");
        return false;
    }

    gAlmLog->INFO("%-24s| Init - SUCCESS",
            "AlarmManager");

    return true;
}

void AlarmManager::
Do()
{
    if(nm_if_.IsEmpty() == true)
        return;

    EventHandler & h = event_factory_.Find(nm_if_.GetCommand());

    h.Clear();

    nm_if_.TakeOutBody(h.Buffer());

    if(h.DecodingJSON() != true)
    {
        gAlmLog->WARNING("%-24s| received msg is unavailable",
            "AlarmManager");
        return;
    }

    if(h.IsValidSequenceId(sequence_by_node_) != true)
    {
        gAlmLog->WARNING("%-24s| sequence id is invalid",
            "AlarmManager");
        requestRetry(h.GetNodeId(), sequence_by_node_.GetId(h.GetNodeId()));
        return ;
    }

    while(true)
    {
        if(h.Do(nm_if_, db_) == true)
            break;

        poll(NULL, 0, 0.5 * 1000);
    }
}

void AlarmManager::
requestRetry(int _node_no,  unsigned int  _seq_id)
{
    gAlmLog->DEBUG("inside requestRetry\n");

    // 메시지 생성

    // 전달

    // 현재 전달했으나, Agent 가 능력이 안되서 짝을 못맞춰주는 경우에는,
    // 재전송 처리에 올라오는 값으로 seq_id 를 ReSet 하여 처리 합니다.

}

void AlarmManager::
Final()
{
    // Empty
}


