#ifndef _TIMERENTRY_HPP_
#define _TIMERENTRY_HPP_

#include <list>
#include "Timer.hpp"
#include "TimerNode.hpp"

using namespace std;

class TimerEntry{
	private:
		list<TimerNode*> *node;

	public:
		TimerEntry();
		~TimerEntry();

		TimerNode* RegNode(ULONG expTick, UINT event, UINT entryId, VOID *data);

		RT_RESULT DelNode(TimerNode *timerNode);
		UINT Size();

		RT_RESULT Find(ULONG tick, UINT *rt_event, VOID **rt_data);
		RT_RESULT DelAllNode();
};

#endif

