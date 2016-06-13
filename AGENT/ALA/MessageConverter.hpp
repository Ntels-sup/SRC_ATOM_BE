
#ifndef __MESSAGE_CONVERTER_HPP__
#define __MESSAGE_CONVERTER_HPP__

#include <string>
#include "AlarmDataDef.hpp"

class MessageConverter
{
public:
    static bool Alarm(ST_AlarmEventMsg & _stAlarmEventMsg,
                      std::string      & _apiMsg);
    static bool Process(ST_AlarmEventMsg & _stAlarmEventMsg,
                      std::string        & _apiMsg);
    static bool Connect(ST_AlarmEventMsg & _stAlarmEventMsg,
                      std::string        & _apiMsg);
    static bool Ping(ST_AlarmEventMsg & _stAlarmEventMsg,
                      std::string     & _apiMsg);

private:
    explicit MessageConverter();
    ~MessageConverter();
};


#endif // __MESSAGE_CONVERTER_HPP__
