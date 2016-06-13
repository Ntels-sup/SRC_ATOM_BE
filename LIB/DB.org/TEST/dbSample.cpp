#include <cstdio>
#include <cstring>
#include <cassert>
#include <time.h>
#include <signal.h>

#include "MariaDB.hpp"
// #include "FetchData.hpp"
#include "FetchMaria.hpp"

void callfunc(DB * _db)
{

    char sql[256];

    printf("--- inside callfunc\n");
    sprintf(sql, "SELECT * FROM TAT_DB_TEST");

    if(_db->Query(sql, strlen(sql)) < 0)
    {
        printf("---- Query Fail [%s] [%d:%s]\n", sql, _db->GetError(), _db->GetErrorMsg());
        delete _db;
        return ;
    }
    else
        printf("SELECT OK [%s]\n\n", sql);

    char    test_id[32];
    char    test[32];
    char    prc_date[32];
    char    test_name[32];

    FetchMaria   fdata;

    fdata.Clear();

    fdata.Set(test_id,      sizeof(test_id));
    fdata.Set(test,         sizeof(test));
    fdata.Set(prc_date,     sizeof(prc_date));
    fdata.Set(test_name,    sizeof(test_name));

    while(true)
    {
        if(fdata.Fetch(_db) == false)
            break;

        printf(" [%s] [%s] [%s] [%s]\n\n", test_id, test, prc_date, test_name);
    }

    printf("--- end callfunc\n");
}

int main()
{
	// NEW
    DB * db = new (std::nothrow) MariaDB();

    if(db == NULL)
    {
        printf("new operator Fail [%d:%s]\n", errno, strerror(errno));
        return 0;
    }

	// CONNECT
    if(db->Connect("127.0.0.1", 3306, "atom", "atom", "ATOM")  != true)
    {
        printf("Connect Fail [%d:%s]\n", db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }

	char sql[1024];
	sprintf(sql, 
		"UPDATE TAT_BATCH_HIST SET END_DATE = NOW(), EXIT_CD = 9999, SUCCESS_YN = 'Y' WHERE GROUP_NAME = 'groupNm' AND JOB_NAME = 'JobNm2' AND PRC_DATE = '20160420110407' AND PKG_NAME = 'ATOM'");

	if(db->Execute(sql, strlen(sql)) < 0)
	{
		printf("------\n");	
	} 
		printf("#######\n");	

	/*--
	// CREATE TABLE
    char sql[256];
    sprintf(sql, "CREATE TABLE  `TAT_DB_TEST` ("
                    "`TEST_ID` INT(11) NOT NULL AUTO_INCREMENT,"
                    "`TEST` VARCHAR(128) NULL DEFAULT NULL,"
                    "`PRC_DATE` datetime NULL DEFAULT NULL,"
                    "`TEST_NAME` VARCHAR(32) NULL DEFAULT NULL,"
                    "PRIMARY KEY (`TEST_ID`)"
                 ")");

    int cnt = 0;
    if((cnt = db->Execute(sql, strlen(sql))) < 0)
    {
        printf("Execute Fail [%d]  [%d:%s]\n", cnt, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("CREATE TABLE OK\n\n");

	// INSERT
    sprintf(sql, "INSERT TAT_DB_TEST (TEST, TEST_NAME) VALUES ('A', 'ABCDEFG')");
    if((cnt = db->Execute(sql, strlen(sql))) <= 0)
    {
        printf("Execute Fail [%d] [%s] [%d:%s]\n", cnt, sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("INSERT OK [%s]\n\n", sql);

    sprintf(sql, "INSERT TAT_DB_TEST (TEST, TEST_NAME) VALUES ('B', '12345')");
    if((cnt = db->Execute(sql, strlen(sql))) <= 0)
    {
        printf("Execute Fail [%d] [%s] [%d:%s]\n", cnt, sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("INSERT OK [%s]\n\n", sql);


	// SELECT
    sprintf(sql, "SELECT * FROM TAT_DB_TEST");
    if(db->Query(sql, strlen(sql)) < 0)
    {
        printf("---- Query Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("SELECT OK [%s]\n\n", sql);

    char    test_id[32];
    char    test[32];
    char    prc_date[32];
    char    test_name[32];

    FetchMaria   fdata;

    fdata.Clear();

    fdata.Set(test_id,      sizeof(test_id));
    fdata.Set(test,         sizeof(test));
    fdata.Set(prc_date,     sizeof(prc_date));
    fdata.Set(test_name,    sizeof(test_name));

    while(true)
    {
        if(fdata.Fetch(db) == false)
            break;

        callfunc(db);

        printf(" [%s] [%s] [%s] [%s]\n\n", test_id, test, prc_date, test_name);
    }

	// UPDATE
    sprintf(sql, "UPDATE TAT_DB_TEST SET TEST='B', PRC_DATE=NOW() WHERE TEST_NAME='ABCDEFG'");
    if((cnt = db->Execute(sql, strlen(sql))) <= 0)
    {
        printf("Execute Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("UPDATE OK [%d] [%s]\n\n", cnt, sql);

	// SELECT
    sprintf(sql, "SELECT * FROM TAT_DB_TEST");
    if(db->Query(sql, strlen(sql)) < 0)
    {
        printf("Query Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("SELECT OK [%s]\n\n", sql);

    while(true)
    {
        if(fdata.Fetch(db) == false)
            break;

        printf(" [%s] [%s] [%s] [%s]\n\n", test_id, test, prc_date, test_name);
    }

	// DELETE
    sprintf(sql, "DELETE FROM TAT_DB_TEST WHERE TEST_NAME='ABCDEFG'");
    if((cnt = db->Execute(sql, strlen(sql))) <= 0)
    {
        printf("Execute Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("DELETE OK [%d] [%s]\n\n", cnt, sql);

	// SELECT
    sprintf(sql, "SELECT * FROM TAT_DB_TEST");
    if(db->Query(sql, strlen(sql)) < 0)
    {
        printf("Query Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("SELECT OK [%s]\n\n", sql);

	// DROP TABLE
    sprintf(sql, "DROP TABLE TAT_DB_TEST");
    if((cnt = db->Execute(sql, strlen(sql))) < 0)
    {
        printf("Execute Fail [%s] [%d:%s]\n", sql, db->GetError(), db->GetErrorMsg());
        delete db;
        return 0;
    }
    else
        printf("DROP OK [%d] [%s]\n\n", cnt, sql);
	--*/

	// DELETE
    delete db;

    return 0;
}
