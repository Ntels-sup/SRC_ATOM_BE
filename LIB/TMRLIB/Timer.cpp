#include "Timer.hpp"

Timer::Timer(UINT type){
    switch(type){
        case TIMER_TYPE_SEC:
            tmSlice = TIMER_TICK_SEC;
            type = TIMER_TYPE_SEC;
            break;
        case TIMER_TYPE_100M:
            tmSlice = TIMER_TICK_100MS;
            type = TIMER_TYPE_100M;
            break;
        default :
            type = TIMER_TYPE_100M;
            break;
    };

    /* set init time */
    GetTime(&curTs);

    lastTs.tv_sec = curTs.tv_sec;
    lastTs.tv_nsec = curTs.tv_nsec;
    tps = TIMER_TICK_SEC / tmSlice;

    errTm = 0;
    tps = 0;
    avgDiff = 1000000000;

    tick = 0;
}

Timer::~Timer()
{
};

UINT Timer::GetType()
{
    return type;
}

VOID Timer::GetTime(struct timespec *tm)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    tm->tv_sec = tv.tv_sec;
    tm->tv_nsec = tv.tv_usec*1000;
}

VOID Timer::Calc()
{
    ULONG diff = 0;
    SINT ret = 0;

    GetTime(&curTs);

    if(curTs.tv_sec == lastTs.tv_sec){
        if(curTs.tv_nsec < lastTs.tv_nsec){
            errTm += avgDiff;
            diff = 0;
        }
        else{
            diff = (ULONG)(curTs.tv_nsec - lastTs.tv_nsec);
            /*(tmr)->avgDiff = (diff + (tmr)->avgDiff) / 2;*/
            avgDiff = (0.875 * avgDiff) + (0.125 * diff);
        }
    }
    else if((ret=curTs.tv_sec - lastTs.tv_sec) > 0){
        diff = curTs.tv_nsec + (TIMER_TICK_SEC - lastTs.tv_nsec);
        if(ret > 1){
            tick += (ULONG)((ret -1) * tps);
            lastTs.tv_sec = curTs.tv_sec;
        }
        /*(tmr)->avgDiff = (diff + (tmr)->avgDiff) / 2;*/
        avgDiff = (0.875 * avgDiff) + (0.125 * diff);
    }
    else {
        errTm += avgDiff;
        diff = 0;
    }
    if(diff > (avgDiff * 3)){
        diff = avgDiff;
    }
    diff += errTm;
    if(diff >= (ULONG)tmSlice){
        tick += (ULONG)(diff / (ULONG)tmSlice);
        lastTs.tv_sec = curTs.tv_sec;
        lastTs.tv_nsec = curTs.tv_nsec;
        errTm = diff % (ULONG)tmSlice;
    }
}

ULONG Timer::CheckTickDiff(ULONG prevTick, ULONG curTick)
{
	if(prevTick > curTick){
		return ((((ULONG)~0) - prevTick) + curTick);
	}
	else {
		return curTick - prevTick;
	}
}


ULONG Timer::GetCurTick(){
	return tick;
}

