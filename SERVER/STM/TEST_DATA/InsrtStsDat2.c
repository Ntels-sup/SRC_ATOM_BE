#include <stdio.h>
#include <time.h>
#include <mysql.h>

int insrtFunc(time_t stTm, time_t endTm)
{
		MYSQL mysql;
		FILE *fp = NULL;
		int queryLen = 0;
		char query[8192];

		time_t curTm = 0;

		fp = fopen("./TEST_QUERY2.sql","w");
		curTm = stTm;
		//fprintf(fp,"ALTER TABLE DIAM_STS DISABLE KEYS;\n");
		fprintf(fp,"INSERT INTO TOF_STAT_FILE(PRC_DATE, NODE_NAME, DST_FLAG, PROC_NAME, CTF_NODE_NAME, RECV_PATH, FILE_CNT, RECORD_CNT, FILE_SIZE)\n");
		fprintf(fp,"VALUES(FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", \"%s\", '%c', %d, %d, %d)\n", curTm, "CSCF", "N", 
						"GIF","PGW01", 'P', 300, 300000, 77850000);
		//curTm += 300;
		//fprintf(fp,"(FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", \"%s\", '%c', %d, %d, %d)\n", curTm, "CSCF", "N", 
		//				"GIF","PGW01", 'P', 300, 300000, 77850000);

		curTm += 300;
		while(1){
				if(curTm >= endTm){
						break;
				}

				fprintf(stderr,"curTime=%lu endTime=%lu, diff=%lu\n",curTm, endTm, endTm - curTm);

				fprintf(fp,", (FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", \"%s\", '%c', %d, %d, %d)\n", curTm, "CSCF", "N", 
								"GIF","PGW01", 'P', 300, 300000, 77850000);
				curTm += 300;
		}
		fprintf(fp,";");

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
