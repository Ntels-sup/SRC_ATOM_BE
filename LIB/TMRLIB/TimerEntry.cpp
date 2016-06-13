#include "TimerEntry.hpp"

TimerEntry::TimerEntry()
{
	node = new list<TimerNode*>;
}

TimerEntry::~TimerEntry()
{
	delete node;
}

TimerNode* TimerEntry::RegNode(ULONG expTick, UINT event, UINT entryId, VOID *data)
{
	TimerNode *regNode = NULL;

	regNode = new TimerNode(expTick, event, entryId, data);

	node->push_back(regNode);

	return regNode;
}

RT_RESULT TimerEntry::DelNode(TimerNode *timerNode)
{
	node->remove(timerNode);
	delete timerNode;

	return RC_OK;
}

UINT TimerEntry::Size()
{
	return node->size();
}

RT_RESULT TimerEntry::Find(ULONG tick, UINT *rt_event, VOID **rt_data)
{
	TimerNode *findNode = NULL;
	list<TimerNode*>::iterator nodeIter;

	for(nodeIter = node->begin();nodeIter != node->end(); nodeIter++){
		findNode = *nodeIter;
		if(findNode->GetExpTick() >= tick){
			node->erase(nodeIter);

			*rt_event = findNode->GetEvent();
			*rt_data = findNode->GetData();

			delete findNode;

			return RC_OK;
		}
	}

	return RC_NOK;
}

RT_RESULT TimerEntry::DelAllNode()
{
	TimerNode *findNode = NULL;
	list<TimerNode*>::iterator nodeIter;

	for(nodeIter = node->begin();nodeIter != node->end(); nodeIter++){
		findNode = *nodeIter;
		node->erase(nodeIter);

		delete findNode;
	}

	return RC_OK;
}
