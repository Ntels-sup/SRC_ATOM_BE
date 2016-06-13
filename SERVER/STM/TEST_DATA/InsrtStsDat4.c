#include <stdio.h>
#include <time.h>
#include <mysql.h>

int insrtFunc(time_t stTm, time_t endTm)
{
		MYSQL mysql;
		FILE *fp = NULL;
		int queryLen = 0;
		char query[8192];
		unsigned int i = 0;

		time_t curTm = 0;

		fp = fopen("./TEST_QUERY4.sql","w");
		curTm = stTm;
		i = 1;
		//fprintf(fp,"ALTER TABLE DIAM_STS DISABLE KEYS;\n");
		fprintf(fp,"INSERT INTO DIAM_HIST(SEQUENCE, PRC_DATE, NODE_NAME, DST_FLAG, VALUE)\n");
		fprintf(fp,"VALUES(%d, FROM_UNIXTIME(%lu), \"%s\", \"%s\", %d)\n", i, curTm, "CSCF", "N",  100);
		//curTm += 300;
		//fprintf(fp,"(FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", \"%s\", '%c', %d, %d, %d)\n", curTm, "CSCF", "N", 
		//				"GIF","PGW01", 'P', 300, 300000, 77850000);

		i++;
		curTm += 300;
		while(1){
				if(curTm >= endTm){
						break;
				}

				fprintf(stderr,"curTime=%lu endTime=%lu, diff=%lu\n",curTm, endTm, endTm - curTm);

		fprintf(fp,", (%d, FROM_UNIXTIME(%lu), \"%s\", \"%s\", %d)\n", i, curTm, "CSCF", "N",  100);
				curTm += 300;
				i++;
		}
		//fprintf(fp,";");
		//fprintf(fp,"ALTER TABLE DIAM_STS ENABLE KEYS;\n");

		return 0;
}

int main()
{
		time_t stTm = 0;
		time_t endTm = 0;
		struct tm *t = NULL;

		endTm = time(NULL);

		t = localtime(&endTm);
		endTm += t->tm_gmtoff;

		//t = localtime(&endTm);
		if((endTm % 300) != 0){
				endTm -= (endTm%300);
		}
		//printf("%s",ctime(&endTm));

		//stTm = endTm - 2592000;
		stTm = endTm - 31536000; /* 1 year */
//stTm = endTm - 63072000; /* 2 year */
//stTm = endTm - 47260800; /* 1.6 year */


		insrtFunc(stTm, endTm);

		return 0;
}
