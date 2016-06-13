

#include "CommonCode.hpp"
#include "NodeStatusEventHandler.hpp"
#include "AlarmCFG.hpp"
#include "FetchMaria.hpp"

#include "AlarmCodeConf.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

NodeStatusEventHandler::
NodeStatusEventHandler()
{
    alarm_h_    = NULL;
    buffer_.resize(gCFG.DEF.buffer_size_);
}

NodeStatusEventHandler::
~NodeStatusEventHandler()
{
    // Empty
}

bool NodeStatusEventHandler::
Init(DB * _db, AlarmEventHandler * _alarm_h)
{
    alarm_h_ = _alarm_h;

    std::vector<std::pair<std::string, std::string> >    vec;

    if(getPkgNamesFromAlarmCodeDef(vec, _db) == false)
    {
        gAlmLog->ERROR("%-24s| Init - can't find pkg names",
            "NodeStatusEventHandler");

        return false;
    }

    for(auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        AlarmDecision   alarm_decision;

        if(alarm_decision.Init(_db, iter->first, iter->second) == false)
        {
            gAlmLog->ERROR("%-24s| Init - can't init for alarm decision",
                "NodeStatusEventHandler");
            return false;
        }

        list_.push_back(alarm_decision);
        gAlmLog->INFO("%-24s| Init - alarm decision [%s] [%s]",
                "NodeStatusEventHandler",
                (iter->first).c_str(),
                (iter->second).c_str());
    }

    gAlmLog->INFO("%-24s| Init - alarm decision list cnt [%d]",
        "NodeStatusEventHandler",
        list_.size());

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "NodeStatusEventHandler");

    return true;
}

bool NodeStatusEventHandler::
getPkgNamesFromAlarmCodeDef(std::vector<std::pair<std::string,
                                                  std::string> > & _vec,
                            DB * _db)
{
    char sql[128];
    sprintf(sql,
        "SELECT PKG_NAME, NODE_TYPE FROM TAT_NODE_TYPE");

    if(_db->Query(sql, strlen(sql)) <= 0)
    {
         gAlmLog->WARNING("%-24s| can't get pkg name and node type [%d:%s] [%s]",
            "NodeStatusEventHandler",
            _db->GetError(),
            _db->GetErrorMsg(),
            sql);

         return false;
    }

    char pkg_name[DB_PROC_NAME_SIZE+1];
    char node_type[DB_NODE_TYPE_SIZE+1];

    FetchMaria  f;
    f.Clear();

    f.Set(pkg_name,     sizeof(pkg_name));
    f.Set(node_type,    sizeof(node_type));

    while(true)
    {
        if(f.Fetch(_db) == false)
            break;

        // _vec.push_back(std::make_pair(pkg_name, node_type));
        _vec.push_back({pkg_name, node_type});
    }

    return true;
}

void NodeStatusEventHandler::
Clear()
{
    buffer_.clear();
}

bool NodeStatusEventHandler::
DecodingJSON()
{
    using namespace rapidjson;

    /*--
    {
        "message"   : ,
        "pkg_name"  : ,
        "node_no"   : ,
        "node_name" : ,
        "node_version"    : " ",
        "node_type"        : " ",
        "node_status"  : " ",
        "prc_date" : " ",
        "dst_yn"      : " ",
    }
    --*/

    gAlmLog->DEBUG("%-24s| DecodingJSON - [%s]",
        "NodeStatusEventHandler",
        buffer_.c_str());

    try {

        stAlarmEventMsg_.Clear();

        rabbit::document    doc;
        doc.parse(buffer_);

        rabbit::object      body = doc["BODY"];

        stAlarmEventMsg_.message_      =   body["message"].as_string();
        stAlarmEventMsg_.node_no_      =   body["node_no"].as_int();
        // stAlarmEventMsg_.severity_id_  =   body["severity_id"].as_int();

        stAlarmEventMsg_.pkg_name_        = body["pkg_name"].as_string();
        // stAlarmEventMsg_.code_            = body["code"].as_string();
        // stAlarmEventMsg_.alias_code_      = body["alias_code"].as_string();
        // stAlarmEventMsg_.probable_cause_  = body["probable_cause"].as_string();
        // stAlarmEventMsg_.additional_text_ = body["additional_text"].as_string();
        stAlarmEventMsg_.location_        = "/ATOM/NM";
        stAlarmEventMsg_.target_          = "/";
        stAlarmEventMsg_.target_.append(stAlarmEventMsg_.pkg_name_);
        stAlarmEventMsg_.target_.append("/");
        stAlarmEventMsg_.target_.append(body["node_name"].as_string());
        stAlarmEventMsg_.value_           = body["node_status"].as_string();
        stAlarmEventMsg_.node_version_    = body["node_version"].as_string();
        stAlarmEventMsg_.node_type_       = body["node_type"].as_string();
        stAlarmEventMsg_.prc_date_        = body["prc_date"].as_string();
        stAlarmEventMsg_.dst_yn_          = body["dst_yn"].as_string();


    } catch(rabbit::type_mismatch &  e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - type mismatch [%s]",
            "NodeStatusEventHandler",
            e.what());
        return false;
    } catch(rabbit::parse_error & e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - parse error [%s]",
            "NodeStatusEventHandler",
            e.what());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| DecodingJSON - can't decoding",
            "NodeStatusEventHandler");
        return false;
    }

    return true;
}


// IF 를 위함..
bool NodeStatusEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    return true;
}

// Do() 함수는 false 를 반환하면, 무한반복으로 시도 합니다.
bool NodeStatusEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    gAlmLog->DEBUG("%-24s| Do - START",
        "NodeStatusEventHandler");

    stAlarmEventMsg_.code_  = ALRM_NODE_STATUS;
    if(fillAlarmEventMsg(stAlarmEventMsg_) == false)
    {
        gAlmLog->WARNING("%-24s| Do - unknown pkg name [%s] and node type [%s]",
            "NodeStatusEventHandler",
            stAlarmEventMsg_.pkg_name_.c_str(),
            stAlarmEventMsg_.node_type_.c_str());
        return true;
    }

    alarm_h_->SetAlarmEventMsg(stAlarmEventMsg_);

    if(alarm_h_->Do(_nm_if, _db) == false)
        return false;

    notify(_nm_if);

    return true;
}

bool NodeStatusEventHandler::
fillAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg)
{
    auto iter = list_.begin();
    for( ; iter != list_.end(); ++iter)
    {
        if(iter->EqualPkgNameAndNodeType(_stAlarmEventMsg.pkg_name_,
                                         _stAlarmEventMsg.node_type_) == true)
            break;
    }

    if(iter == list_.end())
    {
        gAlmLog->WARNING("%-24s| fillAlarmEventMsg - can't find alarm decision",
            "NodeStatusEventHandler");

        return false;
    }

    gAlmLog->DEBUG("%-24s| fillAlarmEventMsg",
        "NodeStatusEventHandler");

    AlarmDecision & alarm_decision = *iter;

    alarm_decision.Do(_stAlarmEventMsg);

    return true;
}

void NodeStatusEventHandler::
notify(NodeManagerIF & _nm_if)
{
    // std::string     empty;

    _nm_if.Notify(CMD_STATUS_NODE_EVENT,
                  gCFG.NM.wsm_node_no_,
                  gCFG.NM.wsm_proc_no_,
                  buffer_);
}