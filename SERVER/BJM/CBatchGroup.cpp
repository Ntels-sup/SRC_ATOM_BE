#include "BJM_Define.hpp"
#include "CBatchGroup.hpp"
#include "CATMDate.hpp"
#include "CATMTime.hpp"
#include <string>
#include <stdlib.h>
#include <vector>

using namespace std;
//extern symbol
extern CFileLog* g_pcLog;

CBatchGroup::CBatchGroup()
{
	m_pDB = NULL;
}

CBatchGroup::~CBatchGroup()
{
    if(m_pDB != NULL)
    {
		delete m_pDB;
		m_pDB = NULL;
	}
}

bool CBatchGroup::Init(DB * a_pDB)
{
	m_pDB = a_pDB;

	return (m_pDB != NULL)?BJM_OK:BJM_NOK;	
}

int CBatchGroup::LoadGroupInfo(ST_BatchGroup *a_batchGroups)
{
	g_pcLog->DEBUG("CBatchGroup Init");
	int  nRet = 0;
	char 						 query[512];

    char hgroup_name             [BJM_GROUP_GROUP_NAME_SIZE +1];
    char hpkg_name 	             [BJM_GROUP_PKG_NAME_SIZE +1];
    char hcreate_date            [BJM_GROUP_CREATE_DATE_SIZE +1];
    char hstart_date             [BJM_GROUP_START_DATE_SIZE  +1];
    char hschedule_cycle_type    [BJM_GROUP_SCHEDULE_CYCLE_TYPE_SIZE +1];
    char hnschedule_cycle		 [256];
    char huse_yn				 [2];
    char hexpire_date            [BJM_GROUP_EXPIRE_DATE_SIZE +1];
	char huser_id				 [BJM_GROUP_USER_ID_SIZE +1];
    char hdescription            [BJM_GROUP_DESC_SIZE +1];
	int  nschedule_cycle;

	ST_BatchGroup				 *batchGroupPtr;		// batchGroup 조회 정보
	int							 nbatchGroupCount = 0; // GroupCount

	sprintf(query, "SELECT "
				   "GROUP_NAME, "
				   "PKG_NAME, "
				   "DATE_FORMAT(CREATE_DATE, '%s'), "
				   "DATE_FORMAT(START_DATE, '%s'), "
				   "SCHEDULE_CYCLE_TYPE, "
				   "SCHEDULE_CYCLE, "
				   "USE_YN, "
				   "DATE_FORMAT(EXPIRE_DATE, '%s'), "
				   "USER_ID, "
				   "DESCRIPTION "
				   "FROM TAT_BATCH_GROUP "
				   "WHERE START_DATE <=  DATE_FORMAT(NOW(), '%s') "
				   "AND EXPIRE_DATE >= DATE_FORMAT(NOW(), '%s' ) "				  
				   "ORDER BY GROUP_NAME", 
				   "%Y%m%d%H%m%s", 
				   "%Y%m%d%H%m%s", 
				   "%Y%m%d%H%m%s", 
				   "%Y%m%d%H%m%s", 
				   "%Y%m%d%H%m%s"
				   );

	if(NULL == m_pDB)
	{
		g_pcLog->INFO("DB table is empty");
		return BJM_NOK;
	}

	g_pcLog->DEBUG("Query [%s]", query);

	FetchMaria	f_mysql;

	nRet = m_pDB->Query(&f_mysql, query, strlen(query));

	if(nRet < 0)
	{	
		g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
		return BJM_NOK;
	}

	// variable inialize
	memset(hgroup_name, 0, BJM_GROUP_GROUP_NAME_SIZE +1);
	memset(hpkg_name, 0, BJM_GROUP_PKG_NAME_SIZE +1);
	memset(hcreate_date, 0, BJM_GROUP_CREATE_DATE_SIZE +1);
	memset(hstart_date, 0, BJM_GROUP_START_DATE_SIZE +1);
	memset(hschedule_cycle_type, 0, BJM_GROUP_SCHEDULE_CYCLE_TYPE_SIZE +1);
	nschedule_cycle = 0;
	memset(huse_yn, 0, 2);
	memset(hexpire_date, 0, BJM_GROUP_EXPIRE_DATE_SIZE +1);
	memset(huser_id, 0, BJM_GROUP_USER_ID_SIZE +1);
	memset(hdescription, 0, BJM_GROUP_DESC_SIZE +1);

    //input local variable
	f_mysql.Set(hgroup_name, sizeof(hgroup_name));
	f_mysql.Set(hpkg_name, sizeof(hpkg_name));
	f_mysql.Set(hcreate_date, sizeof(hcreate_date));
	f_mysql.Set(hstart_date, sizeof(hstart_date));
	f_mysql.Set(hschedule_cycle_type, sizeof(hschedule_cycle_type));
	f_mysql.Set(hnschedule_cycle, 256);
	f_mysql.Set(huse_yn, sizeof(huse_yn));
	f_mysql.Set(hexpire_date, sizeof(hexpire_date));
	f_mysql.Set(huser_id, sizeof(huser_id));
	f_mysql.Set(hdescription, sizeof(hdescription));

	while(true)
	{
		if(f_mysql.Fetch() == BJM_NOK)
		{
			break;
		}

		// input struct
		batchGroupPtr = a_batchGroups + nbatchGroupCount;
		
		strcpy(batchGroupPtr->group_name, hgroup_name);
		strcpy(batchGroupPtr->pkg_name, hpkg_name);
		strcpy(batchGroupPtr->create_date, hcreate_date);
		strcpy(batchGroupPtr->start_date, hstart_date); 
		strcpy(batchGroupPtr->schedule_cycle_type, hschedule_cycle_type); 
		batchGroupPtr->nschedule_cycle = atoi(hnschedule_cycle);
		strcpy(batchGroupPtr->use_yn, huse_yn);
		strcpy(batchGroupPtr->expire_date, hexpire_date);
		strcpy(batchGroupPtr->user_id, huser_id);
		strcpy(batchGroupPtr->description, hdescription);

		g_pcLog->DEBUG("%s, %s, %s, %s, %s, %d, %s, %s, %s, %s", batchGroupPtr->group_name 
												   , batchGroupPtr->pkg_name
												   , batchGroupPtr->create_date
												   , batchGroupPtr->start_date
												   , batchGroupPtr->schedule_cycle_type
												   , batchGroupPtr->nschedule_cycle
												   , batchGroupPtr->use_yn
												   , batchGroupPtr->expire_date
												   , batchGroupPtr->user_id
												   , batchGroupPtr->description);


		nbatchGroupCount++;

    }

	return nbatchGroupCount;
}

char* CBatchGroup::GetGroupName(char * a_strJobName)
{
    g_pcLog->DEBUG("CBatchGroup GetGroupName");

	char 			strSql[256];
	int 			nRet = 0;
	char 			hGroup_name          [BJM_GROUP_GROUP_NAME_SIZE +1];
	char* 			strGroup_name = NULL;

    sprintf(strSql, "SELECT GROUP_NAME FROM TAT_BATCH_JOB WHERE JOB_NAME = '%s'", a_strJobName);

	if(m_pDB == NULL)
    {
        g_pcLog->INFO("DB table is empty");
        return BJM_NOK;
    }

    g_pcLog->DEBUG("[%s]", strSql);

    FetchMaria  f_mysql;

    nRet = m_pDB->Query(&f_mysql, strSql, strlen(strSql));
    if(nRet < 0)
    {
		g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
		return BJM_NOK;
    }

    // variable inialize
    memset(hGroup_name, 0, BJM_GROUP_GROUP_NAME_SIZE +1);

    //input local variable
    f_mysql.Set(hGroup_name, sizeof(hGroup_name));

    while(true)
    {
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }
	
		strGroup_name = hGroup_name;
    }	

    return strGroup_name;
}

// update
int CBatchGroup::GroupUpdate()
{
    g_pcLog->INFO("CBatchGroup Update");
	
	char 			updatesql[256];
	int 			cnt = 0;

    sprintf(updatesql, "UPDATE TAT_BATCH_JOB SET GROUP_NAME='WEEKLY_BACKUP_GROUP', START_DATE=NOW() WHERE GROUP_NAME='WEEKLY_BACKUP_GRP'");
    if((cnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]", updatesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("UPDATE OK [%d] [%s]", cnt, updatesql);


    return BJM_OK;
}

// delete
int CBatchGroup::GroupDelete()
{
    g_pcLog->INFO("CBatchGroup Delete");

	char 		deletesql[256];
	int 			cnt = 0;

    sprintf(deletesql, "DELETE FROM TAT_BATCH_JOB WHERE GROUP_NAME='WEEKLY_BACKUP_GRP'");
    if((cnt = m_pDB->Execute(deletesql, strlen(deletesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]", deletesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("DELETE OK [%d] [%s]", cnt, deletesql);

    return BJM_OK;
}

// insert
int CBatchGroup::GroupInsert()
{
    g_pcLog->INFO("CBatchGroup Insert");

	char 			insertsql[256];
	int 			cnt = 0;

    sprintf(insertsql, "INSERT TAT_BATCH_JOB (GROUP_NAME, CREATE_DATE, START_DATE, SCHEDULE_CYCLE_TYPE, SCHEDULE_CYCLE, EXPIRE_TIME, DESCRIPTION) VALUES ('WEEKLY_BACKUP_GRP', '20160403121131', '20160404133011', '01', 25, '20161231132359', '')");
    if((cnt = m_pDB->Execute(insertsql, strlen(insertsql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%d] [%s] [%d:%s]\n", cnt, insertsql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("INSERT OK [%s]\n\n", insertsql);

    return BJM_OK;
}

