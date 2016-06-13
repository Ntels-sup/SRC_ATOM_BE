#include "TimerEvent.hpp"

class Test : public TimerEvent{
	public:
		Test(UINT type) : TimerEvent(type){};
		RT_RESULT TmrEvtFunc(UINT event, VOID *data){
			fprintf(stderr,"TIMEOUT EVENT(event=%d, data=0x%x)\n",event, *(UINT*)&data);
			return RC_OK;
		};
};

int main()
{
	int i=0;
	TimerNode *node1 =NULL;
	TimerNode *node2 =NULL;
	TimerNode *node3 =NULL;
	TimerNode *node4 =NULL;
	Test tmTest(TIMER_TYPE_100M);

	node1 = tmTest.Start(30, 1, (VOID*)11);
	fprintf(stderr,"start timer event(event=%d, data=0x%x)\n", 1, *(UINT*)&node1);
	node2 = tmTest.Start(30, 2, (VOID*)12);
	fprintf(stderr,"start timer event(event=%d, data=0x%x)\n", 2, *(UINT*)&node2);
	node3 = tmTest.Start(50, 3, (VOID*)13);
	fprintf(stderr,"start timer event(event=%d, data=0x%x)\n", 3, *(UINT*)&node3);
	node4 = tmTest.Start(30, 4, (VOID*)14);
	fprintf(stderr,"start timer event(event=%d, data=0x%x)\n", 4, *(UINT*)&node4);

	tmTest.Stop(node1);
	fprintf(stderr,"cancel timer event(event=%d, data=0x%x)\n", 1, *(UINT*)&node1);
	for(i=0;i<100;i++){
		tmTest.Calc();

		printf("CURENT TIME=%lu\n",tmTest.GetCurTick());
		tmTest.Handler();
		sleep(1);
	}
	//tm = new Timer(;
	return 0;
}
