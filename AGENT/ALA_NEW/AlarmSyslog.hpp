
#ifndef __ALARM_SYSLOG_HPP__
#define __ALARM_SYSLOG_HPP__

#include <syslog.h>
#include <string>
#include "AlarmDataDef.hpp"

class AlarmSyslog
{
public:
    static void SendSyslog( ST_AlarmEventMsg & _stAlarmEventMsg );

	static std::string	GetAlarmGroup( int a_nAlarmGroupId );
	static std::string	GetAlarmSeverity( int a_nAlarmSeverityId );

private:
    explicit AlarmSyslog	();
    ~AlarmSyslog		();

};

#endif // __ALARM_SYSLOG_HPP__
