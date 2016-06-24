#include "BJM_Define.hpp"
#include "CBatchHist.hpp"
#include <ctype.h>
#include <cstring>

extern CFileLog* g_pcLog;

CBatchHist::CBatchHist()
{
    m_pDB = NULL;
}

CBatchHist::~CBatchHist()
{
    if(m_pDB != NULL)
    {
//		delete m_pDB;
	    m_pDB = NULL;
	}
}

bool CBatchHist::Init(DB * a_pDB)
{
    m_pDB = a_pDB;

	return (m_pDB != NULL)?BJM_OK:BJM_NOK;
}

char* CBatchHist::GetPrcdate(ST_BatchJob *a_batchJob)
{
    char            selectsql[1024];
    char        	hresult[20 + 1];
	int				nRet = 0;

    g_pcLog->DEBUG("CBatchHist Select");

    sprintf(selectsql, "SELECT "
						"DATE_FORMAT(PRC_DATE, '%%Y%%m%%d%%H%%m') "
						"FROM TAT_BATCH_HIST "
						"WHERE GROUP_NAME = '%s' "
						"AND JOB_NAME = '%s' "
						"AND PKG_NAME = '%s' "
						"LIMIT 1", 
						a_batchJob->group_name, a_batchJob->job_name, a_batchJob->pkg_name);

    g_pcLog->DEBUG("Query [%s]", selectsql);

    FetchMaria  f;
	nRet = m_pDB->Query(&f, selectsql, strlen(selectsql));
	if(nRet < 0)
    {
        g_pcLog->ERROR("Query Fail [%s] [%d:%s]\n", selectsql, nRet, m_pDB->GetErrorMsg(nRet));
        return NULL;
    }


    memset(hresult, 0, 21);

	f.Set(hresult, sizeof(hresult));

	while(true)
	{
	    if(f.Fetch() == BJM_NOK)
    	{
        	break;
	    }

		strcpy(m_strPrc_date, hresult);
	}
    
    return m_strPrc_date;
}

int	CBatchHist::HistUpdate(DB * a_pDB, ST_BATCHHIST *a_batchHist)
{
	m_pDB = a_pDB;
    g_pcLog->DEBUG("CBatchHist Update");

    char            updatesql[1024];
    int             nHistCnt = 0;
	ST_BATCHHIST stbatchHist;
	stbatchHist = *a_batchHist;

    sprintf(updatesql, "UPDATE TAT_BATCH_HIST SET "
						"END_DATE = NOW(), "
						"EXIT_CD = %d, "
						"SUCCESS_YN = '%s' "
						"WHERE GROUP_NAME = '%s' "
						"AND JOB_NAME = '%s' "
						"AND PRC_DATE = '%s' "
						, stbatchHist.exit_cd, stbatchHist.success_yn, stbatchHist.group_name 
						, stbatchHist.job_name, stbatchHist.prc_date);
						
    g_pcLog->DEBUG("UPDATE [%p][%s]\n\n", m_pDB, updatesql);

    if((nHistCnt = m_pDB->Execute(updatesql, strlen(updatesql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", updatesql, nHistCnt, m_pDB->GetErrorMsg(nHistCnt));
        return BJM_NOK;
    }
    else
        g_pcLog->DEBUG("UPDATE OK [%p][%d] [%s]\n\n", m_pDB, nHistCnt, updatesql);

    return BJM_OK;

}

int	CBatchHist::HistInsert(ST_BATCHHIST *a_batchHist)
{
    g_pcLog->DEBUG("CBatchHist Insert");

    char            insertsql[1024];
    int             nHistCnt = 0;
	ST_BATCHHIST stbatchHist;
	stbatchHist = *a_batchHist;

    sprintf(insertsql, "INSERT TAT_BATCH_HIST "
					   "(GROUP_NAME, JOB_NAME, PRC_DATE, PKG_NAME, PROC_NAME, NODE_NAME, EXIT_CD) "
					   "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', %d)", 
						stbatchHist.group_name, 
						stbatchHist.job_name, 
						stbatchHist.prc_date, 
						stbatchHist.pkg_name, 
						stbatchHist.proc_name, 
						stbatchHist.node_name, 
						stbatchHist.exit_cd);

    if((nHistCnt = m_pDB->Execute(insertsql, strlen(insertsql))) <= 0)
    {
        g_pcLog->ERROR("Fail to execute [%s] [%d:%s]\n", insertsql, nHistCnt, m_pDB->GetErrorMsg(nHistCnt));
        return BJM_NOK;
    }
    else
        g_pcLog->DEBUG("INSERT OK [%p][%s]\n\n", m_pDB, insertsql);

    return BJM_OK;

}

int	CBatchHist::HistDelete()
{
    g_pcLog->INFO("CBatchHist Delete");
    return BJM_OK;

}

int CBatchHist::LoadHistInfo(ST_BATCHHIST *a_batchHist)
{
	g_pcLog->INFO("CBatchHist Init");
	int	 nRet = 0;
	int	 nbatchHistCount = 0;
    char query[1024];

	char hgroup_name			[BJM_HIST_GROUP_NAME_SIZE + 1];
	char hjob_name				[BJM_HIST_JOB_NAME_SIZE + 1];
	char hprc_date				[BJM_HIST_PRC_DATE_SIZE + 1];
	char hpkg_name				[BJM_HIST_PKG_NAME_SIZE + 1];
	char hend_date				[BJM_HIST_END_DATE_SIZE + 1];
	char hproc_name				[BJM_HIST_PROC_NAME_SIZE + 1];
	char hnode_name				[BJM_HIST_NODE_NAME_SIZE + 1];
	char hexit_cd				[12];
	char hsuccess_yn			[2];

	ST_BATCHHIST				*batchHistPtr;

	sprintf(query, "SELECT "
					"GROUP_NAME, "
				    "JOB_NAME, "
				    "PRC_DATE, "
					"PKG_NAME, "
					"END_DATE, "
					"PROC_NAME, "
					"NODE_NAME, "
					"EXIT_CD, "
					"SUCCESS_YN "
					"FROM TAT_BATCH_HIST "
					"ORDER BY GROUP_NAME");

    if(m_pDB == NULL)
    {
        g_pcLog->WARNING("DB table is empty");
        return BJM_NOK;
    }

    FetchMaria  f_mysql;
	nRet = m_pDB->Query(&f_mysql, query, strlen(query));
	if(nRet < 0)
    {   
        g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }


    // variable inialize
    memset(hgroup_name, 0, BJM_HIST_GROUP_NAME_SIZE +1);
    memset(hjob_name, 0, BJM_HIST_JOB_NAME_SIZE +1);
    memset(hprc_date, 0, BJM_HIST_PRC_DATE_SIZE +1);
    memset(hpkg_name, 0, BJM_HIST_PKG_NAME_SIZE +1);
    memset(hend_date, 0, BJM_HIST_END_DATE_SIZE +1);
    memset(hproc_name, 0, BJM_HIST_PROC_NAME_SIZE +1);
    memset(hnode_name, 0, BJM_HIST_NODE_NAME_SIZE +1);
    memset(hexit_cd, 0, 12);
    memset(hsuccess_yn, 0, 2);

    //input local variable
    f_mysql.Set(hgroup_name, sizeof(hgroup_name));
    f_mysql.Set(hjob_name, sizeof(hjob_name));
    f_mysql.Set(hprc_date, sizeof(hprc_date));
    f_mysql.Set(hpkg_name, sizeof(hpkg_name));
    f_mysql.Set(hend_date, sizeof(hend_date));
    f_mysql.Set(hproc_name, sizeof(hproc_name));
    f_mysql.Set(hnode_name, sizeof(hnode_name));
    f_mysql.Set(hexit_cd, sizeof(hexit_cd));
    f_mysql.Set(hsuccess_yn, sizeof(hsuccess_yn));

	while(true)
	{
        if(f_mysql.Fetch() == BJM_NOK)
        {
            break;
        }

        // input struct
        batchHistPtr = a_batchHist + nbatchHistCount;

        strcpy(batchHistPtr->group_name, hgroup_name);
        strcpy(batchHistPtr->job_name, hjob_name);
        strcpy(batchHistPtr->prc_date, hprc_date);
        strcpy(batchHistPtr->pkg_name, hpkg_name);
        strcpy(batchHistPtr->end_date, hend_date);
        strcpy(batchHistPtr->proc_name, hproc_name);
        strcpy(batchHistPtr->node_name, hnode_name);
        batchHistPtr->exit_cd = atoi(hexit_cd);
        strcpy(batchHistPtr->success_yn, hsuccess_yn);

        g_pcLog->INFO("%s, %d, %d, %s, %s, %s, %d, %s"
												   , batchHistPtr->group_name
                                                   , batchHistPtr->job_name
                                                   , batchHistPtr->prc_date
                                                   , batchHistPtr->pkg_name
                                                   , batchHistPtr->end_date
                                                   , batchHistPtr->proc_name
                                                   , batchHistPtr->node_name
                                                   , batchHistPtr->exit_cd
                                                   , batchHistPtr->success_yn);

        nbatchHistCount++;
	}

    return BJM_OK;
}
