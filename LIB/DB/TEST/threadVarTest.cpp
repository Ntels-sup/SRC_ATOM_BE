
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

bool g_bInService = true;
pthread_mutex_t 	g_mutex;

class CTest
{
public:
	CTest() { };
	~CTest() { };

	void Call(pthread_t 	tid);
private:
	int  m;
};

void CTest::
Call(pthread_t 	tid)
{
	int a;

	pthread_mutex_lock(&g_mutex);

	printf("< %u - %p   %p\n", pthread_self(), &a, &m);

	pthread_mutex_unlock(&g_mutex);
}

void f1(bool _flag)
{
	int 	a;
	int 	b;
	int 	c;
	char 	d[256];
	int 	e;

	if(_flag)
	{
		pthread_mutex_lock(&g_mutex);

		printf("# %u - %p\n", pthread_self(), &a);
		printf("# %u - %p\n", pthread_self(), &b);
		printf("# %u - %p\n", pthread_self(), &c);
		printf("# %u - %p\n", pthread_self(), &d);
		printf("# %u - %p\n", pthread_self(), &e);

		pthread_mutex_unlock(&g_mutex);
	}
}

void f2(bool _flag)
{
	int 	a;
	int 	b;

	int 	c;
	char 	d[256];
	int 	e;

	if(_flag)
	{
		pthread_mutex_lock(&g_mutex);

		printf("! %u - %p\n", pthread_self(), &a);
		printf("! %u - %p\n", pthread_self(), &b);
		printf("! %u - %p\n", pthread_self(), &c);
		printf("! %u - %p\n", pthread_self(), &d);
		printf("! %u - %p\n", pthread_self(), &e);

		pthread_mutex_unlock(&g_mutex);
	}
}

void f3(bool _flag)
{
	int 	a;
	int 	b;

	f2(_flag);

	int 	c;
	char 	d[256];
	int 	e;

	if(_flag)
	{
		pthread_mutex_lock(&g_mutex);

		printf("$ %u - %p\n", pthread_self(), &a);
		printf("$ %u - %p\n", pthread_self(), &b);
		printf("$ %u - %p\n", pthread_self(), &c);
		printf("$ %u - %p\n", pthread_self(), &d);
		printf("$ %u - %p\n", pthread_self(), &e);

		pthread_mutex_unlock(&g_mutex);
	}
}


void * thr_fn(void * inst)
{
	CTest * p = (CTest *)inst;

	int bfirst = true;
	while(g_bInService)
	{
		if(bfirst)
		{
			p->Call(pthread_self());
			printf("* %u - %p\n", pthread_self(), (void *)(*f1));
			printf("* %u - %p\n", pthread_self(), (void *)(*f2));
			printf("* %u - %p\n", pthread_self(), (void *)(*f3));
		}

		f1(bfirst);
		f2(bfirst);
		f3(bfirst);

		bfirst = false;
	}

    pthread_exit(0);
}

int main()
{

	pthread_mutex_init(&g_mutex, NULL);

	CTest 	a;

    pthread_t   tid;

    for(int nLoop=0; nLoop < 30; nLoop++)
    {
        if(pthread_create(&tid, NULL, thr_fn, (void *)&a) != 0)
        {
            printf("thread create fail\n");
            return 0;
        }

        pthread_detach(tid);

        printf("creat thread success [%d]\n", nLoop);
    }

    printf("i am going to sleep\n");

    sleep(10);

    printf("i am going to down\n");
    g_bInService = false;

    sleep(3);

    printf("terminated\n");
    return 0;
}
