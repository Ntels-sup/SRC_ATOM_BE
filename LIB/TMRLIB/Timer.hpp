#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <sys/time.h>
#include "GenDef.hpp"

#define TIMER_TYPE_SEC               0
#define TIMER_TYPE_100M              1

#define TIMER_TICK_SEC               1000000000     /* second */
#define TIMER_TICK_100MS             100000000	     /* 0.1sec */
#define TIMER_TICK_1MS               1000000        /* 1 millisecond */

class Timer{
	private:
		UINT type;
		UINT tmSlice;
		struct timespec lastTs;
		ULONG errTm;
		ULONG tps;
		struct timespec curTs;
		ULONG avgDiff;
		ULONG tick;

		VOID GetTime(struct timespec *tm);

	public :
		Timer(UINT type);
		~Timer();

		UINT GetType();
		VOID Calc();
		ULONG CheckTickDiff(ULONG prevTick, ULONG curTick);
		ULONG GetCurTick();
};

#endif 
