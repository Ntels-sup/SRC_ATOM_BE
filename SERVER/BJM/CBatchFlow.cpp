#include "BJM_Define.hpp"
#include "CBatchFlow.hpp"

extern CFileLog* g_pcLog;

CBatchFlow::CBatchFlow()
{
    m_pDB = NULL;
}

CBatchFlow::~CBatchFlow()
{
	if(m_pDB != NULL)
	{
		delete m_pDB;
	    m_pDB = NULL;
	}
}

bool CBatchFlow::Init(DB * a_pDB)
{
    m_pDB = a_pDB;

    return (m_pDB != NULL)?BJM_OK:BJM_NOK;
}

int CBatchFlow::LoadFlowInfo(ST_BATCHFLOW *_batchFlow)
{
    g_pcLog->DEBUG("CBatchFlow Select");

	int nRet = 0;
	char query				[256];
	char hflow_no			[12];
	char hjob_name			[BJM_FLOW_JOB_NAME_SIZE + 1];
	char hnext_job_name		[BJM_FLOW_JOB_NAME_SIZE + 1];
	char hexit_cd			[12];
	int nbatchFlowCount 	= 0;

	ST_BATCHFLOW			*batchFlowPtr;

	sprintf(query, "SELECT FLOW_NO, "
				   "JOB_NAME, "
				   "NEXT_JOB_NAME, "
				   "EXIT_CD "
				   "FROM TAT_BATCH_FLOW");

	if(m_pDB == NULL)
    {
        g_pcLog->WARNING("DB table is empty");
        return BJM_NOK;
    }

	g_pcLog->DEBUG("[%s]", query);

    FetchMaria  f_mysql;

    nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }


    // variable inialize
    memset(hflow_no, 0, 12);
    memset(hjob_name, 0, BJM_FLOW_JOB_NAME_SIZE +1);
    memset(hnext_job_name, 0, BJM_FLOW_JOB_NAME_SIZE +1);
    memset(hexit_cd, 0, 12);

    //input local variable
    f_mysql.Set(hflow_no, sizeof(hflow_no));
    f_mysql.Set(hjob_name, sizeof(hjob_name));
    f_mysql.Set(hnext_job_name, sizeof(hnext_job_name));
    f_mysql.Set(hexit_cd, sizeof(hexit_cd));

    while(true)
    {
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }

        // input struct
        batchFlowPtr = _batchFlow + nbatchFlowCount;

        batchFlowPtr->flow_no = atoi(hflow_no);
        strncpy(batchFlowPtr->job_name, hjob_name, sizeof(hjob_name));
        strncpy(batchFlowPtr->next_job_name, hnext_job_name, sizeof(hnext_job_name));
        batchFlowPtr->exit_cd = atoi(hexit_cd);

        g_pcLog->DEBUG("%d, %s, %s, %d"
                                                   , batchFlowPtr->flow_no
                                                   , batchFlowPtr->job_name
                                                   , batchFlowPtr->next_job_name
                                                   , batchFlowPtr->exit_cd);

		nbatchFlowCount++;
	}

	return nbatchFlowCount;
}

int	CBatchFlow::FlowSelect(ST_MULTI_JOB_FLOW *a_MultiFlow)
{
    g_pcLog->DEBUG("CBatchFlow FlowSelect");

	int nRet = 0;
	char query				[256];
//	char hjob_name			[BJM_FLOW_JOB_NAME_SIZE + 1];
	char hnext_job_name		[BJM_FLOW_JOB_NAME_SIZE + 1];
	char hexit_cd			[12];
	int nMultiFlowCount 	= 0;

	ST_MULTI_JOB_FLOW			*MultiFlowPtr;

	sprintf(query, "SELECT "
				   "distinct(EXIT_CD), "
//				   "JOB_NAME, "
				   "NEXT_JOB_NAME "
				   "FROM TAT_BATCH_FLOW "
				   "WHERE EXIT_CD != 0 AND NEXT_JOB_NAME IS NOT NULL");

	if(m_pDB == NULL)
    {
        g_pcLog->WARNING("DB table is empty");
        return BJM_NOK;
    }

	g_pcLog->DEBUG("[%s]", query);

    FetchMaria  f_mysql;

    nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }

	// variable inialize
    memset(hexit_cd, 0, 12);
//    memset(hjob_name, 0, BJM_FLOW_JOB_NAME_SIZE +1);
    memset(hnext_job_name, 0, BJM_FLOW_JOB_NAME_SIZE +1);

    //input local variable
    f_mysql.Set(hexit_cd, sizeof(hexit_cd));
//    f_mysql.Set(hjob_name, sizeof(hjob_name));
    f_mysql.Set(hnext_job_name, sizeof(hnext_job_name));

    while(true)
    {
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }

        // input struct
        MultiFlowPtr = a_MultiFlow + nMultiFlowCount;

        MultiFlowPtr->exit_cd = atoi(hexit_cd);
//        strncpy(MultiFlowPtr->job_name, hjob_name, sizeof(hjob_name));
        strncpy(MultiFlowPtr->next_job_name, hnext_job_name, sizeof(hnext_job_name));

        g_pcLog->DEBUG("%s, %d"
//                                                   , MultiFlowPtr->job_name
                                                   , MultiFlowPtr->next_job_name
                                                   , MultiFlowPtr->exit_cd);

		nMultiFlowCount++;
	}

	return nMultiFlowCount;
}

int CBatchFlow::GetMultiFlowCount(int a_nExit_cd)
{
    g_pcLog->DEBUG("CBatchFlow FlowCount");

	int nRet = 0;
	char query				[256];
	char hCount			[12];
	int nMultiFlowCount 	= 0;

	sprintf(query, "select count(*) from TAT_BATCH_FLOW WHERE EXIT_CD=%d", a_nExit_cd);
	
	if(m_pDB == NULL)
    {
        g_pcLog->WARNING("DB table is empty");
        return BJM_NOK;
    }

	g_pcLog->DEBUG("[%s]", query);

    FetchMaria  f_mysql;

    nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }

	// variable inialize
    memset(hCount, 0, 12);

    //input local variable
    f_mysql.Set(hCount, sizeof(hCount));

    while(true)
    {
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }

        // input struct
		nMultiFlowCount = atoi(hCount);

	}

	return nMultiFlowCount;
}

int CBatchFlow::FlowCount()
{
	int nCount = 0;


	return nCount;
}

int	CBatchFlow::FlowUpdate()
{
    g_pcLog->INFO("CBatchFlow Update");

    char            updatesql[1024];
    int             cnt = 0;

    sprintf(updatesql, "UPDATE TAT_BATCH_FLOW SET GROUP_NAME='WEEKLY_BACKUP_GROUP' WHERE GROUP_NAME='WEEKLY_BACKUP_GRP'");
    if((cnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", updatesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("UPDATE OK [%d] [%s]\n\n", cnt, updatesql);

    return BJM_OK;

}

int	CBatchFlow::FlowDelete()
{
    g_pcLog->INFO("CBatchFlow Delete");

    char        deletesql[1024];
    int             cnt = 0;

    sprintf(deletesql, "DELETE FROM TAT_BATCH_FLOW WHERE GROUP_NAME='WEEKLY_BACKUP_GROUP'");
    if((cnt = m_pDB->Execute(deletesql, strlen(deletesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", deletesql, cnt, m_pDB->GetErrorMsg(cnt));
        return BJM_NOK;
    }
    else
        g_pcLog->INFO("DELETE OK [%d] [%s]\n\n", cnt, deletesql);

    return BJM_OK;

}

int	CBatchFlow::FlowInsert()
{
    g_pcLog->INFO("CBatchFlow Insert");

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


