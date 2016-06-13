
#include <cstring>
#include <poll.h>

#include "DebugMacro.hpp"

#include "AlarmCFG.hpp"
#include "AlarmManager.hpp"
#include "AlarmSTS.hpp"

#include "MariaDB.hpp"


#include "CFileLog.hpp"

extern  CFileLog *      gAlmLog;
extern  AlarmCFG        gCFG;

AlarmSTS                gSTS;

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
        if(gAlmLog != NULL)
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

		int ret = 0;
        if((ret = db_->Connect(gCFG.DBInfo.ip_.c_str(),
                        gCFG.DBInfo.port_,
                        gCFG.DBInfo.user_.c_str(),
                        gCFG.DBInfo.pw_.c_str(),
                        gCFG.DBInfo.name_.c_str())) < 0)
        {
            gAlmLog->ERROR("%-24s| Init - db connect fail [%d:%s]",
                "AlarmManager",
				ret,
                db_->GetErrorMsg(ret));

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

    // Cmd.Register(this);

    gAlmLog->INFO("%-24s| Init - SUCCESS",
            "AlarmManager");

    return true;
}

void AlarmManager::
Do()
{
    gSTS.Save(db_);

    if(nm_if_.GetData() == false)
        return;

    /*--
    if(Cmd.Do(nm_if_.GetCProtocol()) == true)
    {
        // Cmd 를 받았던 것이고, 처리를 했다는 뜻 입니다.
        return ;
    }
    --*/

    EventHandler & h = event_factory_.Find(nm_if_.GetCommand());

    h.Clear();

    nm_if_.TakeOutBody(h.Buffer());

    if(h.DecodingJSON() == false)
    {
        gAlmLog->WARNING("%-24s| received msg is unavailable",
            "AlarmManager");
        return;
    }

    if(h.IsValidSequenceId(sequence_by_node_) == false)
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
    rabbit::object      root;
    rabbit::object      retry = root["BODY"];

    retry["node_no"]    = _node_no;
    retry["req_seq_id"] = _seq_id;

    std::string     body = root.str();
    // 전달
    if(nm_if_.Notify(CMD_ALM_RETRY_REQ, _node_no, PROCID_ATOM_NA_ALA, body) == false)
    {
        gAlmLog->WARNING("%-24s| requestRetry - Notify Send Fail node [%d] seq id %u]",
            "AlarmManager",
            _node_no,
            _seq_id);
    }
    else
        gAlmLog->INFO("%-24s| requestRetry - Notify Send SUCCESS node [%d] seq id %u]",
            "AlarmManager",
            _node_no,
            _seq_id);
}

void AlarmManager::
Final()
{
    // Empty
}


