
#include <string.h>

#include "AlarmCodeConf.hpp"
#include "DebugMacro.hpp"

#include "CFileLog.hpp"

extern CFileLog * gAlmLog;

void ST_AlarmCodeDef::
Clear()
{
    pkg_name_.clear();
    code_.clear();
    event_type_id_  = 0;
    event_group_id_ = 0;
    alias_code_.clear();
    probable_cause_.clear();
    severity_id_    = 0;
    sms_yn_         = "N";
    email_yn_       = "N";
    snmp_yn_        = "N";
}

ST_AlarmCodeDef & ST_AlarmCodeDef::
operator=(const ST_AlarmCodeDef & _st)
{
    if(this != &_st)
    {
        pkg_name_       = _st.pkg_name_;
        code_           = _st.code_;
        event_type_id_  = _st.event_type_id_;
        event_group_id_ = _st.event_group_id_;
        alias_code_     = _st.alias_code_;
        probable_cause_ = _st.probable_cause_;
        severity_id_    = _st.severity_id_;
        sms_yn_         = _st.sms_yn_;
        email_yn_       = _st.email_yn_;
        snmp_yn_        = _st.snmp_yn_;
    }

    return *this;
}


AlarmCodeConf::
AlarmCodeConf()
{
    map_.clear();
    empty_.clear();
}

AlarmCodeConf::
~AlarmCodeConf()
{
    // Empty
}

AlarmCodeConf & AlarmCodeConf::
operator=(const AlarmCodeConf & _alarm_code_conf)
{
    if(this != &_alarm_code_conf)
    {
        map_    = _alarm_code_conf.map_;
        empty_  = "";

        // 다른 건, 초기화가 필요 없어요.
    }

    return *this;
}

bool AlarmCodeConf::
SetAlarmCode(DBConfig       * _dbConfig,
            std::string     & _code,
            const char      * _pkg_name)
{
    FetchMaria      f;
    setFetchData(f);

    if(queryAlarmCode(_dbConfig, f, _code.c_str(), _pkg_name) == false)
    {
        gAlmLog->WARNING("%-24s| SetAlarmCode - queryAlarmCode fail",
            "AlarmCodeConf");
        return false;
    }

    if(f.Fetch() == false)
    {
        gAlmLog->WARNING("%-24s| SetAlarmCode - select no data code [%s]",
            "AlarmCodeConf",
            _code.c_str());

        return false;
    }

    setAlarmCodeDef();

    return true;
}

void AlarmCodeConf::
setFetchData(FetchMaria & _f)
{
    _f.Clear();

    _f.Set(pkg_name_,         sizeof(pkg_name_));
    _f.Set(code_,             sizeof(code_));
    _f.Set(event_type_id_,    sizeof(event_type_id_));
    _f.Set(event_group_id_,   sizeof(event_group_id_));
    _f.Set(alias_code_,       sizeof(alias_code_));
    _f.Set(probable_cause_,   sizeof(probable_cause_));
    _f.Set(severity_id_,      sizeof(severity_id_));
    _f.Set(sms_yn_,           sizeof(sms_yn_));
    _f.Set(email_yn_,         sizeof(email_yn_));
    _f.Set(snmp_yn_,          sizeof(snmp_yn_));
}

bool AlarmCodeConf::
SetAlarmCode(DBConfig   *    _dbConfig,
             const char *    _pkg_name)
{
    FetchMaria      f;

    setFetchData(f);

    if(queryAlarmCode(_dbConfig, f, "", _pkg_name) == false)
    {
        gAlmLog->WARNING("%-24s| SetAlarmCode - select fail",
            "AlarmCodeConf");
        return false;
    }

    while(f.Fetch())
    {
        setAlarmCodeDef();
    }

    return true;
}

bool AlarmCodeConf::
queryAlarmCode(DBConfig   * _dbConfig,
               FetchMaria & _f,
               const char *_code,
               const char * _pkg_name)
{
    int  len = 0;
    char sql[256];

    len =
    sprintf(sql,
        "SELECT PKG_NAME, CODE, TYPE_CCD, GROUP_CCD, ALIAS_CODE, "
        " PROBABLE_CAUSE, SEVERITY_CCD, "
        " SMS_YN, EMAIL_YN, SNMP_YN "
        " FROM TAT_ALM_CODE_DEF "
        " WHERE ALARM_YN='Y' ");

    char * p = sql + len;

    if(_code[0] != '\0')
    {
        len = sprintf(p, " AND CODE='%s' ", _code);
        p += len;
    }

    if(_pkg_name[0] != '\0')
    {
        len = sprintf(p, " AND (PKG_NAME='ATOM' OR PKG_NAME='%s') ", _pkg_name);
        p += len;
    }

    int eNo = 0;
    if(_dbConfig->Get(_f, eNo, sql) == false)
    {
        gAlmLog->ERROR("%-24s| queryAlarmCode - get fail [%d:%s] [%s]",
            "AlarmCodeConf",
            eNo,
            _dbConfig->GetErrorMsg(eNo),
            sql);
        return false;
    }

    return true;
}

void AlarmCodeConf::
setAlarmCodeDef()
{
    ST_AlarmCodeDef     st;
    st.Clear();

    st.pkg_name_       = pkg_name_;
    st.code_           = code_;
    st.event_type_id_  = atoi(event_type_id_);
    st.event_group_id_ = atoi(event_group_id_);
    st.alias_code_     = alias_code_;
    st.probable_cause_ = probable_cause_;
    st.severity_id_    = atoi(severity_id_);
    st.sms_yn_         = sms_yn_;
    st.email_yn_       = email_yn_;
    st.snmp_yn_        = snmp_yn_;

    map_[st.code_]     = st;

    gAlmLog->INFO("%-24s| P[%s], CODE[%s] T[%d] G[%d] A-CODE[%s] CAUSE[%.16s] SEVER[%d] SMS[%s] EMAIL[%s] SNMP[%s]",
        "AlarmCodeConf",
        st.pkg_name_.c_str(),
        st.code_.c_str(),
        st.event_type_id_,
        st.event_group_id_,
        st.alias_code_.c_str(),
        st.probable_cause_.c_str(),
        st.severity_id_,
        st.sms_yn_.c_str(),
        st.email_yn_.c_str(),
        st.snmp_yn_.c_str());
}


bool AlarmCodeConf::
IsExist(std::string & _code)
{
    auto iter = map_.find(_code);
    return (iter != map_.end())?true:false;
}

ST_AlarmCodeDef * AlarmCodeConf::
GetAlarmCodeDef(std::string & _code)
{
    auto iter = map_.find(_code);

    if(iter == map_.end())
        return NULL;

    return &(iter->second);
}

std::string & AlarmCodeConf::
GetAliasCode(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return _code;

    return def->alias_code_;
}

std::string & AlarmCodeConf::
GetProbableCause(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return empty_;

    return def->probable_cause_;
}

int AlarmCodeConf::
GetSeverity(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return ALM::eINDETERMINATE;

    return def->severity_id_;
}

bool AlarmCodeConf::
IsSMS(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return false;

    return (def->sms_yn_ == "Y")?true:false;
}

bool AlarmCodeConf::
IsEmail(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return false;

    return (def->email_yn_ == "Y")?true:false;
}

bool AlarmCodeConf::
IsSNMP(std::string & _code)
{
    ST_AlarmCodeDef * def = GetAlarmCodeDef(_code);

    if(def == NULL)
        return false;

    return (def->snmp_yn_ == "Y")?true:false;
}