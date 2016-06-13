
#include "AlarmDecision.hpp"
#include "CommonCode.hpp"

#include "CFileLog.hpp"

extern CFileLog *   gAlmLog;

AlarmDecision::
AlarmDecision()
{


}

AlarmDecision::
~AlarmDecision()
{

    // Empty
}

bool AlarmDecision::
Init(DBConfig * _dbConfig, ST_Profile & _profile)
{
    return Init(_dbConfig, _profile.m_strPkgName, _profile.m_strNodeType);
}

bool AlarmDecision::
Init(DBConfig * _dbConfig, std::string & _pkg_name, std::string & _node_type)
{
    if(code_conf_.SetAlarmCode(_dbConfig,
                               _pkg_name.c_str()) != true)
    {
        gAlmLog->ERROR("%-24s| Init - alarm code conf read fail [%s]",
            "AlarmDecision",
            _pkg_name.c_str());
        return false;
    }

    if(level_conf_.SetAlarmLevel(_dbConfig,
                                 _pkg_name.c_str(),
                                 _node_type.c_str()) != true)
    {
        gAlmLog->ERROR("%-24s| Init - alarm level conf read fail [%s] [%s]",
            "AlarmDecision",
            _pkg_name.c_str(),
            _node_type.c_str());
        return false;
    }

    pkg_name_   = _pkg_name;
    node_type_  = _node_type;

    return true;
}

bool AlarmDecision::
EqualPkgNameAndNodeType(std::string & _pkg_name, std::string & _node_type)
{
    if(pkg_name_.size()  > 0 &&
       pkg_name_.compare(_pkg_name) != 0)
        return false;

    if(node_type_.size() > 0 &&
       node_type_.compare(_node_type) != 0)
        return false;

    return true;
}


bool AlarmDecision::
Do(ST_AlarmEventMsg & _stAlarmEventMsg)
{
    ST_AlarmCodeDef * def = code_conf_.GetAlarmCodeDef(_stAlarmEventMsg.code_);

    if(def == NULL)
    {
        gAlmLog->INFO("%-24s| Do - Unknown defined Alarm code [%s]",
            "AlarmDecision",
            _stAlarmEventMsg.code_.c_str());
        return false;
    }

    _stAlarmEventMsg.event_type_id_ = def->event_type_id_;
    _stAlarmEventMsg.event_group_id_= def->event_group_id_;
    _stAlarmEventMsg.alias_code_    = def->alias_code_;
    _stAlarmEventMsg.probable_cause_= def->probable_cause_;


    if(level_conf_.IsExist(_stAlarmEventMsg.node_type_,
                            _stAlarmEventMsg.code_,
                            _stAlarmEventMsg.target_) == true)
    {
        _stAlarmEventMsg.severity_id_ =
            level_conf_.GetSeverity(_stAlarmEventMsg.node_type_,
                                    _stAlarmEventMsg.code_,
                                    _stAlarmEventMsg.target_,
                                    _stAlarmEventMsg.value_,
                                    _stAlarmEventMsg.complement_);
        gAlmLog->DEBUG("%-24s| Do - Code[%s] Value[%s] Severity[%d]",
            "AlarmDecision",
            _stAlarmEventMsg.code_.c_str(),
            _stAlarmEventMsg.value_.c_str(),
            _stAlarmEventMsg.severity_id_);
        return true;
    }

    _stAlarmEventMsg.severity_id_ =
        code_conf_.GetSeverity(_stAlarmEventMsg.code_);

    return true;
}

bool AlarmDecision::
Do(ST_AlarmStatus & _stAlarmStatus)
{

    ST_AlarmCodeDef * def = code_conf_.GetAlarmCodeDef(_stAlarmStatus.code_);

    if(def == NULL)
    {
        gAlmLog->INFO("%-24s| Do - Unknown defined Alarm code [%s]",
            "AlarmDecision",
            _stAlarmStatus.code_.c_str());
        return false;
    }

    _stAlarmStatus.event_type_id_ = def->event_type_id_;
    _stAlarmStatus.event_group_id_= def->event_group_id_;
    _stAlarmStatus.alias_code_    = def->alias_code_;
    _stAlarmStatus.probable_cause_= def->probable_cause_;


    if(level_conf_.IsExist(_stAlarmStatus.node_type_,
                            _stAlarmStatus.code_,
                            _stAlarmStatus.target_) == true)
    {
        _stAlarmStatus.severity_id_ =
            level_conf_.GetSeverity(_stAlarmStatus.node_type_,
                                    _stAlarmStatus.code_,
                                    _stAlarmStatus.target_,
                                    _stAlarmStatus.value_,
                                    _stAlarmStatus.complement_);
        gAlmLog->DEBUG("%-24s| Do - Code[%s] Value[%s] Severity[%d]",
            "AlarmDecision",
            _stAlarmStatus.code_.c_str(),
            _stAlarmStatus.value_.c_str(),
            _stAlarmStatus.severity_id_);
        return true;
    }

    gAlmLog->DEBUG("%-24s| ------ Do - level conf is not exist node type [%s] code [%s] target [%s]",
            "AlarmDecision",
            _stAlarmStatus.node_type_.c_str(),
            _stAlarmStatus.code_.c_str(),
            _stAlarmStatus.target_.c_str());


    _stAlarmStatus.severity_id_ =
        code_conf_.GetSeverity(_stAlarmStatus.code_);

    return true;
}