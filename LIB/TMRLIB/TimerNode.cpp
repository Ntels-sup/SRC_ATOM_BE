#include "TimerNode.hpp"

TimerNode::TimerNode(ULONG expTick, UINT event ,UINT entryId, VOID *data)
{
	this->expTick = expTick;
	this->event = event;
	this->data = data;
	this->entryId = entryId;
}

TimerNode::~TimerNode() 
{
	//fprintf(stderr,"remove node(event=%d\n)",this->event);
	this->data = NULL;
	this->expTick = 0;
	this->event = 0;
	this->entryId = 0;
}

UINT TimerNode::GetEntryId()
{
	return entryId;
}

UINT TimerNode::GetEvent()
{
	return event;
};

VOID *TimerNode::GetData()
{
	return data;
}

ULONG TimerNode::GetExpTick()
{
	return expTick;
}
