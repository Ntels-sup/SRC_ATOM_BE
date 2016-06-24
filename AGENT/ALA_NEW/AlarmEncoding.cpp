#include "AlarmEncoding.hpp"

#include "AlarmDataDef.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

void AlarmEncoding::
MakeBody(std::string      & _body,
         ST_AlarmEventMsg & _stAlarmEventMsg)
{
        // JSON
    rabbit::object      root;

    rabbit::object      body = root["BODY"];

    body["message"]         =   _stAlarmEventMsg.message_;
    body["node_seq_id"]     =   _stAlarmEventMsg.sequence_id_;

    body["node_no"]         =   _stAlarmEventMsg.node_no_;
    body["node_name"]       =   _stAlarmEventMsg.node_name_;
    body["proc_no"]         =   _stAlarmEventMsg.proc_no_;
    body["severity_id"]     =   _stAlarmEventMsg.severity_id_;

    body["pkg_name"]        =   _stAlarmEventMsg.pkg_name_;
    body["event_type_id"]   =   _stAlarmEventMsg.event_type_id_;
    body["event_group_id"]  =   _stAlarmEventMsg.event_group_id_;
    body["code"]            =   _stAlarmEventMsg.code_;
    body["alias_code"]      =   _stAlarmEventMsg.alias_code_;
    body["probable_cause"]  =   _stAlarmEventMsg.probable_cause_;
    body["additional_text"] =   _stAlarmEventMsg.additional_text_;
    body["location"]        =   _stAlarmEventMsg.location_;
    body["target"]          =   _stAlarmEventMsg.target_;
    body["complement"]      =   _stAlarmEventMsg.complement_;
    body["value"]           =   _stAlarmEventMsg.value_;
    body["node_version"]    =   _stAlarmEventMsg.node_version_;
    body["node_type"]       =   _stAlarmEventMsg.node_type_;
    body["prc_date"]        =   _stAlarmEventMsg.prc_date_;
    body["dst_yn"]          =   _stAlarmEventMsg.dst_yn_;
    body["vnfm_yn"]         =   _stAlarmEventMsg.vnfm_yn_;

    _body.assign(root.str());
}
