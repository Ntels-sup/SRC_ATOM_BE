
#ifndef __ALARM_UTIL_HPP__
#define __ALARM_UTIL_HPP__

#include <string>

#include "DB.hpp"
#include "FetchMaria.hpp"
#include "AlarmDataDef.hpp"

class AlarmUtil
{
public:
    static bool SetAlarmCodeDef(DB *                _db,
                                ST_AlarmCodeDef &   _st,
                                std::string &       _code,
                                const char *        _pkg_name = "");
private:
    static int queryAlarmCode(DB          *  _db,
                              std::string & _code,
                              const char  * _pkg_name);


private:
    AlarmUtil() {}
    ~AlarmUtil() {}

};


#endif // __ALARM_UTIL_HPP__
