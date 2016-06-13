
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "DB_r.hpp"
#include "MariaDB_r.hpp"
#include "FetchMaria.hpp"

bool g_bInService = true;

void inifinte_1(DB_r * _db)
{
    char    code[16];
    char    probable_cause[32];
    char    severity_ccd[4];

    FetchMaria      f;
    f.Clear();

    f.Set(code,             sizeof(code));
    f.Set(probable_cause,   sizeof(probable_cause));
    f.Set(severity_ccd,     sizeof(severity_ccd));

	char sql[128];
	sprintf(sql, "SELECT CODE, PROBABLE_CAUSE, SEVERITY_CCD FROM TAT_ALM_CODE_DEF");

	int ret = 0;
	if(_db->Query(ret,
				 f,
				 sql,
				 strlen(sql)) < 0)
	{
        printf("Init Fail [%d:%s]\n", ret, sql);
        return ;
	}

    while(true)
    {
        if(f.Fetch() != true)
            break;

        printf("[%lu] CODE [%s] PROBABLE_CAUSE [%s] SEVERITY_CCD [%d]\n",
            pthread_self(), code, probable_cause, atoi(severity_ccd));

        if(strlen(code) != 8 || strlen(severity_ccd) != 1)
        {
            g_bInService = false;
            printf("-------------- MIXED\n");
            break;
        }
    }
}

void inifinte_2(DB_r * _db)
{
    char    a[32];
    char    b[32];
    char    c[64];

    FetchMaria      f;
    f.Clear();

    f.Set(a,   sizeof(a));
    f.Set(b,   sizeof(b));
    f.Set(c,   sizeof(c));

    int ret = 0;
	char sql[128];
	sprintf(sql, "SELECT GROUP_CD, DETAIL_CD, CODE_NAME FROM TAT_COMMON_CODE");

    if(_db->Query(ret,
				  f,
				  sql,
				  strlen(sql)) < 0)
    {
        printf("Init Fail [%d:%s]\n", ret, sql);
        return ;
    }

    while(true)
    {
        if(f.Fetch() != true)
            break;

        printf("[%lu] A [%s] B [%s] C [%s]\n",
            pthread_self(), a, b, c);

        if(strlen(a) != 6)
        {
            g_bInService = false;
            printf("-------------- MIXED\n");
            break;
        }
    }
}

void inifinte_3(DB_r * _db)
{
    char    d[16];
    char    e[32];
    char    g[32];

    FetchMaria      f;
    f.Clear();

    f.Set(d,   sizeof(d));
    f.Set(e,   sizeof(e));
    f.Set(g,   sizeof(g));

    int ret = 0;
	char sql[128];
	sprintf(sql, "SELECT RSC_ID, RSC_NAME, RSC_GRP_ID FROM TAT_RSC_DEF");

    if(_db->Query(ret,
				  f,
				  sql,
				  strlen(sql)) < 0)
    {
        printf("Init Fail [%d:%s]\n", ret, sql);
        return ;
    }

    while(true)
    {
        if(f.Fetch() != true)
            break;

        printf("[%lu] D [%s] E [%s] G [%s]\n",
            pthread_self(), d, e, g);

        if(strlen(d) != 10 || strlen(g) != 10)
        {
            g_bInService = false;
            printf("-------------- MIXED\n");
            break;
        }
    }
}

void * thr_fn(void * arg)
{
    DB_r * _db = (DB_r *)arg;

    while(g_bInService)
    {
        if(pthread_self() % 3 == 0)
            inifinte_1(_db);
        else if(pthread_self() % 3 == 1)
            inifinte_2(_db);
        else
            inifinte_3(_db);
    }

    pthread_exit(0);
}

int main()
{
    DB_r * db = new MariaDB_r();

	int ret = 0;
    if(db->Connect(ret,
				 "192.168.10.43",
                 3306,
                 "atom",
                 "atom",
                 "ATOM") == false)
    {
        printf("Init Fail [%d]\n", ret);
        return 0;
    }

    pthread_t   tid;

    for(int nLoop=0; nLoop < 30; nLoop++)
    {
        if(pthread_create(&tid, NULL, thr_fn, db) != 0)
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
