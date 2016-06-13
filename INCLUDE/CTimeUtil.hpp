
#ifndef __C_TIME_UTIL_HPP__
#define __C_TIME_UTIL_HPP__

#include <ctime>
#include <string>

// -lrt 필요.

class CTimeUtil
{
public:
    static void SetTimestampAndDstYn(std::string & _timestamp,
                                     char        & _dstYn,
                                     int           _millitime = 0);
    static void SetDstYn(std::string & _dstYn);

private:
    explicit CTimeUtil() { }
    ~CTimeUtil() { }
};

#endif // __C_TIME_UTIL_HPP__
