#include "BJM_Define.hpp"
#include "CBatchJob.hpp"
#include <cstring>

extern CFileLog* g_pcLog;


CBatchJob::CBatchJob()
{
    m_pDB = NULL;
}

CBatchJob::~CBatchJob()
{
    if(NULL != m_pDB)
    {
		delete m_pDB;
	    m_pDB = NULL;
	}
}

bool CBatchJob::Init(DB * a_pDB)
{
    m_pDB = a_pDB;

	return (m_pDB != NULL)?BJM_OK:BJM_NOK;
}

int CBatchJob::LoadJobInfo(ST_BatchGroup *batchgroup_, ST_BatchJob *_batchjob)
{
	g_pcLog->DEBUG("CBatchJob Init");
	int nRet = 0;
    char                        query[256];

	char hjob_name				[BJM_JOB_JOB_NAME_SIZE + 1];
	char hgroup_name			[BJM_JOB_GROUP_NAME_SIZE +1];
	char hpkg_name				[40 + 1];
	char hrootjob_yn			[2];
	char hnode_no				[11 + 1];
	char hnode_type				[40 + 1];
	char hproc_no				[11 + 1];
	char hfail_cont_yn			[2];
	char hrun_yn				[2];
	char hdescription			[BJM_JOB_DESC_SIZE +1];

	ST_BatchJob					*batchJobPtr;
	ST_BatchGroup				stBatchGroup;

	stBatchGroup 				= *batchgroup_;
	int							nbatchJobCount = 0;

	sprintf(query,  "SELECT "
					"JOB_NAME, "
					"GROUP_NAME, "
					"PKG_NAME, "
					"ROOTJOB_YN, "
					"NODE_NO, "
					"NODE_TYPE, "
					"PROC_NO, "
					"FAIL_CONT_YN, "
					"RUN_YN, "
					"DESCRIPTION "
					"FROM TAT_BATCH_JOB "
					"WHERE "
//					"ROOTJOB_YN = 'Y' AND "
					"RUN_YN = 'Y' "
					"AND GROUP_NAME = '%s' "
					"AND PKG_NAME = '%s' "
					"ORDER BY GROUP_NAME", stBatchGroup.group_name, stBatchGroup.pkg_name);

    if(NULL == m_pDB)
    {
        g_pcLog->INFO("DB table is empty");
        return BJM_NOK;
    }

    g_pcLog->DEBUG("Query [%s]", query);

    FetchMaria  f_mysql;

    nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {   
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }

    //f_mysql.Clear();

    // variable inialize
    memset(hjob_name, 0, BJM_JOB_JOB_NAME_SIZE +1);
    memset(hgroup_name, 0, BJM_JOB_GROUP_NAME_SIZE +1);
    memset(hpkg_name, 0, BJM_JOB_PKG_NAME_SIZE + 1);
    memset(hrootjob_yn, 0, 2);
    memset(hnode_no, 0, 12);
    memset(hnode_type, 0, 40 + 1);
    memset(hproc_no, 0, BJM_JOB_PROC_NAME_SIZE +1);
    memset(hfail_cont_yn, 0, 2);
    memset(hrun_yn, 0, 2);
    memset(hdescription, 0, BJM_JOB_DESC_SIZE +1);

    //input local variable
    f_mysql.Set(hjob_name, sizeof(hjob_name));
    f_mysql.Set(hgroup_name, sizeof(hgroup_name));
    f_mysql.Set(hpkg_name, sizeof(hpkg_name));
    f_mysql.Set(hrootjob_yn, sizeof(hrootjob_yn));
    f_mysql.Set(hnode_no, sizeof(hnode_no));
    f_mysql.Set(hnode_type, sizeof(hnode_type));
    f_mysql.Set(hproc_no, sizeof(hproc_no));
    f_mysql.Set(hfail_cont_yn, sizeof(hfail_cont_yn));
    f_mysql.Set(hrun_yn, sizeof(hrun_yn));
    f_mysql.Set(hdescription, sizeof(hdescription));

	while(true)
	{
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }

        // input struct
        batchJobPtr = _batchjob + nbatchJobCount;

        strncpy(batchJobPtr->job_name, hjob_name, sizeof(hjob_name));
        strncpy(batchJobPtr->group_name, hgroup_name, sizeof(hgroup_name));
        strncpy(batchJobPtr->pkg_name, hpkg_name, sizeof(hpkg_name));
        strncpy(batchJobPtr->rootjob_yn, hrootjob_yn, sizeof(hrootjob_yn));
        batchJobPtr->node_no = atoi(hnode_no);
        strncpy(batchJobPtr->node_type, hnode_type, sizeof(hnode_type)); 
        batchJobPtr->proc_no = atoi(hproc_no);
        strncpy(batchJobPtr->fail_cont_yn, hfail_cont_yn, sizeof(hfail_cont_yn));
        strncpy(batchJobPtr->run_yn, hrun_yn, sizeof(hrun_yn));
        strncpy(batchJobPtr->description, hdescription, sizeof(hdescription));

        g_pcLog->DEBUG("%s, %s, %s, %s, %d, %s, %d, %s, %s, %s"
                                                   , batchJobPtr->job_name
												   , batchJobPtr->group_name
												   , batchJobPtr->pkg_name
												   , batchJobPtr->rootjob_yn
                                                   , batchJobPtr->node_no
                                                   , batchJobPtr->node_type
												   , batchJobPtr->proc_no
                                                   , batchJobPtr->fail_cont_yn
                                                   , batchJobPtr->run_yn
                                                   , batchJobPtr->description);


        nbatchJobCount++;		
	}

    return nbatchJobCount;

}

int	CBatchJob::JobUpdate()
{
    g_pcLog->INFO("CBatchJob Update");

    char            updatesql[1024];
    int             cnt = 0;

    sprintf(updatesql, "UPDATE TAT_BATCH_CMD SET GROUP_NAME='WEEKLY_BACKUP_GROUP', START_DATE=NOW() WHERE GROUP_NAME='WEEKLY_BACKUP_GRP'");
    if((cnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", updatesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("UPDATE OK [%d] [%s]\n\n", cnt, updatesql);

    return BJM_OK;

}

int	CBatchJob::JobDelete()
{
    g_pcLog->INFO("CBatchJob Delete");

    char        deletesql[1024];
    int             cnt = 0;

    sprintf(deletesql, "DELETE FROM TAT_BATCH_CMD WHERE GROUP_NAME='WEEKLY_BACKUP_GROUP'");
    if((cnt = m_pDB->Execute(deletesql, strlen(deletesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", deletesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("DELETE OK [%d] [%s]\n\n", cnt, deletesql);

    return BJM_OK;

}

int	CBatchJob::JobInsert()
{
    g_pcLog->INFO("CBatchJob Insert");

    char            insertsql[1024];
    int             cnt = 0;

    sprintf(insertsql, "INSERT TAT_BATCH_JOB (GROUP_NAME, CREATE_DATE, START_DATE, SCHEDULE_CYCLE_TYPE, SCHEDULE_CYCLE, EXPIRE_TIME) VALUES ('WEEKLY_BACKUP_GRP', '20160403 121131', '20160404 133011', '01', 30, '20161232359', '')");
    if((cnt = m_pDB->Execute(insertsql, strlen(insertsql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%d] [%s] [%d:%s]\n", cnt, insertsql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("INSERT OK [%s]\n\n", insertsql);

    return BJM_OK;

}

