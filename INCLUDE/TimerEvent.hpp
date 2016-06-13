#ifndef _TIMEREVENT_HPP_
#define _TIMEREVENT_HPP_

#include "TimerEntry.hpp"

#define TIMER_MAX_ENTRY 1024

class TimerEvent : public Timer{
	private:
		ULONG prevTick;
		TimerEntry *entry;

	public:
		TimerEvent(UINT type);
		~TimerEvent();

		virtual RT_RESULT TmrEvtFunc(UINT event, VOID *data) = 0;
		TimerNode* Start(ULONG expTm, UINT event, VOID *data);
		RT_RESULT Stop(TimerNode *node);
		RT_RESULT Handler();
};

#endif
