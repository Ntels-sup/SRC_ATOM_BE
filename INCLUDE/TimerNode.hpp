#ifndef _TIMERNODE_HPP_
#define _TIMERNODE_HPP_
#include "GenDef.hpp"

class TimerNode{
	private:
		ULONG expTick;
		UINT event;
		VOID *data;
		UINT entryId;

	public:
		TimerNode(ULONG expTick, UINT event ,UINT entryId, VOID *data);

		~TimerNode();

		UINT GetEntryId();
		UINT GetEvent();
		VOID *GetData();
		ULONG GetExpTick();
};

#endif
