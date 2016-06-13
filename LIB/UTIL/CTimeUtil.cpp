

#include "CTimeUtil.hpp"



void CTimeUtil::
SetTimestampAndDstYn(std::string & _timestamp,
                     char        & _dstYn,
                     int           _millitime)
{
    char    date[64];

    struct tm       stT;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);


    int size = strftime(date,
                        sizeof(date),
                        "%Y-%m-%d %H:%M:%S",
                        localtime_r(&(now.tv_sec), &stT));

    if(stT.tm_isdst > 0)
        _dstYn = 'Y';
    else
        _dstYn = 'N';

    if(_millitime != 0)
    {
        if(_millitime == 1)
            sprintf(date+size, ".%01d", (int)(now.tv_nsec / (1000 * 1000 * 100)) );
        else if(_millitime == 2)
            sprintf(date+size, ".%02d", (int)(now.tv_nsec / (1000 * 1000 * 10)) );
        else
            sprintf(date+size, ".%03d", (int)(now.tv_nsec / (1000 * 1000)) );
    }

    _timestamp = date;

    return ;
}

void CTimeUtil::
SetDstYn(std::string & _dstYn)
{
    time_t      now = time(NULL);
    struct tm   stT;
    localtime_r(&now, &stT);

    if(stT.tm_isdst > 0)
        _dstYn = "Y";
    else
        _dstYn = "N";
}
