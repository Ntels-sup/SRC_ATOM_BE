
#include <algorithm>

#include "VnfmAlarm.hpp"
#include "FetchMaria.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rabbit.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

VnfmAlarm::
VnfmAlarm()
{
    // Empty
}

VnfmAlarm::
~VnfmAlarm()
{
    // Empty
}

bool VnfmAlarm::
Init(DB * _db, const char * _pkg_name)
{
    if(setMapVnfmSendConf(_db, _pkg_name) == false)
        return false;

    if(setAlarmGroupString(_db) == false)
        return false;

    if(setAlarmSeverityString(_db) == false)
        return false;

    return true;
}

bool VnfmAlarm::
setMapVnfmSendConf(DB * _db, const char * _pkg_name)
{
    map_vnfm_send_conf_.clear();

    char sql[256];
    sprintf(sql, "SELECT CODE, "
                   " SEVERITY_CCD, "
                   " PKG_NAME, "
                   " ALIAS_CODE, "
                   " ALIAS_SEVERITY_CCD, "
                   " ALIAS_PROBABLE_CAUSE, "
                   " REBOOT_YN "
                   " FROM TAT_VNF_SEND_DEF "
                   " WHERE USE_YN='Y' AND PKG_NAME='%s'",
                   _pkg_name);

	int 			ret = 0;
    FetchMaria      f;

    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->INFO("%-24s| setMapVnfmSendConf - query fail [%s]",
            "VnfmAlarm",
			_db->GetErrorMsg(ret));
        return false;
    }

    char    code[DB_ALM_CODE_SIZE+1];
    char    severity_id[4];
    char    pkg_name[DB_PKG_NAME_SIZE+1];
    char    alias_code[DB_ALM_ALIAS_CODE_SIZE+1];
    char    alias_severity_id[4];
    char    alias_probable_cause[DB_ALM_PROBABLE_CAUSE_SIZE+1];
    char    reboot_yn[4];

    f.Set(code,                 sizeof(code));
    f.Set(severity_id,          sizeof(severity_id));
    f.Set(pkg_name,             sizeof(pkg_name));
    f.Set(alias_code,           sizeof(alias_code));
    f.Set(alias_severity_id,    sizeof(alias_severity_id));
    f.Set(alias_probable_cause, sizeof(alias_probable_cause));
    f.Set(reboot_yn,            sizeof(reboot_yn));

    while(f.Fetch())
    {
        ST_VnfmSendDef  def;

        def.Clear();
        def.code_               = code;
        def.severity_id_        = atoi(severity_id);
        def.pkg_name_           = pkg_name;
        def.alias_code_         = alias_code;
        def.alias_severity_id_  = atoi(severity_id);
        def.alias_probable_cause_ = alias_probable_cause;
        def.reboot_yn_          = reboot_yn;

        map_vnfm_send_conf_[def.code_] = def;
    }

    gAlmLog->DEBUG("%-24s| setMapVnfmSendConf - SUCCESS size[%d]",
        "VnfmAlarm",
        map_vnfm_send_conf_.size());

    return true;
}

bool VnfmAlarm::
setAlarmGroupString(DB * _db)
{
    map_group_ccd_.clear();

    char    sql[128];
    sprintf(sql,
        "SELECT DETAIL_CD, CODE_NAME FROM TAT_COMMON_CODE WHERE GROUP_CD='030100'");

	int             ret = 0;
    FetchMaria      f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->INFO("%-24s| setAlarmGroupString - query fail [%s]",
            "VnfmAlarm",
			_db->GetErrorMsg(ret));
        return false;
    }

    char    detail_cd[16];
    char    code_name[32];

    f.Set(detail_cd,        sizeof(detail_cd));
    f.Set(code_name,        sizeof(code_name));

    while(f.Fetch())
    {
        map_group_ccd_[atoi(detail_cd)] = code_name;
    }

    gAlmLog->DEBUG("%-24s| setAlarmGroupString - SUCCESS size[%d]",
        "VnfmAlarm",
        map_group_ccd_.size());

    return true;

}

bool VnfmAlarm::
setAlarmSeverityString(DB * _db)
{
    map_severity_ccd_.clear();

    char    sql[128];
    sprintf(sql,
        "SELECT DETAIL_CD, CODE_NAME FROM TAT_COMMON_CODE WHERE GROUP_CD='030101'");

	int 			ret = 0;
    FetchMaria      f;
    if((ret = _db->Query(&f, sql, strlen(sql))) < 0)
    {
        gAlmLog->INFO("%-24s| setAlarmSeverityString - query fail",
            "VnfmAlarm");
        return false;
    }

    char    detail_cd[16];
    char    code_name[32];

    f.Set(detail_cd,        sizeof(detail_cd));
    f.Set(code_name,        sizeof(code_name));

    while(f.Fetch())
    {
        map_severity_ccd_[atoi(detail_cd)] = code_name;
    }

    gAlmLog->DEBUG("%-24s| setAlarmSeveritytring - SUCCESS size[%d]",
        "VnfmAlarm",
        map_severity_ccd_.size());

    return true;

}

bool VnfmAlarm::
MakeBody(std::string        & _out,
         ST_AlarmEventMsg   & _stAlarmEventMsg)
{

    auto iter = map_vnfm_send_conf_.find(_stAlarmEventMsg.code_);

    if(iter == map_vnfm_send_conf_.end())
        return false;


    std::string     code;
    int             severity_id = 0;
    std::string     probable_cause;

    ST_VnfmSendDef * def = getVnfmSendDef(_stAlarmEventMsg.code_);
    if(def != NULL)
    {
        code            = def->alias_code_;
        severity_id     = def->alias_severity_id_;
        probable_cause  = def->alias_probable_cause_;
    }
    else
    {
        code            = _stAlarmEventMsg.code_;
        severity_id     = _stAlarmEventMsg.severity_id_;
        probable_cause  = _stAlarmEventMsg.probable_cause_;
    }

    std::string         group_name;
    GetAlarmGroupName(group_name, _stAlarmEventMsg.event_group_id_);
    std::string         severity_name;
    GetAlarmSeverityName(severity_name, severity_id);

    rabbit::object      doc;
    rabbit::object      body = doc["BODY"];

    body["event_type"]          = group_name;
    body["prc_date"]            = _stAlarmEventMsg.prc_date_;
    body["code"]                = code;
    body["perceived_severity"]  = severity_name;
    body["probable_cause"]      = probable_cause;
    body["additional_text"]     = _stAlarmEventMsg.probable_cause_;

    if(IsRebootY(_stAlarmEventMsg.code_))
    {
        body["coresponse_action"]     = "Reboot";
        body["coresponse_action_cnt"] = 2;
    }

    _out = doc.str();

    return true;
}

void VnfmAlarm::
GetAlarmGroupName(std::string & _out, int _event_group_id)
{
    auto iter = map_group_ccd_.find(_event_group_id);

    if(iter == map_group_ccd_.end())
        _out = "unknown";
    else
        _out = iter->second;
}

void VnfmAlarm::
GetAlarmSeverityName(std::string & _out, int _severity_id)
{
    auto iter = map_severity_ccd_.find(_severity_id);

    if(iter == map_severity_ccd_.end())
        _out = "Unknown";
    else
    {
        _out = iter->second;
        std::transform(_out.begin()+1, _out.end(), _out.begin()+1, ::tolower);
    }
}

bool VnfmAlarm::
IsRebootY(std::string & _code)
{
    ST_VnfmSendDef * def = getVnfmSendDef(_code);

    if(def == NULL)
        return false;

    return (def->reboot_yn_ == "Y") ? true : false;
}

ST_VnfmSendDef * VnfmAlarm::
getVnfmSendDef(std::string _code)
{
    auto iter = map_vnfm_send_conf_.find(_code);

    if(iter == map_vnfm_send_conf_.end())
        return NULL;

    return &(iter->second);
}
