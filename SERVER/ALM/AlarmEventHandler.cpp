
#include "CommonCode.hpp"
#include "AlarmEventHandler.hpp"
#include "AlarmCFG.hpp"

#include "AlarmDuplication.hpp"

#include "CTimeUtil.hpp"

#include "AlarmSTS.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

extern AlarmSTS     gSTS;

AlarmEventHandler::
AlarmEventHandler()
{
    subscribe_h_        = NULL;
    buffer_.resize(gCFG.DEF.buffer_size_);

    set_db_alarm_ = false;
}

AlarmEventHandler::
~AlarmEventHandler()
{
    // Empty
}

// 처리 말미에 Noti 를 할 지도 모르니 까요..
bool AlarmEventHandler::
Init(DB                   * _db,
    SubscribeEventHandler * _subscribe_h)
{
    subscribe_h_ = _subscribe_h;

    if(mid_.Init(gCFG.ENV.message_id_path_.c_str(),
                 "history") == false)
    {
        gAlmLog->ERROR("%-24s| Init - sequence id init fail",
            "AlarmEventHandler");
        return false;
    }
    else
    {
        gAlmLog->INFO("%-24s| LAST SEQUENCE ID [%u]\n",
            "AlarmEventHandler", mid_.Get());
    }

    if(snmpId_.Init(gCFG.ENV.message_id_path_.c_str(),
                    "snmp") == false)
    {
        gAlmLog->ERROR("%-24s| Init - snmp sequence id init fail",
            "AlarmEventHandler");
        return false;
    }
    else
    {
        gAlmLog->INFO("%-24s| LInit - AST SNMP SEQUENCE ID [%u]\n",
            "AlarmEventHandler", snmpId_.Get());
    }


    setCCD(map_group_ccd_, "030100", _db);
    setCCD(map_type_ccd_,  "030102", _db);

    if(initAlarmConfMap(_db) == false)
    {
        gAlmLog->ERROR("%-24s| Init - can't init alarm config map",
            "AlarmEventHandler");

        return false;
    }

    if(snmp_.Init(_db) == false)
    {
        gAlmLog->ERROR("%-24s| Init - can't init Snmp",
            "AlarmEventHandler");
        return false;
    }

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "AlarmEventHandler");

    return true;
}

void AlarmEventHandler::
setCCD(std::unordered_map<int, std::string> & _map,
       const char                           * _group_cd,
       DB                                   * _db)
{
    char sql[256];
    sprintf(sql,
        "SELECT DETAIL_CD, CODE_NAME FROM TAT_COMMON_CODE "
        "WHERE GROUP_CD='%s'",
        _group_cd);

    int         ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->WARNING("%-24s| can't get alarm group ccd [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);

        return ;
    }

    char    detail_cd[8];
    char    code_name[128];

    f.Clear();
    f.Set(detail_cd, sizeof(detail_cd));
    f.Set(code_name, sizeof(code_name));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        _map[atoi(detail_cd)] = code_name;
    }
}

bool AlarmEventHandler::
initAlarmConfMap(DB * _db)
{
    map_alarm_conf_.clear();

    std::vector<std::string>   vec;

    if(getPkgName(vec, _db) == false)
    {
        gAlmLog->ERROR("%-24s| initAlarmConfMap - can't find pkg names",
            "AlarmEventHandler");

        return false;
    }

    for(auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        AlarmCodeConf   alarm_conf;

        if(alarm_conf.SetAlarmCode(_db, (*iter).c_str()) == false)
        {
            gAlmLog->ERROR("%-24s| initAlarmConfMap - can't init for alarm code conf",
                "AlarmEventHandler");
            return false;
        }

        //gAlmLog->DEBUG("%-24s| initAlarmConfMap ---- [%s] ",
        //    "AlarmEventHandler",
        //    (*iter).c_str());

        map_alarm_conf_[*iter] = alarm_conf;

        gAlmLog->INFO("%-24s| initAlarmConfMap - alarm code conf [%s]",
                "AlarmEventHandler",
                (*iter).c_str());
    }

    gAlmLog->INFO("%-24s| initAlarmConfMap - alarm conf map cnt [%d]",
        "NodeStatusEventHandler",
        map_alarm_conf_.size());

    return true;
}

bool AlarmEventHandler::
getPkgName(std::vector<std::string> & _vec,
            DB                      * _db)
{
    char sql[128];
    sprintf(sql,
        "SELECT distinct PKG_NAME FROM TAT_NODE_TYPE");

    int         ret = 0;
    FetchMaria  f;

    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
         gAlmLog->WARNING("%-24s| getPkgName -can't get pkg name [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);

         return false;
    }

    char pkg_name[DB_PROC_NAME_SIZE+1];

    f.Clear();
    f.Set(pkg_name,     sizeof(pkg_name));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        _vec.push_back(pkg_name);
    }

    return true;
}

void AlarmEventHandler::
Clear()
{
    gAlmLog->INFO("%-24s| Clear - START",
        "AlarmEventHandler");

    buffer_.clear();
    stAlarmEventMsg_.Clear();

    gAlmLog->INFO("%-24s| Clear - SUCCESS",
        "AlarmEventHandler");
}

bool AlarmEventHandler::
DecodingJSON(ST_AlarmEventMsg & _stAlarmEventMsg, std::string & _buffer)
{
    buffer_     = _buffer;

    if(DecodingJSON() == true)
    {
        _stAlarmEventMsg.Clear();
        _stAlarmEventMsg = stAlarmEventMsg_;
        return true;
    }

    return false;
}

bool AlarmEventHandler::
DecodingJSON()
{
    using namespace rapidjson;

    /*--
    {
        "node_seq_id" : ,
        "node_no"     : ,
        "node_name"   : ,
        "proc_no"     : ,
        "severity_id" : ,
        "pkg_name"    : " ",
        "event_type_id" :  " ",
        "event_group_id": " ",
        "code"        : " ",
        "alias_code"  : " ",
        "probable_cause" : " ",
        "additional_text" : " ",
        "location"    : " ",
        "target"      : " ",
        "complement"  : " ",
        "value"       : " ",
        "node_version": " ",
        "node_type"   : " ",
        "prc_date"   : " ",
        "dst_yn"      : " ",
    }
    --*/

    try {

        rabbit::document    doc;
        doc.parse(buffer_);

        rabbit::object      body = doc["BODY"];

        stAlarmEventMsg_.sequence_id_  =   body["node_seq_id"].as_uint();
        stAlarmEventMsg_.node_no_      =   body["node_no"].as_int();
        stAlarmEventMsg_.node_name_    =   body["node_name"].as_string();
        stAlarmEventMsg_.proc_no_      =   body["proc_no"].as_int();
        stAlarmEventMsg_.severity_id_  =   body["severity_id"].as_int();

        stAlarmEventMsg_.pkg_name_        = body["pkg_name"].as_string();
        stAlarmEventMsg_.event_type_id_   = body["event_type_id"].as_int();
        stAlarmEventMsg_.event_group_id_  = body["event_group_id"].as_int();
        stAlarmEventMsg_.code_            = body["code"].as_string();
        stAlarmEventMsg_.alias_code_      = body["alias_code"].as_string();
        stAlarmEventMsg_.probable_cause_  = body["probable_cause"].as_string();
        stAlarmEventMsg_.additional_text_ = body["additional_text"].as_string();
        stAlarmEventMsg_.location_        = body["location"].as_string();
        stAlarmEventMsg_.target_          = body["target"].as_string();
        stAlarmEventMsg_.complement_      = body["complement"].as_string();
        stAlarmEventMsg_.value_           = body["value"].as_string();
        stAlarmEventMsg_.node_version_    = body["node_version"].as_string();
        stAlarmEventMsg_.node_type_       = body["node_type"].as_string();
        stAlarmEventMsg_.prc_date_        = body["prc_date"].as_string();
        stAlarmEventMsg_.dst_yn_          = body["dst_yn"].as_string();


    } catch(rabbit::type_mismatch   e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - type mismatch [%s]",
            "AlarmEventHandler",
            buffer_.c_str());
        return false;
    } catch(rabbit::parse_error e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - parse error [%s]",
            "AlarmEventHandler",
            buffer_.c_str());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| DecodingJSON - [%s]",
            "AlarmEventHandler",
            buffer_.c_str());
        return false;
    }

    return true;
}

bool AlarmEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    return _sequence_by_node.IsItContinuously(stAlarmEventMsg_.node_no_,
                                              stAlarmEventMsg_.sequence_id_);
}

bool AlarmEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    gAlmLog->DEBUG("%-24s| Do - START",
        "AlarmEventHandler");

    if(stAlarmEventMsg_.code_.size() == 0)
    {
        gAlmLog->DEBUG("%-24s| Do - code is null",
                "AlarmEventHandler");
        return true;
    }

    stAlarmStatus_.Clear();

    stAlarmStatus_.SetFromAlarmEventMsg(stAlarmEventMsg_);

    stAlarmStatus_.msg_id_            = mid_.Generate();
    stAlarmStatus_.associated_msg_id_ = stAlarmStatus_.msg_id_;

    DoIfConnectStatus(stAlarmStatus_,   _db, _nm_if);
    DoIfPingStatus(stAlarmStatus_,      _db, _nm_if);
    DoIfProcessStatus(stAlarmStatus_,   _db, _nm_if);
    DoIfNodeStatus(stAlarmStatus_,      _db, _nm_if);

    if(applyAlarmStatus(_nm_if, _db))
    {

        gAlmLog->DEBUG("AlarmEventHandler - applyAlarmStatus SUCCESS");

        // 일반적인 Alarm Vnmf 으로 전송 여부를 체크하지 않아요.

        if(trySnmp(stAlarmStatus_) == true)
        {
            stAlarmHistory_.snmp_send_yn_ = "Y";
            stAlarmHistory_.snmp_seq_id_  = snmpId_.Get();
        }


        /*---
        if(alarm_control_.isSMS(stAlarmStatus_.pkg_name_,
                                stAlarmStatus_.code_) == true)
        {
            stAlarmHistory_.sms_send_yn_ = sms_if_->Send(stAlarmStatus_);
        }

        if(alarm_control_.isEmail(stAlarmStatus_.pkg_name_,
                                  stAlarmStatus_.code_) == true)
        {
            stAlarmHistory_.email_send_yn_ = email_if_->Send(stAlarmStatus_);
        }
        --*/

        // TO DO : Leader Msg ID 로 Noti 하도록 해야 합니다.
        std::string     body_for_notify = makeBodyForNotify(stAlarmStatus_);

        subscribe_h_->Notify(_nm_if, stAlarmStatus_, body_for_notify);

    }

    if(applyAlarmHistory(_db) == false)
    {
        gAlmLog->WARNING("%-24s| Do - applyAlarmHistory fail",
            "AlarmEventHandler");

        set_db_alarm_ = true;

        ST_AlarmStatus   alarm_status;
        alarm_status.Clear();

        makeAlarmStatusForDBFail(alarm_status, "EXECUTE");
        std::string     body_for_notify = makeBodyForNotify(alarm_status);

        subscribe_h_->Notify(_nm_if, alarm_status, body_for_notify);

        return false;
    }

    gSTS.Add(stAlarmStatus_.node_no_,
             stAlarmStatus_.severity_id_,
             stAlarmStatus_.prc_date_,
             stAlarmStatus_.dst_yn_);

    if(set_db_alarm_)
    {
        set_db_alarm_ = false;

        ST_AlarmStatus   alarm_status;
        alarm_status.Clear();

        makeAlarmStatusForDBFail(alarm_status, "CLEARED");
        std::string     body_for_notify = makeBodyForNotify(alarm_status);

        subscribe_h_->Notify(_nm_if, alarm_status, body_for_notify);
    }


    return true;
}


void AlarmEventHandler::
SetAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg)
{
    stAlarmEventMsg_    = _stAlarmEventMsg;
}

void AlarmEventHandler::
DoIfConnectStatus(ST_AlarmStatus & _stAlarmStatus,
                  DB             * _db,
                  NodeManagerIF  & _nm_if)
{
    if(_stAlarmStatus.code_.compare(ALRM_APP_CONNECT) != 0)
        return ;

    // DB Update
    if(updateConnectStatus(_db, _stAlarmStatus) == false)
        insertConnectStatus(_db, _stAlarmStatus);

    std::string     empty;

    _nm_if.Notify(CMD_STATUS_CONNECT_EVENT,
                  gCFG.NM.wsm_node_no_,
                  gCFG.NM.wsm_proc_no_,
                  empty);
}

bool AlarmEventHandler::
updateConnectStatus(DB             * _db,
                    ST_AlarmStatus & _stAlarmStatus)
{
    char    sql[256];
    if(_stAlarmStatus.value_.compare("CONNECT") == 0)
    {
        sprintf(sql,
            "UPDATE TAT_CONNECT SET CNT=CNT+1, UPDATE_DATE='%s' "
            " WHERE MY_IP='%s' AND PEER_IP='%s' AND SERVICE_NAME=%s",
            _stAlarmStatus.prc_date_.c_str(),
            _stAlarmStatus.location_.c_str(),
            _stAlarmStatus.target_.c_str(),
            _stAlarmStatus.complement_.c_str());

        return (_db->Execute(sql, strlen(sql)) > 0);
    }

    sprintf(sql,
        "UPDATE TAT_CONNECT SET CNT=CNT-1, UPDATE_DATE='%s' "
        " WHERE MY_IP='%s' AND PEER_IP='%s' AND SERVICE_NAME=%s AND CNT > 0",
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.location_.c_str(),
        _stAlarmStatus.target_.c_str(),
        _stAlarmStatus.complement_.c_str());

    // Connection 을 빼는 것은, UPDATE 실패해도 괜찮아요..
    // INSERT 할 껀 아니잖아요..
    return (_db->Execute(sql, strlen(sql)) >= 0);
}

bool AlarmEventHandler::
insertConnectStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus)
{
    // 나의 NODE 는 알고 있습니다.
    // Peer 의 NODE 를 찾아야 합니다.
    // PROTOCOL 도 찾습니다.
    // 쿼리를 한방에 하면, 문제를 모르니까, SELECT 하고, 이후 INSERT 할께요..
    // 욕하지 마셈.
    char    sql[256];
    sprintf(sql,
        "SELECT NODE_NO FROM TAT_NODE WHERE IP='%s'",
        _stAlarmStatus.target_.c_str());

    int             ret = 0;
    FetchMaria      f;
    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->WARNING("%-24s| insertConnectStatus - fail [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    char    peer_node_no[16];

    f.Clear();
    f.Set(peer_node_no, sizeof(peer_node_no));

    if(f.Fetch() == false)
    {
        gAlmLog->WARNING("%-24s| insertConnectStatus - fetch fail",
            "AlarmEventHandler");
        return false;
    }

    if(strlen(peer_node_no) == 0)
    {
        gAlmLog->WARNING("%-24s| insertConnectStatus - can't get peer node no ip:[%s]",
            "AlarmEventHandler",
            _stAlarmStatus.target_.c_str());
        return false;
    }


    sprintf(sql,
        "INSERT INTO TAT_CONNECT "
        " (MY_IP, PEER_IP, SERVICE_NAME, "
        "  MY_NODE_NO, PEER_NODE_NO, "
        "  STATUS_CCD, CNT, CREATE_DATE, UPDATE_DATE ) "
        " VALUES "
        " ('%s', '%s', %s, "
        "   %d, %s, "
        "  'SUCCESS', 1, '%s', '%s')",
        _stAlarmStatus.location_.c_str(),
        _stAlarmStatus.target_.c_str(),
        _stAlarmStatus.complement_.c_str(),
        _stAlarmStatus.node_no_,
        peer_node_no,
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.prc_date_.c_str());


    //gAlmLog->DEBUG("%-24s| insertConnectStatus - [%s]",
    //    "AlarmEventHandler",
    //    sql);

    if(_db->Execute(sql, strlen(sql)) > 0)
        return true;

    return false;
}

void AlarmEventHandler::
DoIfPingStatus(ST_AlarmStatus & _stAlarmStatus,
               DB             * _db,
               NodeManagerIF  & _nm_if)
{
    if(_stAlarmStatus.code_.compare(ALRM_NETWORK_FAIL) != 0)
        return ;

    // DB Update
    if(updatePingStatus(_db, _stAlarmStatus) == false)
        insertPingStatus(_db, _stAlarmStatus);

    std::string     empty;

    _nm_if.Notify(CMD_STATUS_PING_EVENT,
                  gCFG.NM.wsm_node_no_,
                  gCFG.NM.wsm_proc_no_,
                  empty);
}

bool AlarmEventHandler::
updatePingStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus)
{
    char        sql[256];

    sprintf(sql,
        "UPDATE TAT_CONNECT SET STATUS_CCD='%s', UPDATE_DATE='%s' "
        " WHERE MY_IP='%s' AND PEER_IP='%s'",
        _stAlarmStatus.value_.c_str(),
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.location_.c_str(),
        _stAlarmStatus.target_.c_str());

        gAlmLog->DEBUG("# [%s]", sql);

    return (_db->Execute(sql, strlen(sql)) > 0);
}

bool AlarmEventHandler::
insertPingStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus)
{
    char    sql[256];

    sprintf(sql,
        "INSERT INTO TAT_CONNECT "
        " (MY_IP, PEER_IP, SERVER_L4_PORT, "
        "  MY_NODE_NO, PEER_NODE_NO,  "
        "  STATUS_CCD, CREATE_DATE, UPDATE_DATE ) "
        " VALUES "
        " ('%s', '%s', 0, "
        "  %d, %d, "
        "  '%s', '%s', '%s')",
        _stAlarmStatus.location_.c_str(),
        _stAlarmStatus.target_.c_str(),

        _stAlarmStatus.node_no_,
        atoi(_stAlarmStatus.additional_text_.c_str()),

        _stAlarmStatus.value_.c_str(),
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.prc_date_.c_str());

        gAlmLog->DEBUG("# [%s]", sql);

    return _db->Execute(sql, strlen(sql)) > 0;
}

void AlarmEventHandler::
DoIfProcessStatus(ST_AlarmStatus & _stAlarmStatus,
                  DB             * _db,
                  NodeManagerIF  & _nm_if)
{
    if(_stAlarmStatus.code_.compare(ALRM_PROCESS_STATUS) != 0)
        return ;

    // DB Update
    if(updateProcessStatus(_db, _stAlarmStatus) == false)
        return ;

    // Status Arragne
    if(_stAlarmStatus.value_.compare("STOPPED") == 0 ||
       _stAlarmStatus.value_.compare("ABNORMAL") == 0)
    {
        std::vector<ST_AlarmStatus>   vec(10);

        gatherSameProcessFromStatusTBL(vec,
                                       _db,
                                       _stAlarmStatus.node_no_,
                                       _stAlarmStatus.proc_no_);

        for(auto iter=vec.begin(); iter != vec.end(); ++iter)
        {
            updateHistoryToClear(_db, (*iter).msg_id_, _stAlarmStatus);
            ST_AlarmStatus::Delete(_db, (*iter).msg_id_);
        }

        if(vec.size() > 0)
            sendToALAForSync(vec,
                             _stAlarmStatus.node_no_,
                             _stAlarmStatus.proc_no_,
                             _nm_if);
    }

    std::string     empty;

    _nm_if.Notify(CMD_STATUS_PROC_EVENT,
                  gCFG.NM.wsm_node_no_,
                  gCFG.NM.wsm_proc_no_,
                  empty);
}

bool AlarmEventHandler::
updateProcessStatus(DB * _db, ST_AlarmStatus & _stAlarmStatus)
{
    char    sql[256];
    sprintf(sql,
        "UPDATE TAT_NODE SET PROC_STATUS_CCD='%s', UPDATE_DATE='%s' "
        " WHERE NODE_NO=%d",
        _stAlarmStatus.complement_.c_str(),
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.node_no_);

    gAlmLog->DEBUG("# [%s]", sql);

    int     ret = 0;
    if((ret = _db->Execute(sql, strlen(sql))) <= 0)
    {
        gAlmLog->WARNING("%-24s| updateProcessStatus - fail [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    gAlmLog->DEBUG("%-24s| updateProcessStatus - update success",
        "AlarmEventHandler");

    return true;
}


void AlarmEventHandler::
gatherSameProcessFromStatusTBL(std::vector<ST_AlarmStatus> & _vec,
                               DB                          * _db,
                               int                           _node_no,
                               int                           _proc_no)
{
    char sql[256];
    sprintf(sql,
        "SELECT MSG_ID, CODE, SEVERITY_CCD, LOCATION, TARGET, COMPLEMENT "
        " FROM TAT_ALM_STATUS "
        " WHERE NODE_NO=%d AND PROC_NO=%d",
        _node_no,
        _proc_no);

    FetchMaria  f;
    if(_db->Query(&f, sql, strlen(sql)) <= 0)
        return ;

    char    msg_id[16];
    char    code[DB_ALM_CODE_SIZE+1];
    char    severity_id[4];
    char    location[DB_ALM_LOCATION_SIZE+1];
    char    target[DB_ALM_OBJECT_SIZE+1];
    char    complement[DB_ALM_COMPLEMENT_SIZE+1];

    f.Clear();
    f.Set(msg_id,       sizeof(msg_id));
    f.Set(code,         sizeof(code));
    f.Set(severity_id,  sizeof(severity_id));
    f.Set(location,     sizeof(location));
    f.Set(target,       sizeof(target));
    f.Set(complement,   sizeof(complement));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        ST_AlarmStatus  stAlarmStatus;

        stAlarmStatus.Clear();
        stAlarmStatus.node_no_      = _node_no;
        stAlarmStatus.proc_no_      = _proc_no;

        stAlarmStatus.msg_id_       = atoi(msg_id);
        stAlarmStatus.code_         = code;
        stAlarmStatus.severity_id_  = atoi(severity_id);
        stAlarmStatus.location_     = location;
        stAlarmStatus.target_       = target;
        stAlarmStatus.complement_   = complement;

        _vec.push_back(stAlarmStatus);
    }
}

bool AlarmEventHandler::
sendToALAForSync(std::vector<ST_AlarmStatus> & _vec,
                 int                           _node_no,
                 int                           _proc_no,
                 NodeManagerIF               & _nm_if)
{
    AlarmDuplication    duplication;

    std::string     temp;

    duplication.MakeBodyForNotify(temp, _vec);

    return _nm_if.Notify(CMD_MANAGE_DUPLICATE,
                         _node_no,
                         _proc_no,
                         temp);
}

void AlarmEventHandler::
DoIfNodeStatus(ST_AlarmStatus & _stAlarmStatus,
                DB            * _db,
                NodeManagerIF & _nm_if)
{
    if(_stAlarmStatus.code_.compare(ALRM_NODE_STATUS) != 0)
        return ;

    // DB Update
    if(updateNodeStatus(_db, _stAlarmStatus) == false)
        return ;

    // Status Arragne
    if(_stAlarmStatus.value_.compare("SCALE_IN") == 0 ||
       _stAlarmStatus.value_.compare("SCALE_OUT") == 0 ||
       _stAlarmStatus.value_.compare("REMOVE") == 0)
    {
        std::vector<unsigned int>   vec;

        gatherSameNodeFromStatusTBL(vec,
                                    _db,
                                    _stAlarmStatus.node_no_);

        for(unsigned int nLoop=0; nLoop < vec.size(); nLoop++)
        {
            updateHistoryToClear(_db, vec.at(nLoop), _stAlarmStatus);
            ST_AlarmStatus::Delete(_db, vec.at(nLoop));
        }
    }

    // TO DO : Sync Request ?? SCREEN ??

    std::string     empty;

    _nm_if.Notify(CMD_STATUS_NODE_EVENT,
                  gCFG.NM.wsm_node_no_,
                  gCFG.NM.wsm_proc_no_,
                  empty);
}

bool AlarmEventHandler::
updateNodeStatus(DB              * _db,
                 ST_AlarmStatus  & _stAlarmStatus)
{

    char sql[128];
    sprintf(sql,
        "UPDATE TAT_NODE SET NODE_STATUS_CCD='%s', UPDATE_DATE='%s' "
        " WHERE NODE_NO=%d",
        _stAlarmStatus.value_.c_str(),
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.node_no_);

        gAlmLog->DEBUG("# [%s]", sql);

    int     ret = 0;
    if((ret = _db->Execute(sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| updateNodeStatus - execute fail [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    return true;
}

void AlarmEventHandler::
gatherSameNodeFromStatusTBL(std::vector<unsigned int> & _vec,
                            DB                        * _db,
                            int                         _node_no)
{
    char sql[256];
    sprintf(sql,
        "SELECT MSG_ID FROM TAT_ALM_STATUS WHERE NODE_NO=%d",
        _node_no);

    FetchMaria  f;
    if(_db->Query(&f, sql, strlen(sql)) <= 0)
        return ;

    char    msg_id[16];

    f.Clear();
    f.Set(msg_id,      sizeof(msg_id));

    while(true)
    {
        if(f.Fetch() == false)
            break;

        _vec.push_back(atoi(msg_id));
    }
}

bool AlarmEventHandler::
applyAlarmStatus(NodeManagerIF & _nm_if, DB * _db)
{

    ST_AlarmStatus      stLeaderAlarmStatus;
    stLeaderAlarmStatus.Clear();

    // 동일 알람 찾기
    // Node ID + PROC ID + Code + Location + Object + Complement
    if(findLeaderAlarmMsg(stLeaderAlarmStatus, _db, stAlarmStatus_) != true)
    {
        gAlmLog->DEBUG("Leader Alarm is not exist");

        if(stAlarmStatus_.severity_id_ != ALM::eCLEARED)
            return stAlarmStatus_.Insert(_db);

        return true;
    }

    stAlarmStatus_.associated_msg_id_ = stLeaderAlarmStatus.msg_id_;

    // Same + Severity
    if(stAlarmStatus_.CompareSeverity(stLeaderAlarmStatus.severity_id_) == true)
    {
        gAlmLog->DEBUG("Leader Alarm is the same [severity:%d]",
            stAlarmStatus_.severity_id_);

        updateLeaderAlarm(_db, stLeaderAlarmStatus.msg_id_, stAlarmStatus_);

        if(gCFG.SVC.b_avoide_duplicate_)
        {
            std::vector<ST_AlarmStatus>     vec(1);
            vec.push_back(stAlarmStatus_);

            sendToALAForSync(vec,
                             stAlarmStatus_.node_no_,
                             stAlarmStatus_.proc_no_,
                             _nm_if);
        }

        return false;
    }

    // Same, but severity is different
    if(stAlarmStatus_.severity_id_ == ALM::eCLEARED)
    {
        gAlmLog->DEBUG("Clear Alarm");

        updateHistoryToClear(_db, stLeaderAlarmStatus.msg_id_, stAlarmStatus_);

        gAlmLog->DEBUG("Leader Alarm is going to delete");

        ST_AlarmStatus::Delete(_db, stLeaderAlarmStatus.msg_id_);
    }
    else
    {
        gAlmLog->DEBUG("Leader Alarm is existed, But Severity is different");

        if(stLeaderAlarmStatus.confirm_yn_.compare("Y") == 0)
            return false;

        updateLeaderAlarm(_db, stLeaderAlarmStatus.msg_id_, stAlarmStatus_);
    }

    return true;
}

bool AlarmEventHandler::
findLeaderAlarmMsg(ST_AlarmStatus & _stLeaderAlarmStatus,
                   DB             * _db,
                   ST_AlarmStatus & _stAlarmStatus)
{
    int  len = 0;
    char sql[512];

    len =
    sprintf(sql,
        "SELECT MSG_ID, SEVERITY_CCD, CONFIRM_YN"
        " FROM TAT_ALM_STATUS WHERE CODE='%s' AND NODE_NO=%d AND PROC_NO=%d ",
        _stAlarmStatus.code_.c_str(),
        _stAlarmStatus.node_no_,
        _stAlarmStatus.proc_no_);

    char * p = sql + len;

    if(_stAlarmStatus.location_.size() > 0)
    {
        len = sprintf(p, " AND LOCATION='%s' ", _stAlarmStatus.location_.c_str());
        p += len;
    }

    if(_stAlarmStatus.target_.size() > 0)
    {
        len = sprintf(p, " AND TARGET='%s' ", _stAlarmStatus.target_.c_str());
        p += len;
    }

    if(_stAlarmStatus.complement_.size() > 0)
    {
        len = sprintf(p, " AND COMPLEMENT='%s' ", _stAlarmStatus.complement_.c_str());
        p += len;
    }

    int ret = 0;
    FetchMaria      f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| findLeaderAlarm -  query fail [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);
        return false;
    }

    if(ret == 0)
        return false;

    char    msg_id[32];
    char    severity_id[16];
    char    confirm_yn[4];

    f.Clear();
    f.Set(msg_id,       sizeof(msg_id));
    f.Set(severity_id,  sizeof(severity_id));
    f.Set(confirm_yn,   sizeof(confirm_yn));

    if(f.Fetch() == false)
    {
        gAlmLog->WARNING("%-24s| findLeaderAlarm - fetch is fail [rows:%d]",
            "AlarmEventHandler",
            ret);

        return false;
    }

    _stLeaderAlarmStatus.msg_id_        = strtoul(msg_id, NULL, 10);
    _stLeaderAlarmStatus.severity_id_   = atoi(severity_id);
    _stLeaderAlarmStatus.confirm_yn_    = confirm_yn;

    return true;
}


bool AlarmEventHandler::
updateHistoryToClear(DB *               _db,
                     unsigned int       _associated_msg_id,
                     ST_AlarmStatus &   _stAlarmStatus)
{
    // associated 으로 조회하여, 모두 clear 시킵니다.
    char sql[256];
    sprintf(sql,
        "UPDATE TAT_ALM_HIST SET "
    //    " SEVERITY_CCD=%d, "
        " CLEARED_DATE='%s', "
        " CLEARED_YN='Y', "
        " CLEARED_MSG_ID=%u "
        " WHERE ASSOCIATED_MSG_ID=%u",
    //    ALM::eCLEARED,
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.msg_id_,
        _associated_msg_id);

    int ret = 0;
    ret = _db->Execute(sql, strlen(sql));

    if(ret < 0)
    {
        gAlmLog->DEBUG("updateHistoryToClear - execute fail [%d:%s] [%s]",
            ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    return true;
}

// SEVERITY, VALUE, LST_DATE, OPER_NO
bool AlarmEventHandler::
updateLeaderAlarm(DB             *    _db,
                  unsigned int        _to_msg_id,
                  ST_AlarmStatus &    _stAlarmStatus)
{
    char sql[256];
    sprintf(sql, "UPDATE TAT_ALM_STATUS SET "
                 " SEVERITY_CCD=%d, "
                 " VALUE='%s', "
                 " LST_DATE='%s' "
                 " WHERE MSG_ID=%u",
                 _stAlarmStatus.severity_id_,
                 _stAlarmStatus.value_.c_str(),
                 _stAlarmStatus.prc_date_.c_str(),
                 _to_msg_id);

    int     ret = 0;
    if((ret = _db->Execute(sql, strlen(sql))) < 0)
    {
        gAlmLog->WARNING("%-24s| updateLeaderAlarm - execute fail [%d:%s] [%s]",
            "AlarmEventHandler",
            ret,
            _db->GetErrorMsg(ret),
            sql);

        return false;
    }

    return true;
}

bool AlarmEventHandler::
applyAlarmHistory(DB * _db)
{
    stAlarmHistory_.Clear();

    stAlarmHistory_.stAlarmStatus_ = stAlarmStatus_;
    stAlarmHistory_.node_seq_id_   = stAlarmEventMsg_.sequence_id_;

    if(stAlarmEventMsg_.severity_id_ == ALM::eCLEARED)
    {
        stAlarmHistory_.cleared_date_   = stAlarmEventMsg_.prc_date_;
        stAlarmHistory_.cleared_yn_     = "Y";
        stAlarmHistory_.cleared_msg_id_ = stAlarmStatus_.msg_id_;
    }

    return stAlarmHistory_.Insert(_db);
}

std::string AlarmEventHandler::
makeBodyForNotify(ST_AlarmStatus & _stAlarmStatus)
{
rabbit::object      root;

    rabbit::object     body = root["BODY"];

    body["message"]         =   "alarm";
    // MSG ID 값은 ALM_HIST 에 들어가는 자신의 MSG ID 를 보내는 대신에,
    // 현재 발생 중인 ALARM ID 에 대해서 NOTI 합니다.
    // body["msg_id"]          =   _stAlarmStatus.msg_id_;
    body["msg_id"]          =   _stAlarmStatus.associated_msg_id_;
    body["node_no"]         =   _stAlarmStatus.node_no_;
    body["node_name"]       =   _stAlarmStatus.node_name_;
    body["proc_no"]         =   _stAlarmStatus.proc_no_;
    body["severity_id"]     =   _stAlarmStatus.severity_id_;

    body["pkg_name"]        =   _stAlarmStatus.pkg_name_;
    body["code"]            =   _stAlarmStatus.code_;
    body["event_type_id"]   =   _stAlarmStatus.event_type_id_;
    body["event_group_id"]  =   _stAlarmStatus.event_group_id_;

    if(map_type_ccd_.find(_stAlarmStatus.event_type_id_) != map_type_ccd_.end())
        body["event_type_name"] =
            map_type_ccd_[_stAlarmStatus.event_type_id_].c_str();
    else
        body["event_type_name"] = "";

    if(map_group_ccd_.find(_stAlarmStatus.event_group_id_) != map_group_ccd_.end())
        body["event_group_name"]=
            map_group_ccd_[_stAlarmStatus.event_group_id_].c_str();
    else
        body["event_group_name"]= "";

    body["alias_code"]      =   _stAlarmStatus.alias_code_;
    body["probable_cause"]  =   _stAlarmStatus.probable_cause_;
    body["additional_text"] =   _stAlarmStatus.additional_text_;
    body["location"]        =   _stAlarmStatus.location_;
    body["target"]          =   _stAlarmStatus.target_;
    body["complement"]      =   _stAlarmStatus.complement_;
    body["value"]           =   _stAlarmStatus.value_;
    body["node_version"]    =   _stAlarmStatus.node_version_;
    body["node_type"]       =   _stAlarmStatus.node_type_;
    body["prc_date"]        =   _stAlarmStatus.prc_date_;
    body["lst_date"]        =   _stAlarmStatus.lst_date_;
    body["oper_no"]         =   _stAlarmStatus.oper_no_;
    body["dst_yn"]          =   _stAlarmStatus.dst_yn_;
    body["manual_yn"]       =   _stAlarmStatus.manual_yn_;

    return root.str();
}

void AlarmEventHandler::
makeAlarmStatusForDBFail(ST_AlarmStatus & _stAlarmStatus,
                         const char     * _value)
{
    _stAlarmStatus.msg_id_          = mid_.Generate();
    _stAlarmStatus.node_no_         = gCFG.profile_.m_nNodeNo;
    _stAlarmStatus.node_name_       = "ALM";
    _stAlarmStatus.proc_no_         = gCFG.profile_.m_nProcNo;

    _stAlarmStatus.pkg_name_        = gCFG.profile_.m_strPkgName;
    _stAlarmStatus.code_            = ALRM_DB_EXECUTE_FAIL;

    _stAlarmStatus.location_        = "/ATOM/ALM";
    _stAlarmStatus.target_          = "DB";
    _stAlarmStatus.value_           = _value;

    char    dst = 'N';
    CTimeUtil::SetTimestampAndDstYn(_stAlarmStatus.prc_date_, dst, 2);

    _stAlarmStatus.lst_date_        = _stAlarmStatus.prc_date_;
    _stAlarmStatus.oper_no_         = -1;
    _stAlarmStatus.dst_yn_          = dst;
    _stAlarmStatus.manual_yn_       = "N";

    return ;
}

bool AlarmEventHandler::
trySnmp(ST_AlarmStatus & _stAlarmStatus)
{
    auto iter = map_alarm_conf_.find(_stAlarmStatus.pkg_name_);

    if(iter == map_alarm_conf_.end())
        return false;

    AlarmCodeConf & alarm_conf = iter->second;

    if(alarm_conf.IsSNMP(_stAlarmStatus.code_) == false)
        return false;

    char bindValue[1024];
    sprintf(bindValue,
        "%s %s %s %d %d %s %u",
        _stAlarmStatus.code_.c_str(),
        _stAlarmStatus.location_.c_str(),
        _stAlarmStatus.prc_date_.c_str(),
        _stAlarmStatus.severity_id_,
        _stAlarmStatus.event_group_id_,
        _stAlarmStatus.probable_cause_.c_str(),
        snmpId_.Generate()
        );

    return snmp_.SendTrap(_stAlarmStatus.pkg_name_.c_str(),
                          _stAlarmStatus.code_.c_str(),
                          bindValue);
}
