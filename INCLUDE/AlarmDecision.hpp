
#ifndef __ALARM_DECISION_HPP__
#define __ALARM_DECISION_HPP__

#include <string>

#include "DB.hpp"
#include "AlarmDataDef.hpp"
#include "AlarmCodeConf.hpp"
#include "AlarmLevelConf.hpp"

class AlarmDecision
{
public:
    AlarmDecision();
    ~AlarmDecision();

    bool    Init(DB * _db,  ST_Profile & _profile);
    bool    Init(DB * _db,  std::string & _pkg_name, std::string & _node_type);
    bool    EqualPkgNameAndNodeType(std::string & _pkg_name,
                                    std::string & _node_type);
    bool    Do(ST_AlarmEventMsg & _stAlarmEventMsg);
    bool    Do(ST_AlarmStatus   & _stAlarmStatus);

private:

    std::string     pkg_name_;
    std::string     node_type_;

    AlarmCodeConf   code_conf_;
    AlarmLevelConf  level_conf_;
};


#endif // __ALARM_DECISION_HPP__
