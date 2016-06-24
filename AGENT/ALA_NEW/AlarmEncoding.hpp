
#ifndef __ALARM_ENCODING_HPP__
#define __ALARM_ENCODING_HPP__

#include <string>
#include "AlarmDataDef.hpp"

class AlarmEncoding
{
public:
    static void MakeBody(std::string      & _body,
                         ST_AlarmEventMsg & _stAlarmEventMsg);

private:
    explicit AlarmEncoding() { }
    ~AlarmEncoding() { }
};

#endif // __ALARM_ENCODING_HPP__
