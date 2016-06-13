
#ifndef __VNFM_ALARM_HPP__
#define __VNFM_ALARM_HPP__

#include <string>
#include <unordered_map>

#include "DB.hpp"
#include "AlarmDataDef.hpp"

class VnfmAlarm
{
public:
    VnfmAlarm();
    ~VnfmAlarm();

    bool Init(DB         * _db,
              const char * _pkg_name);
    bool MakeBody(std::string       & _out,
                  ST_AlarmEventMsg  & _stAlarmEventMsg);

    void GetAlarmGroupName(std::string & _out, int _event_group_id);
    void GetAlarmSeverityName(std::string & _out, int _severity_id);

    bool IsRebootY(std::string & _code);

private:
    bool setMapVnfmSendConf(DB * _db, const char * _pkg_name);
    bool setAlarmGroupString(DB * _db);
    bool setAlarmSeverityString(DB * _db);

    ST_VnfmSendDef * getVnfmSendDef(std::string _code);


private:

    // key : code
    std::unordered_map<std::string, ST_VnfmSendDef>   map_vnfm_send_conf_;

    // key : group_cd of TAT_COMMON_CODE
    std::unordered_map<int, std::string>      map_group_ccd_;
    std::unordered_map<int, std::string>      map_severity_ccd_;

};


#endif // __VNFM_ALARM_HPP__
