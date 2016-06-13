
#include <string.h>
#include "AlarmDataDef.hpp"
#include "CFileLog.hpp"

extern CFileLog * gAlmLog;

void ST_ProtocolHeader::
Clear()
{
    version_        = 0x00;
    command_.clear();

    flag_           = 0x00;
    src_node_       = -1;
    src_proc_       = -1;
    dest_node_      = -1;
    dest_proc_      = -1;
    seq_            = -1;
    length_         = -1;
}

void ST_AlarmEventMsg::
Clear()
{
    message_.clear();
    sequence_id_    = 0;
    node_no_        = -1;
    node_name_.clear();
    proc_no_        = -1;
    severity_id_    = -1;

    pkg_name_.clear();
    event_type_id_  = 0;
    event_group_id_ = 0;
    code_.clear();
    alias_code_.clear();
    probable_cause_.clear();
    additional_text_.clear();
    location_.clear();
    target_.clear();
    complement_.clear();
    value_.clear();
    node_version_.clear();
    node_type_.clear();
    prc_date_       = "0000-00-00 00:00:00.00";
    dst_yn_         = "N";

    manual_yn_      = "N";
    vnfm_yn_        = "N";

    oper_no_        = -1;

}

ST_AlarmEventMsg & ST_AlarmEventMsg::
operator=(const ST_AlarmEventMsg & _stAlarmEventMsg)
{
    if(this != &_stAlarmEventMsg)
    {
        message_        = _stAlarmEventMsg.message_;
        sequence_id_    = _stAlarmEventMsg.sequence_id_;
        node_no_        = _stAlarmEventMsg.node_no_;
        node_name_      = _stAlarmEventMsg.node_name_;
        proc_no_        = _stAlarmEventMsg.proc_no_;
        severity_id_    = _stAlarmEventMsg.severity_id_;

        pkg_name_       = _stAlarmEventMsg.pkg_name_;
        event_type_id_  = _stAlarmEventMsg.event_type_id_;
        event_group_id_ = _stAlarmEventMsg.event_group_id_;
        code_           = _stAlarmEventMsg.code_;
        alias_code_     = _stAlarmEventMsg.alias_code_;
        probable_cause_ = _stAlarmEventMsg.probable_cause_;
        additional_text_= _stAlarmEventMsg.additional_text_;
        location_       = _stAlarmEventMsg.location_;
        target_         = _stAlarmEventMsg.target_;
        complement_     = _stAlarmEventMsg.complement_;
        value_          = _stAlarmEventMsg.value_;
        node_version_   = _stAlarmEventMsg.node_version_;
        node_type_      = _stAlarmEventMsg.node_type_;
        prc_date_       = _stAlarmEventMsg.prc_date_;
        dst_yn_         = _stAlarmEventMsg.dst_yn_;

        manual_yn_      = _stAlarmEventMsg.manual_yn_;
        vnfm_yn_        = _stAlarmEventMsg.vnfm_yn_;

        oper_no_        = _stAlarmEventMsg.oper_no_;

    }

    return *this;
}

void ST_AlarmEventMsg::
SetFromAlarmStatus(ST_AlarmStatus & _stAlarmStatus)
{
    // sequence_id_
    node_no_        = _stAlarmStatus.node_no_;
    node_name_      = _stAlarmStatus.node_name_;
    proc_no_        = _stAlarmStatus.proc_no_;
    severity_id_    = _stAlarmStatus.severity_id_;

    pkg_name_       = _stAlarmStatus.pkg_name_;
    event_type_id_  = _stAlarmStatus.event_type_id_;
    event_group_id_ = _stAlarmStatus.event_group_id_;
    code_           = _stAlarmStatus.code_;
    alias_code_     = _stAlarmStatus.alias_code_;
    probable_cause_ = _stAlarmStatus.probable_cause_;
    additional_text_= _stAlarmStatus.additional_text_;
    location_       = _stAlarmStatus.location_;
    target_         = _stAlarmStatus.target_;
    complement_     = _stAlarmStatus.complement_;
    value_          = _stAlarmStatus.value_;
    node_version_   = _stAlarmStatus.node_version_;
    node_type_      = _stAlarmStatus.node_type_;
    prc_date_       = _stAlarmStatus.prc_date_;
    dst_yn_         = _stAlarmStatus.dst_yn_;

    manual_yn_      = _stAlarmStatus.manual_yn_;
    oper_no_        = _stAlarmStatus.oper_no_;
}

void ST_AlarmStatus::
Clear()
{
    msg_id_             = 0;

    node_no_            = -1;
    node_name_.clear();
    proc_no_            = -1;
    severity_id_        = -1;

    pkg_name_.clear();
    code_.clear();
    event_type_id_      = 0;
    event_group_id_     = 0;
    alias_code_.clear();
    probable_cause_.clear();
    additional_text_.clear();
    location_.clear();
    target_.clear();
    complement_.clear();
    value_.clear();
    node_version_.clear();
    node_type_.clear();
    prc_date_           = "0000-00-00 00:00:00.00";
    lst_date_           = "0000-00-00 00:00:00.00";
    confirm_yn_         = "N";

    manual_yn_          = "N";
    oper_no_            = -1;
    dst_yn_             = "N";

    associated_msg_id_  = 0;
}

ST_AlarmStatus & ST_AlarmStatus::
operator=(const ST_AlarmStatus & _stAlarmStatus)
{
    if(this != &_stAlarmStatus)
    {
        msg_id_         = _stAlarmStatus.msg_id_;

        node_no_        = _stAlarmStatus.node_no_;
        node_name_      = _stAlarmStatus.node_name_;
        proc_no_        = _stAlarmStatus.proc_no_;
        severity_id_    = _stAlarmStatus.severity_id_;

        pkg_name_       = _stAlarmStatus.pkg_name_;
        event_type_id_  = _stAlarmStatus.event_type_id_;
        event_group_id_ = _stAlarmStatus.event_group_id_;
        code_           = _stAlarmStatus.code_;
        alias_code_     = _stAlarmStatus.alias_code_;
        probable_cause_ = _stAlarmStatus.probable_cause_;
        additional_text_= _stAlarmStatus.additional_text_;
        location_       = _stAlarmStatus.location_;
        target_         = _stAlarmStatus.target_;
        complement_     = _stAlarmStatus.complement_;
        value_          = _stAlarmStatus.value_;
        node_version_   = _stAlarmStatus.node_version_;
        node_type_      = _stAlarmStatus.node_type_;
        prc_date_       = _stAlarmStatus.prc_date_;
        lst_date_       = _stAlarmStatus.lst_date_;
        confirm_yn_     = _stAlarmStatus.confirm_yn_;

        manual_yn_      = _stAlarmStatus.manual_yn_;
        oper_no_        = _stAlarmStatus.oper_no_;
        dst_yn_         = _stAlarmStatus.dst_yn_;

        associated_msg_id_ = _stAlarmStatus.associated_msg_id_;
    }

    return *this;
}

void ST_AlarmStatus::
SetFromAlarmEventMsg(ST_AlarmEventMsg & _stAlarmEventMsg)
{
    node_no_        = _stAlarmEventMsg.node_no_;
    node_name_      = _stAlarmEventMsg.node_name_;
    proc_no_        = _stAlarmEventMsg.proc_no_;
    severity_id_    = _stAlarmEventMsg.severity_id_;

    pkg_name_       = _stAlarmEventMsg.pkg_name_;
    event_type_id_  = _stAlarmEventMsg.event_type_id_;
    event_group_id_ = _stAlarmEventMsg.event_group_id_;
    code_           = _stAlarmEventMsg.code_;
    alias_code_     = _stAlarmEventMsg.alias_code_;
    probable_cause_ = _stAlarmEventMsg.probable_cause_;
    additional_text_= _stAlarmEventMsg.additional_text_;
    location_       = _stAlarmEventMsg.location_;
    target_         = _stAlarmEventMsg.target_;
    complement_     = _stAlarmEventMsg.complement_;
    value_          = _stAlarmEventMsg.value_;
    node_version_   = _stAlarmEventMsg.node_version_;
    node_type_      = _stAlarmEventMsg.node_type_;
    prc_date_       = _stAlarmEventMsg.prc_date_;
    dst_yn_         = _stAlarmEventMsg.dst_yn_;
    lst_date_       = _stAlarmEventMsg.prc_date_;

    manual_yn_      = _stAlarmEventMsg.manual_yn_;
    oper_no_        = _stAlarmEventMsg.oper_no_;

}

bool ST_AlarmStatus::
Insert(DB * _db)
{
    if(_db == NULL)
    {
        DEBUG_PRINT("Insert - DB is null\n");
        return false;
    }

    char sql[1024];
    sprintf(sql,
        "INSERT INTO TAT_ALM_STATUS "
        " (MSG_ID, NODE_NO, NODE_NAME, PROC_NO,"
        " SEVERITY_CCD, PKG_NAME, TYPE_CCD, GROUP_CCD,"
        " CODE, ALIAS_CODE, PROBABLE_CAUSE, ADDITIONAL_TEXT,"
        " LOCATION, TARGET, COMPLEMENT, VALUE, "
        " PRC_DATE, LST_DATE, CONFIRM_YN, OPER_NO) "
        " VALUES "
        " (%u, %d,'%s',%d,"
        "  %d,'%s', %d, %d,"
        " '%s','%s','%s','%s',"
        " '%s','%s','%s','%s',"
        " '%s','%s','%s',%d)",
        msg_id_,
        node_no_,
        node_name_.c_str(),
        proc_no_,

        severity_id_,
        pkg_name_.c_str(),
        event_type_id_,
        event_group_id_,

        code_.c_str(),
        alias_code_.c_str(),
        probable_cause_.c_str(),
        additional_text_.c_str(),

        location_.c_str(),
        target_.c_str(),
        complement_.c_str(),
        value_.c_str(),

        prc_date_.c_str(),
        lst_date_.c_str(),
        confirm_yn_.c_str(),
        oper_no_);

    if(_db->Execute(sql, strlen(sql)) <= 0)
    {
        gAlmLog->WARNING("%-24s| Insert - fail [%d:%s] [%s]",
            "ST_AlarmStatus",
            _db->GetError(),
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    gAlmLog->DEBUG("INSERT SUCCESS [%s]\n", sql);

    return true;
}

bool ST_AlarmStatus::
Delete(DB * _db)
{
    return ST_AlarmStatus::Delete(_db, msg_id_);
}

bool ST_AlarmStatus::
Delete(DB * _db,    unsigned int _msg_id)
{
    char sql[256];
    sprintf(sql,
        "DELETE FROM TAT_ALM_STATUS WHERE MSG_ID=%u", _msg_id);

    if(_db->Execute(sql, strlen(sql)) <= 0)
    {
        gAlmLog->WARNING("%-24s| Delete - fail [%d:%s] [%s]",
            "ST_AlarmStatus",
            _db->GetError(),
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    return true;
}

bool ST_AlarmStatus::
CompareSeverity(int _severity_id)
{
    return (severity_id_ == _severity_id)?true:false;
}

bool ST_AlarmStatus::
IsCleared()
{
    return (severity_id_ == ALM::eCLEARED)?true:false;
}

void ST_AlarmHistory::
Clear()
{
    stAlarmStatus_.Clear();

    cleared_date_ = "0000-00-00 00:00:00.00";
    elapsed_time_ = "0000-00-00 00:00:00.00";

    snmp_seq_id_    = 0;
    node_seq_id_    = 0;
    cleared_msg_id_ = 0;

    cleared_yn_     = "N";
    snmp_send_yn_   = "N";
    sms_send_yn_    = "N";
    email_send_yn_  = "N";
}

bool ST_AlarmHistory::
Insert(DB * _db)
{
    char sql[1024];
    sprintf(sql,
        "INSERT INTO TAT_ALM_HIST"
        " (MSG_ID, NODE_NO, NODE_NAME, PROC_NO, "
        " SEVERITY_CCD, PKG_NAME, TYPE_CCD, GROUP_CCD,"
        " CODE, ALIAS_CODE, PROBABLE_CAUSE, ADDITIONAL_TEXT, "
        " LOCATION, TARGET, COMPLEMENT, VALUE, "
        " NODE_VERSION, NODE_TYPE, "
        " PRC_DATE, CLEARED_DATE, ELAPSED_TIME, "
        " SNMP_SEQ_ID, NODE_SEQ_ID, CLEARED_MSG_ID, ASSOCIATED_MSG_ID, "
        " OPER_NO, "
        " CLEARED_YN, MANUAL_YN, SNMP_SEND_YN, SMS_SEND_YN, EMAIL_SEND_YN, DST_YN) "
        " VALUES "
        " (%u, %d, '%s', %d,"
        "  %d, '%s', %d, %d,"
        " '%s', '%s', '%s', '%s',"
        " '%s', '%s', '%s', '%s',"
        " '%s', '%s',"
        " '%s', '%s', '%s',"
        "  %u, %u, %u, %u,"
        "  %d,"
        " '%s', '%s', '%s', '%s', '%s', '%s')",
        stAlarmStatus_.msg_id_,
        stAlarmStatus_.node_no_,
        stAlarmStatus_.node_name_.c_str(),
        stAlarmStatus_.proc_no_,

        stAlarmStatus_.severity_id_,
        stAlarmStatus_.pkg_name_.c_str(),
        stAlarmStatus_.event_type_id_,
        stAlarmStatus_.event_group_id_,

        stAlarmStatus_.code_.c_str(),
        stAlarmStatus_.alias_code_.c_str(),
        stAlarmStatus_.probable_cause_.c_str(),
        stAlarmStatus_.additional_text_.c_str(),

        stAlarmStatus_.location_.c_str(),
        stAlarmStatus_.target_.c_str(),
        stAlarmStatus_.complement_.c_str(),
        stAlarmStatus_.value_.c_str(),

        stAlarmStatus_.node_version_.c_str(),
        stAlarmStatus_.node_type_.c_str(),

        stAlarmStatus_.prc_date_.c_str(),
        cleared_date_.c_str(),
        elapsed_time_.c_str(),

        snmp_seq_id_,
        node_seq_id_,
        cleared_msg_id_,
        stAlarmStatus_.associated_msg_id_,

        stAlarmStatus_.oper_no_,

        cleared_yn_.c_str(),
        stAlarmStatus_.manual_yn_.c_str(),
        snmp_send_yn_.c_str(),
        sms_send_yn_.c_str(),
        email_send_yn_.c_str(),
        stAlarmStatus_.dst_yn_.c_str());

    if(_db->Execute(sql, strlen(sql)) < 0)
    {
        gAlmLog->WARNING("%-24s| Insert - fail [%d:%s] [%s]\n",
            "ST_AlarmStatus",
            _db->GetError(),
            _db->GetErrorMsg(),
            sql);
        return false;
    }

    gAlmLog->DEBUG("INSERT SUCCESS [%s]\n", sql);

    return true;
}

bool ST_Subscriber::
Compare(ST_Subscriber & _stSubscriber)
{
    return (node_no_ == _stSubscriber.node_no_ &&
            proc_no_ == _stSubscriber.proc_no_ &&
            code_.compare(_stSubscriber.code_) == 0 &&
            pkg_name_.compare(_stSubscriber.pkg_name_) == 0)?true:false;
}

void ST_UserChangeMsg::
Clear()
{
    msg_id_         = 0;
    oper_no_        = -1;
    user_id_.clear();
    severity_id_    = -1;
    prc_date_.clear();
    dst_yn_.clear();
}

bool ST_UserChangeMsg::
IsFill()
{
    return (user_id_.size() != 0 &&
            prc_date_.size() != 0)?true:false;
}

void ST_ProcStatusMsg::
Clear()
{

    sequence_id_    = 0;
    node_no_        = -1;

    worst_status_.clear();
    proc_no_        = -1;
    proc_name_.clear();
    proc_status_.clear();

    prc_date_.clear();
    dst_yn_        = "N";
}

void ST_ConnectStatusMsg::
Clear()
{
    sequence_id_    = 0;
    node_no_        = -1;

    my_ip_.clear();
    peer_ip_.clear();
    server_port_    = 0;

    status_.clear();
    prc_date_.clear();
    dst_yn_        = "N";
}


void ST_PingStatusMsg::
Clear()
{
    sequence_id_    = 0;
    node_no_        = -1;
    peer_no_        = -1;

    my_ip_.clear();
    peer_ip_.clear();

    status_.clear();
    prc_date_.clear();
    dst_yn_        = "N";
}

void ST_VnfmSendDef::
Clear()
{
    code_.clear();
    severity_id_        = 0;
    pkg_name_.clear();
    alias_code_.clear();
    alias_severity_id_  = 0;
    alias_probable_cause_.clear();
    reboot_yn_.clear();
}