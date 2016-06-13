#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string>
#include <list>

using namespace std;

#define COM_TIMER_TYPE_SEC 0
#define COM_TIMER_TYPE_100M 1

#define COM_TIMER_TICK_SEC               1000000000     /* second */
#define COM_TIMER_TICK_100MS             100000000	     /* 0.1sec */
#define COM_TIMER_TICK_1MS               1000000        /* 1 millisecond */

#define COMERR_INVALID_TIMER_TYPE        302

#define TIMER_MAX_ENTRY 1024

#define CHECK_TICK_DIFF(prevTick, curTick, diff){\
	if(prevTick > curTick){\
		diff = (((unsigned long)~0) - prevTick) + curTick;\
	}\
	else {\
		diff = curTick - prevTick;\
	}\
}

class Timer{
	private:
		unsigned int type;
		unsigned int tmSlice;
		struct timespec lastTs;
		unsigned long errTm;
		unsigned long tps;
		struct timespec curTs;
		unsigned long avgDiff;
		unsigned long tick;

		void GetTime(struct timespec *tm) {
			struct timeval tv;

			gettimeofday(&tv, NULL);

			tm->tv_sec = tv.tv_sec;
			tm->tv_nsec = tv.tv_usec*1000;
		}

	public :
		Timer(unsigned int type){

			switch(type){
				case COM_TIMER_TYPE_SEC:
					tmSlice = COM_TIMER_TICK_SEC;
					type = COM_TIMER_TYPE_SEC;
					break;
				case COM_TIMER_TYPE_100M:
					tmSlice = COM_TIMER_TICK_100MS;
					type = COM_TIMER_TYPE_100M;
					break;
				default :
					type = COM_TIMER_TYPE_100M;
					break;
			};

			/* set init time */
			GetTime(&curTs);

			lastTs.tv_sec = curTs.tv_sec;
			lastTs.tv_nsec = curTs.tv_nsec;
			tps = COM_TIMER_TICK_SEC / tmSlice;

			avgDiff = 1000000000;

			tick = 0;

		};

		~Timer() {};

		void Calc(){
			unsigned long diff = 0;
			int ret = 0;

			GetTime(&curTs);

			if(curTs.tv_sec == lastTs.tv_sec){
				if(curTs.tv_nsec < lastTs.tv_nsec){
					errTm += avgDiff;
					diff = 0;
				}
				else{
					diff = (unsigned long)(curTs.tv_nsec - lastTs.tv_nsec);
					/*(tmr)->avgDiff = (diff + (tmr)->avgDiff) / 2;*/
					avgDiff = (0.875 * avgDiff) + (0.125 * diff);
				}
			}
			else if((ret=curTs.tv_sec - lastTs.tv_sec) > 0){
				diff = curTs.tv_nsec + (COM_TIMER_TICK_SEC - lastTs.tv_nsec);
				if(ret > 1){
					tick += (unsigned long)((ret -1) * tps);
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
			if(diff >= (unsigned long)tmSlice){
				tick += (unsigned long)(diff / (unsigned long)tmSlice);
				lastTs.tv_sec = curTs.tv_sec;
				lastTs.tv_nsec = curTs.tv_nsec;
				errTm = diff % (unsigned long)tmSlice;
			}
		};

		unsigned long getCurTick(){
			return tick;
		};

};

class TimerNode{
	private:
		unsigned long expTick;
		unsigned int event;
		void *data;
		unsigned int entryId;

	public:
		TimerNode(unsigned long expTick, unsigned int event ,unsigned int entryId, void *data){
			this->expTick = expTick;
			this->event = event;
			this->data = data;
			this->entryId = entryId;
		};

		~TimerNode() {
			fprintf(stderr,"remove node(event=%d\n)",this->event);
			this->data = NULL;
			this->expTick = 0;
			this->event = 0;
			this->entryId = 0;
		};

		unsigned int GetEntryId(){
			return entryId;
		};

		unsigned int GetEvent(){
			return event;
		};

		void *GetData() {
			return data;
		};

		unsigned long GetExpTick(){
			return expTick;
		};
};

class TimerEntry{
	private:
		list<TimerNode*> *node;

	public:
		TimerEntry(){
			node = new list<TimerNode*>;
			fprintf(stderr,"ENTRY init\n");
		};

		~TimerEntry() {
		};

		TimerNode* RegNode(unsigned long expTick, unsigned int event, unsigned int entryId, void *data) {
			TimerNode *regNode = NULL;

			regNode = new TimerNode(expTick, event, entryId, data);

			node->push_back(regNode);

			return regNode;
		};

		int DelNode(TimerNode *timerNode){
			node->remove(timerNode);
			delete timerNode;

			return 0;
		};

		int size(){
			return node->size();
		};

		int Find(unsigned long tick, unsigned int *rt_event, void **rt_data){
			TimerNode *findNode = NULL;
			list<TimerNode*>::iterator nodeIter;

			for(nodeIter = node->begin();nodeIter != node->end(); nodeIter++){
				findNode = *nodeIter;
				if(findNode->GetExpTick() >= tick){
					node->erase(nodeIter);

					*rt_event = findNode->GetEvent();
					*rt_data = findNode->GetData();

					delete findNode;

					return 1;
				}
			}

			return 0;
		};
};

class TimerHandler{
	private:
		int (*TmrEvtFunc)(unsigned int, void*);
		unsigned long prevTick;
		TimerEntry *entry;
		Timer *tm;

	public:
		TimerHandler(int (*TmrEvtFunc)(unsigned int, void*), Timer *tm){
			this->TmrEvtFunc = TmrEvtFunc;
			this->tm = tm;

			prevTick = tm->getCurTick();

			entry = new TimerEntry[TIMER_MAX_ENTRY];
		};

		~TimerHandler(){};

		TimerNode* Start(unsigned long expTm, unsigned int event, void *data){
			unsigned long expTick = 0;
			unsigned int idx = 0;
			TimerNode *node = NULL;

			 expTick = tm->getCurTick()+ expTm;

			 idx = expTick & (TIMER_MAX_ENTRY - 1);

			 node = entry[idx].RegNode(expTick, event, idx, data);

			 return node;
		};

		int Stop(TimerNode *node){
			unsigned int entryId = 0;

			entryId =  node->GetEntryId();

			entry[entryId].DelNode(node);

			return 0;
		};

		int Handler(){
			int ret = 0;
			TimerEntry *entry = NULL;
			unsigned int indx = 0;
			unsigned long diff = 0;
			unsigned long prevTick = 0;
			unsigned int event = 0;
			void *data = NULL;

			prevTick = this->prevTick;
			CHECK_TICK_DIFF(prevTick, tm->getCurTick(), diff);

			while(diff){
				prevTick++;

				/* select tm entry */
				indx = prevTick & (TIMER_MAX_ENTRY-1);
				entry = &this->entry[indx];

				if(entry->size() == 0){
					diff--;
					continue;
				}

				while(1){
					ret = entry->Find(prevTick, &event, &data);
					if(ret == 1){
						(*TmrEvtFunc)(event, data);
					}
					else if(ret == 0){
						break;
					}
				}
				diff--;
			}

			this->prevTick = prevTick;

			return 0;

		};

};

int func(unsigned int a, void *b)
{
	fprintf(stderr, "timer expired(a=%d, data=%d)\n", a, (int*)b);
	return 0;
}

class test{
	public:
		int func(unsigned int a, void *b)
		{
	fprintf(stderr, "timer expired(a=%d, data=%d)\n", a, (int*)b);
	return 0;
		};
};

int main()
{
	int i=0;
	Timer tm(COM_TIMER_TYPE_100M);
	TimerNode *node1 =NULL;
	TimerNode *node2 =NULL;
	TimerNode *node3 =NULL;
	TimerNode *node4 =NULL;
	test t;

	TimerHandler hdlr(func, &tm);
	//TimerHandler hdlr(t.func, &tm);
	//TimerHandler hdlr;

	node1 = hdlr.Start(30, 1, NULL);
	fprintf(stderr,"node1=0x%x\n", node1);
	node2 = hdlr.Start(30, 2, NULL);
	fprintf(stderr,"node2=0x%x\n", node2);
	node3 = hdlr.Start(50, 3, NULL);
	fprintf(stderr,"node3=0x%x\n", node3);
	node4 = hdlr.Start(30, 4, NULL);
	fprintf(stderr,"node4=0x%x\n", node4);

	hdlr.Stop(node1);
	for(i=0;i<100;i++){
		tm.Calc();

		printf("hello\n");
		printf("time=%lu\n",tm.getCurTick());
		hdlr.Handler();
		sleep(1);
	}
	//tm = new Timer(;
	return 0;
}
