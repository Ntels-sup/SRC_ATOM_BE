#include <stdio.h>
#include <time.h>
#include <mysql.h>

int insrtFunc(time_t stTm, time_t endTm, char *nodeName)
{
		MYSQL mysql;
		FILE *fp = NULL;
		int queryLen = 0;
		char query[8192];
		int rcvCnt, sndCnt;

		time_t curTm = 0;

		char file[1024];

		sprintf(file,"TEST_QUERY_%s.sql",nodeName);
		fp = fopen(file,"w");
		curTm = stTm;
		rcvCnt = time(NULL)%1000;
		sndCnt = time(NULL)%1010;
		fprintf(fp,"ALTER TABLE DIAM_STS DISABLE KEYS;\n");
		fprintf(fp,"INSERT INTO DIAM_STS(PRC_DATE, NODE_NAME, DST_FLAG, PEER, RCV_CNT, SND_CNT)\n");
		fprintf(fp,"VALUES(FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", %d, %d),\n", curTm, nodeName, "N", "PEER1",rcvCnt, sndCnt);

		fprintf(fp, "(FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", %d, %d)\n", curTm, nodeName, "N", "PEER2", rcvCnt, sndCnt);
		curTm += 300;
		while(1){
				if(curTm >= endTm){
						break;
				}

				fprintf(stderr,"curTime=%lu endTime=%lu, diff=%lu\n",curTm, endTm, endTm - curTm);

				fprintf(fp,", (FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", %d, %d)\n", curTm, nodeName, "N", "PEER1",rcvCnt, sndCnt);



				fprintf(fp, ", (FROM_UNIXTIME(%lu), \"%s\", \"%s\", \"%s\", %d, %d)\n", curTm, nodeName, "N", "PEER2", rcvCnt, sndCnt);

				rcvCnt++;
				sndCnt++;


				curTm += 300;
		}
		fprintf(fp,";");
		fprintf(fp,"ALTER TABLE DIAM_STS ENABLE KEYS");

		return 0;
}

int main(int argc, char **argv)
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


		insrtFunc(stTm, endTm,argv[1]);

		return 0;
}
