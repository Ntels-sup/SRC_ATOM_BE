#include "TimerEvent.hpp"

TimerEvent::TimerEvent(UINT type) : Timer(type)
{

	prevTick  = GetCurTick();

	entry = new TimerEntry[TIMER_MAX_ENTRY];
}

TimerEvent::~TimerEvent()
{
	delete entry;
}

TimerNode* TimerEvent::Start(ULONG expTm, UINT event, VOID *data)
{
	ULONG expTick = 0;
	UINT idx = 0;
	TimerNode *node = NULL;

	expTick = GetCurTick() + expTm;

	idx = expTick & (TIMER_MAX_ENTRY - 1);

	node = entry[idx].RegNode(expTick, event, idx, data);

	return node;
}

RT_RESULT TimerEvent::Stop(TimerNode *node)
{
	UINT entryId = 0;

	entryId =  node->GetEntryId();

	entry[entryId].DelNode(node);

	return RC_OK;
}

RT_RESULT TimerEvent::Handler()
{
	SINT ret = 0;
	TimerEntry *entry = NULL;
	UINT indx = 0;
	ULONG diff = 0;
	ULONG prevTick = 0;
	UINT event = 0;
	VOID *data = NULL;

	prevTick = this->prevTick;
	diff = CheckTickDiff(prevTick, GetCurTick());

	while(diff){
		prevTick++;

		/* select tm entry */
		indx = prevTick & (TIMER_MAX_ENTRY-1);
		entry = &this->entry[indx];

		if(entry->Size() == 0){
			diff--;
			continue;
		}

		while(1){
			ret = entry->Find(prevTick, &event, &data);
			if(ret == 1){
				//(*TmrEvtFunc)(event, data);
				TmrEvtFunc(event, data);
			}
			else if(ret == 0){
				break;
			}
		}
		diff--;
	}

	this->prevTick = prevTick;

	return RC_OK;
}

