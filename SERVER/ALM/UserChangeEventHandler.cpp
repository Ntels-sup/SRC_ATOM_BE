
#include "UserChangeEventHandler.hpp"
#include "AlarmCFG.hpp"
#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;
extern AlarmCFG     gCFG;

UserChangeEventHandler::
UserChangeEventHandler()
{
    alarm_h_ = NULL;
    buffer_.resize(gCFG.DEF.buffer_size_);
}

UserChangeEventHandler::
~UserChangeEventHandler()
{
    // Empty
}

// 처리 말미에
bool UserChangeEventHandler::
Init(AlarmEventHandler * _alarm_h)
{
    alarm_h_ = _alarm_h;

    gAlmLog->INFO("%-24s| Init - SUCCESS",
        "UserChangeEventHandler");
    return true;
}

void UserChangeEventHandler::
Clear()
{
    buffer_.clear();
}

bool UserChangeEventHandler::
DecodingJSON()
{

    using namespace rapidjson;

    /*--
    {
        "message" : "alarm change by user",
        "msg_id"  : 1,
        "oper_no": ,
        "user_id": " ",
        "severity_id" : ,
        "prc_date: ",
        "dst_yn: ",
    }
    --*/

    try {

        rabbit::document    doc;
        doc.parse(buffer_);

        rabbit::object      body = doc["BODY"];

        stUserChangeMsg_.msg_id_       =   body["msg_id"].as_uint();
        stUserChangeMsg_.oper_no_      =   body["oper_no"].as_int();
        stUserChangeMsg_.user_id_      =   body["user_id"].as_string();
        stUserChangeMsg_.severity_id_  =   body["severity_id"].as_int();

        stUserChangeMsg_.prc_date_     =   body["prc_date"].as_string();
        stUserChangeMsg_.dst_yn_       =   body["dst_yn"].as_string();

    } catch(rabbit::type_mismatch &  e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - type mismatch [%s]",
            "UserChangeEventHandler",
            buffer_.c_str());
        return false;
    } catch(rabbit::parse_error & e) {

        gAlmLog->WARNING("%-24s| DecodingJSON - parse error [%s]",
            "UserChangeEventHandler",
            buffer_.c_str());
        return false;
    } catch(...) {
        gAlmLog->WARNING("%-24s| DecodingJSON - can't decoding [%s]",
            "UserChangeEventHandler",
            buffer_.c_str());
        return false;
    }

    return true;

}

bool UserChangeEventHandler::
IsValidSequenceId(SequenceByNode & _sequence_by_node)
{
    // 이놈은 체크 하지 않습니다.
    return true;
}

// Do() 에서 false 는 무한 시도를 뜻합니다.
bool UserChangeEventHandler::
Do(NodeManagerIF & _nm_if, DB * _db)
{
    // Request 의 Header 를 기록해 둬야 합니다.

    if(stUserChangeMsg_.IsFill() != true)
    {
        response(_nm_if, false, "invalid message");
        return true;
    }

    ST_AlarmEventMsg    stAlarmEventMsg;
    stAlarmEventMsg.Clear();

    if(makeAlarmEventMsg(stAlarmEventMsg, stUserChangeMsg_, _db) != true)
    {
        DEBUG_PRINT("can't setup alarm event msg [oper_no:%d] [user_id:%s]\n",
            stUserChangeMsg_.oper_no_, stUserChangeMsg_.user_id_.c_str());

        response(_nm_if, false, "internal error");
        return true;
    }

    alarm_h_->SetAlarmEventMsg(stAlarmEventMsg);

    // TO DO : 테스트용으로 잠시 변경 했습니다.
    // alarm_h_->Do(_nm_if, _db);
    // poll(NULL, 0, 0.5 * 1000);

    if(alarm_h_->Do(_nm_if, _db) == false)
        response(_nm_if, false, "");
    else
        response(_nm_if, true, "");

    return true;
}

bool UserChangeEventHandler::
makeAlarmEventMsg(ST_AlarmEventMsg   & _stAlarmEventMsg,
                  ST_UserChangeMsg   & _stUserChangeMsg,
                  DB                 * _db)
{
    char sql[256];

    sprintf(sql,
        "SELECT NODE_SEQ_ID, NODE_NO, NODE_NAME, PROC_NO, "
        " PKG_NAME, TYPE_CCD, GROUP_CCD, CODE, ALIAS_CODE, PROBABLE_CAUSE, "
        " LOCATION, TARGET, COMPLEMENT, VALUE, "
        " NODE_VERSION, NODE_TYPE, PRC_DATE, DST_YN "
        " FROM TAT_ALM_HIST WHERE MSG_ID=%u",
        _stUserChangeMsg.msg_id_);

    char    node_seq_id[32];
    char    node_no[16];
    char    node_name[DB_NODE_NAME_SIZE+1];
    char    proc_no[16];
    char    pkg_name[DB_PKG_TYPE_SIZE+1];
    char    type_ccd[4];
    char    group_ccd[4];
    char    code[DB_ALM_CODE_SIZE+1];
    char    alias_code[DB_ALM_ALIAS_CODE_SIZE+1];
    char    probable_cause[DB_ALM_PROBABLE_CAUSE_SIZE+1];
    char    location[DB_ALM_LOCATION_SIZE+1];
    char    target[DB_ALM_OBJECT_SIZE+1];
    char    complement[DB_ALM_COMPLEMENT_SIZE+1];
    char    value[DB_ALM_VALUE_SIZE+1];
    char    node_version[DB_ALM_NODE_VERSION_SIZE+1];
    char    node_type[DB_NODE_TYPE_SIZE+1];

	int 		ret = 0;
    FetchMaria  f;
    if((ret = _db->Query(&f, sql, strlen(sql))) <= 0)
    {
        gAlmLog->WARNING("%-24s| makeAlarmEventMsg - query fail [%d:%s] [%s]",
            "UserChangeEventHandler",
			ret,
			_db->GetErrorMsg(ret),
            sql);
        return false;
    }

    f.Clear();

    f.Set(node_seq_id, sizeof(node_seq_id));
    f.Set(node_no,     sizeof(node_no));
    f.Set(node_name,   sizeof(node_name));
    f.Set(proc_no,     sizeof(proc_no));
    f.Set(pkg_name,    sizeof(pkg_name));
    f.Set(type_ccd,    sizeof(type_ccd));
    f.Set(group_ccd,   sizeof(group_ccd));
    f.Set(code,        sizeof(code));
    f.Set(alias_code,  sizeof(alias_code));
    f.Set(probable_cause, sizeof(probable_cause));
    f.Set(location,    sizeof(location));
    f.Set(target,      sizeof(target));
    f.Set(complement,  sizeof(complement));
    f.Set(value,       sizeof(value));
    f.Set(node_version,sizeof(node_version));
    f.Set(node_type,   sizeof(node_type));

    if(f.Fetch() == false)
    {
        gAlmLog->WARNING("%-24s| can't fetch alarm message [%s]",
            "UserChangeEventHandler",
            sql);
        return false;
    }

    _stAlarmEventMsg.message_           = "user change";
    _stAlarmEventMsg.sequence_id_       = 0;
    _stAlarmEventMsg.node_no_           = atoi(node_no);
    _stAlarmEventMsg.node_name_         = node_name;
    _stAlarmEventMsg.proc_no_           = atoi(proc_no);
    // _stAlarmEventMsg.severity_id_       = _stUserChangeMsg.severity_id_;
    _stAlarmEventMsg.severity_id_       = ALM::eCLEARED;

    _stAlarmEventMsg.pkg_name_          = pkg_name;
    _stAlarmEventMsg.event_type_id_     = atoi(type_ccd);
    _stAlarmEventMsg.event_group_id_    = atoi(group_ccd);
    _stAlarmEventMsg.code_              = code;
    _stAlarmEventMsg.alias_code_        = alias_code;
    _stAlarmEventMsg.probable_cause_    = probable_cause;
    _stAlarmEventMsg.additional_text_   = "USER CLEAR";
    _stAlarmEventMsg.location_          = location;
    _stAlarmEventMsg.target_            = target;
    _stAlarmEventMsg.complement_        = complement;
    _stAlarmEventMsg.value_             = value;
    _stAlarmEventMsg.node_version_      = node_version;
    _stAlarmEventMsg.node_type_         = node_type;
    _stAlarmEventMsg.prc_date_          = _stUserChangeMsg.prc_date_;
    _stAlarmEventMsg.dst_yn_            = _stUserChangeMsg.dst_yn_;
    _stAlarmEventMsg.oper_no_           = _stUserChangeMsg.oper_no_;
    _stAlarmEventMsg.manual_yn_         = "Y";

    return true;

}

bool UserChangeEventHandler::
response(NodeManagerIF & _nm_if, bool _result, const char * _reason)
{
    rabbit::object  root;

    rabbit::object  body = root["BODY"];
    body["message"] = "alarm change by user";

    if(_result)
    {
        body["result"]  = "SUCCESS";
        body["reason"]  = "";
    }
    else
    {
        body["result"]  = "FAIL";
        body["reason"]  = _reason;
    }

    buffer_  = root.str();

    return _nm_if.Response(buffer_);
}
