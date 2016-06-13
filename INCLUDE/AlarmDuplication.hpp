#ifndef __ALARM_DUPLICATION_HPP__
#define __ALARM_DUPLICATION_HPP__

#include <string>
#include <unordered_map>
#include <vector>

#include "CommonCode.hpp"
#include "AlarmDataDef.hpp"

class AlarmDuplication
{
public:
    AlarmDuplication();
    ~AlarmDuplication();

    void Clear() { map_.clear(); }

    bool Is(ST_AlarmEventMsg & _stAlarmEventMsg);
    bool Manage(std::string  & _msg);

    void MakeSuperKey(std::string      & _super_key,
                      ST_AlarmEventMsg & _stAlarmEventMsg);
    void MakeSuperKey(std::string      & _super_key,
                      ST_AlarmStatus   & _stAlarmStatus);
    void MakeBodyForNotify(std::string                 & _out,
                           std::vector<ST_AlarmStatus> & _vec);

private:
    std::unordered_map<std::string, int>    map_;
};

#endif // __ALARM_DUPLICATION_HPP__
