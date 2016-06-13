
#ifndef __TIME_OUT_CHECK_HPP__
#define __TIME_OUT_CHECK_HPP__

#include <time.h>

class TimeOutCheck
{
    public :
        TimeOutCheck() { t = 0; }
        ~TimeOutCheck() { }

        void Update(time_t _time) { t = _time; };
        void Update() { t = time(NULL); };

        bool TimeOut(int period) {return (time(NULL) - period) > t?true:false;};
        bool TimeOut(int period, time_t now)
            { return (now - period) > t?true:false; }

    private :
        time_t     t;
};


#endif // __TIME_OUT_CHECK_HPP__
