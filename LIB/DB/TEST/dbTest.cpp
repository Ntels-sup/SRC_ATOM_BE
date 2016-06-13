
#include <cstdio>
#include <cstring>
#include <cassert>
#include <time.h>
#include <signal.h>

#include "MariaDB.hpp"
#include "FetchMaria.hpp"

bool check_func(DB * _db)
{
    printf("-- Function Check ----- \n");

    printf("-- 1) Connect -Wrong -- \n");
    assert(_db->Connect("127.0.0.1", 3307, "atom", "atom", "ATOM") < 0 && "Wrong Config");

    printf("-- 2) IsConnect -False- \n");
    assert(_db->IsConnect() == false && "False");

    printf("-- 3) Connect -False- \n");
    assert(_db->Connect("127.0.0.1", 3307, "atom", "atom", "ATOM") < 0 && "Correct Config");

    sleep(2);
    printf("-- 4) Connect - TimeOut Check - \n");
    assert(_db->Connect("127.0.0.1", 3307, "atom", "atom", "ATOM") < 0 && "Time Checker");

    sleep(7);
    printf("-- 5) Connect -Correct- \n");
    // assert(_db->Connect("127.0.0.1", 3306, "atom", "atom", "ATOM") == true && "Correct Config");
    assert(_db->Connect("localhost", 3306, "atom", "atom", "ATOM", "/tmp/mysql.sock") > 0 && "Correct Config");

    printf("-- 6) IsConnect -True - \n");
    assert(_db->IsConnect() == true && "In Connect");

    printf("-- 7) Close -True - \n");
    _db->Close();
    assert(_db->IsConnect() != true && "Close");


    printf("-- 8) Insert -False - \n");
    _db->Connect("127.0.0.1", 3306, "atom", "atom", "ATOM");

    // Commit
    _db->SetAutoCommit(true);

    char sql[512];
    sprintf(sql, "INSERT INTO (CMD, DEST_NAME) VALUES () FROM TAT_JHCHOI");
    int ret = _db->Execute(sql, strlen(sql));
	printf("---- return [%d]\n", ret);
    assert(_db->Execute(sql, strlen(sql)) != 1 && "INSERT Fail");

    printf("-- 9) Insert -True - \n");
    sprintf(sql, "INSERT INTO TAT_JHCHOI (CMD, DEST_NAME) VALUES ('DB-TEST', 'jhchoi')");
	printf("## [%s]\n", sql);
    assert(_db->Execute(sql, strlen(sql)) == 1 && "INSERT Success");

    printf("-- 10) Select - 1 more than - \n");
	FetchMaria 	fdata;

    sprintf(sql, "SELECT CMD, DEST_NAME FROM TAT_JHCHOI WHERE DEST_NAME='jhchoi'");

    int cnt =  _db->Query(&fdata, sql, strlen(sql));
    printf("Query Result Cnt [%d]\n", cnt);
    assert(cnt > 0 && "SELECT Cnt One more than");



    printf("-- 11) Select -True - \n");

    sprintf(sql, "SELECT MSG_ID, CODE, PRC_DATE, CLEARED_YN FROM TAT_ALM_HIST");
    assert(_db->Query(&fdata, sql, strlen(sql)) > 0 && "SELECT Success");

    printf("-- 12) Fetch For FetchData -True - \n");

    char msg_id[32];
    char code[32];
    char prc_date[32];
    char cleared_yn[8];

    fdata.Set(msg_id,   sizeof(msg_id));
    fdata.Set(code,     sizeof(code));
    fdata.Set(prc_date, sizeof(prc_date));
    fdata.Set(cleared_yn,sizeof(cleared_yn));

    while(1)
    {
        if(fdata.Fetch() == false)
            break;

        printf(" [%s] [%s] [%s] [%s]\n", msg_id, code, prc_date, cleared_yn);
        printf(" [%s] [%s] [%s] [%s]\n", fdata.Get(0),
                                         fdata.Get(1),
                                         fdata.Get(2),
                                         fdata.Get(3));

    }

    printf("-- 13) Select - 0 cnt \n");
    sprintf(sql, "SELECT PRC_DATE FROM TAT_STS_ALM");

    cnt =  _db->Query(&fdata, sql, strlen(sql));
    printf("Query Result Cnt [%d]\n", cnt);
    assert(cnt == 65 && "SELECT Cnt 0");

    printf("-- 14) Fetch select cnt is 0\n");

    fdata.Set(prc_date, sizeof(prc_date));

    while(1)
    {
        if(fdata.Fetch() == false)
            break;

        printf(" [%s]\n", prc_date);
        printf(" [%s]\n", fdata.Get(0));
    }

    printf("-- 15) Update - no effect\n");
    sprintf(sql, "UPDATE TAT_JHCHOI SET CMD='DB-TEST' WHERE DEST_NAME='jhchoi'");

    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);
    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);
    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);
    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);

    // assert(cnt == 0 && "Update Cnt 0");

    printf("-- 16) Update - no effect\n");
    sprintf(sql, "UPDATE TAT_JHCHOI SET CMD='DB-TEST-1' WHERE DEST_NAME='jhchoi'");

    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);
    assert(cnt > 0 && "Update Cnt is one more");

    printf("-- 17) Delete\n");
    sprintf(sql, "DELETE FROM TAT_JHCHOI WHERE DEST_NAME='jhchoi'");

    cnt = _db->Execute(sql, strlen(sql));
    printf("Execute Result Cnt [%d]\n", cnt);
    assert(cnt > 0 && "Delete Cnt is one more");

	std::string str = _db->GetErrorMsg(-2060);
    printf("# [%s]\n", str.c_str());

    return true;
}


int main(int argc, char * argv[])
{
    DB * db = new MariaDB();
	db->UsedThread();

    if(check_func(db) == false)
        return 0;

    delete db;

    return 0;
}
