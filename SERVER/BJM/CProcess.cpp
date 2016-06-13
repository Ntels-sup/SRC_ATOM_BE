#include "BJM_Define.hpp"
#include "CProcess.hpp"
#include <cstring>

//extern symbol
extern CFileLog* g_pcLog;

CProcess::CProcess()
{
	m_pDB = NULL;
}

CProcess::~CProcess()
{
	if(m_pDB != NULL)
	{
		delete m_pDB;
		m_pDB = NULL;
	}
}

bool CProcess::Init(DB * a_pDB)
{
	m_pDB = a_pDB;

	return (m_pDB != NULL)?true:false;	
}

int CProcess::LoadProcInfo(ST_BatchJob *a_batchjob, ST_PROCESS *a_process)
{
	g_pcLog->DEBUG("CProcess LoadProcInfo");
	char query[256];
	int	 nProcessCnt = 0;

	char hproc_no				 [11];
    char hpkg_name             	 [BJM_PROC_PKG_NAME_SIZE +1];
    char hproc_name           	 [BJM_PROC_PROC_NAME_SIZE +1];
    char hnode_type            	 [BJM_PROC_NODE_TYPE_SIZE +1];
	char hsvc_no				 [11];
    char hexec_bin       	     [BJM_PROC_EXEC_BIN_SIZE +1];
    char hexec_arg       	     [BJM_PROC_EXEC_BIN_SIZE +1];
    char hexec_env       	     [BJM_PROC_EXEC_BIN_SIZE +1];
	char huse_yn				 [2];
    char hproc_type				 [2];
	char hnode_name				 [BJM_PROC_NODE_NAME_SIZE +1];
	int  nRet = 0;

	ST_PROCESS					*stProcessPtr;
	ST_BatchJob					stBatchJob;
	stBatchJob = *a_batchjob;

	sprintf(query, "SELECT "
							"PROC_NO, "
							"PKG_NAME, "
							"PROC_NAME, "
							"NODE_TYPE, "
							"SVC_NO, "
							"EXEC_BIN, "
							"EXEC_ARG, "
							"EXEC_ENV, "
							"USE_YN, "
							"PROC_TYPE, "
							"(SELECT NODE_NAME FROM TAT_NODE WHERE NODE_TYPE = '%s' AND PKG_NAME = '%s' AND NODE_NO = %d AND USE_YN= 'Y') AS NODE_NAME "
							"FROM TAT_PROCESS "
							"WHERE NODE_TYPE = '%s' AND PKG_NAME = '%s' AND PROC_NO = %d ORDER BY PKG_NAME"
							, stBatchJob.node_type, stBatchJob.pkg_name, stBatchJob.node_no
							, stBatchJob.node_type, stBatchJob.pkg_name, stBatchJob.proc_no);

	if(NULL == m_pDB)
	{
		g_pcLog->INFO("DB table is empty");
		return BJM_NOK;
	}
	
	g_pcLog->DEBUG("[%s]", query);

	FetchMaria	f;

	nRet = m_pDB->Query(&f, query, strlen(query));

	if(nRet < 0)
	{	
		g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
		return BJM_NOK;
	}


	memset(hproc_no, 0, 11);
    memset(hpkg_name, 0, BJM_PROC_PKG_NAME_SIZE +1);
	memset(hproc_name, 0, BJM_PROC_PROC_NAME_SIZE +1);
    memset(hnode_type, 0, BJM_PROC_NODE_TYPE_SIZE +1);
	memset(hsvc_no, 0, 11);
    memset(hexec_bin, 0, BJM_PROC_EXEC_BIN_SIZE +1);
    memset(hexec_arg, 0, BJM_PROC_EXEC_BIN_SIZE +1);
    memset(hexec_env, 0, BJM_PROC_EXEC_BIN_SIZE +1);
	memset(huse_yn, 0, 2);
	memset(hproc_type, 0, 2);
	memset(hnode_name, 0, BJM_PROC_NODE_NAME_SIZE +1);

   	f.Set(hproc_no,        sizeof(hproc_no));       
    f.Set(hpkg_name,       sizeof(hpkg_name));      
   	f.Set(hproc_name,      sizeof(hproc_name));     
    f.Set(hnode_type,      sizeof(hnode_type));     
   	f.Set(hsvc_no,         sizeof(hsvc_no));        
    f.Set(hexec_bin,       sizeof(hexec_bin));      
    f.Set(hexec_arg,       sizeof(hexec_arg));      
    f.Set(hexec_env,       sizeof(hexec_env));      
	f.Set(huse_yn,         sizeof(huse_yn));        
	f.Set(hproc_type,      sizeof(hproc_type));
	f.Set(hnode_name,      sizeof(hnode_name)); 

	while(true)
	{
		if(f.Fetch() == false)
			break;

		stProcessPtr = a_process + nProcessCnt;

	    stProcessPtr->proc_no = atoi(hproc_no);       
	    strncpy(stProcessPtr->pkg_name,       hpkg_name,       sizeof(hpkg_name));
    	strncpy(stProcessPtr->proc_name,      hproc_name,      sizeof(hproc_name));
	    strncpy(stProcessPtr->node_type,      hnode_type,      sizeof(hnode_type));
    	stProcessPtr->svc_no = atoi(hsvc_no);        
	    strncpy(stProcessPtr->exec_bin,       hexec_bin,       sizeof(hexec_bin));
	    strncpy(stProcessPtr->exec_arg,       hexec_arg,       sizeof(hexec_arg));
	    strncpy(stProcessPtr->exec_env,       hexec_env,       sizeof(hexec_env));
	    strncpy(stProcessPtr->use_yn,         huse_yn,         sizeof(huse_yn));
	    strncpy(stProcessPtr->proc_type,      hproc_type,      sizeof(hproc_type));
	    strncpy(stProcessPtr->node_name,      hnode_name,      sizeof(hnode_name));


		g_pcLog->DEBUG("%d, %s, %s, %s, %d, %s, %s, %s, %s, %s, %s", 
								stProcessPtr->proc_no, 
								stProcessPtr->pkg_name, 
								stProcessPtr->proc_name, 
								stProcessPtr->node_type,
                                stProcessPtr->svc_no,
                                stProcessPtr->exec_bin,
                                stProcessPtr->exec_arg,
                                stProcessPtr->exec_env,
                                stProcessPtr->use_yn,
                                stProcessPtr->proc_type,
								stProcessPtr->node_name);

    }

    return BJM_OK;
}

int CProcess::LoadCoworkNo(char* a_NodeNm, char* a_NmPrcNm, char* a_BjmPrcNm, char* a_WsmPrcNm, ST_COWORK_INFO *a_coworkinfo)
{
	g_pcLog->DEBUG("CProcess LoadCoworkNo");

	char strQuery[256];
	int	 nQueryCnt = 0;

    char hpkg_name              [BJM_HIST_PKG_NAME_SIZE +1];
    char hnode_type             [BJM_PROC_NODE_TYPE_SIZE +1];
    char hnode_no				[11];
    char hnm_proc_no			[11];
    char hbjm_proc_no			[11];
    char hwsm_proc_no			[11];
	int  nRet = 0;

	ST_COWORK_INFO *stCoworkInfoPtr;
	ST_COWORK_INFO stCoworkInfo;
	stCoworkInfo = *a_coworkinfo;

	sprintf(strQuery, "SELECT PKG_NAME, "
					  "NODE_TYPE, "
					  "NODE_NO, "
					  "(SELECT PROC_NO FROM TAT_PROCESS WHERE PROC_NAME = '%s') AS NM_PROC_NO, "
  					  "(SELECT PROC_NO FROM TAT_PROCESS WHERE PROC_NAME = '%s') AS BJM_PROC_NO, "
				      "(SELECT PROC_NO FROM TAT_PROCESS WHERE PROC_NAME = '%s') AS WSM_PROC_NO "
					  "FROM TAT_NODE "
					  "WHERE NODE_NAME = '%s' "
					  "AND USE_YN= 'Y'"
					 , a_NmPrcNm
					 , a_BjmPrcNm
					 , a_WsmPrcNm
					 , a_NodeNm);

	if(NULL == m_pDB)
	{
		g_pcLog->INFO("DB table is empty");
		return BJM_NOK;
	}
	
	g_pcLog->DEBUG("[%s]", strQuery);
	FetchMaria	f;

	nRet = m_pDB->Query(&f, strQuery, strlen(strQuery));
	if(nRet < 0)
	{	
		g_pcLog->ERROR("Query Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
		return BJM_NOK;
	}


   	memset(hpkg_name, 0, BJM_PROC_PKG_NAME_SIZE +1);
   	memset(hnode_type, 0, BJM_PROC_NODE_TYPE_SIZE +1);
    memset(hnode_no, 0, 11);
    memset(hnm_proc_no, 0, 11);
    memset(hbjm_proc_no, 0, 11);
    memset(hwsm_proc_no, 0, 11);

	f.Set(hpkg_name,       sizeof(hpkg_name));      
	f.Set(hnode_type,      sizeof(hnode_type));     
    f.Set(hnode_no,        sizeof(hnode_no));       
    f.Set(hnm_proc_no,     sizeof(hnm_proc_no));        
	f.Set(hbjm_proc_no,    sizeof(hbjm_proc_no));      
	f.Set(hwsm_proc_no,    sizeof(hwsm_proc_no));      

	while(true)
	{
	
		if(f.Fetch() == false)
			break;

		stCoworkInfoPtr = a_coworkinfo + nQueryCnt;

	    strncpy(stCoworkInfoPtr->pkg_name,       hpkg_name,       sizeof(hpkg_name));
	    strncpy(stCoworkInfoPtr->node_type,      hnode_type,      sizeof(hnode_type));
    	stCoworkInfoPtr->node_no 		= atoi(hnode_no);        
	    stCoworkInfoPtr->nm_proc_no 	= atoi(hnm_proc_no);       
	    stCoworkInfoPtr->bjm_proc_no 	= atoi(hbjm_proc_no);       
	    stCoworkInfoPtr->wsm_proc_no 	= atoi(hwsm_proc_no);

		nQueryCnt++;
	}
    return BJM_OK;
}

int CProcess::ProcSelect(ST_BatchJob *a_batchjob, ST_PROCESS *a_process)
{
	g_pcLog->INFO("CProcess Init");
    return BJM_OK;

}

int CProcess::ProcDelete()
{
	g_pcLog->INFO("CProcess Init");
    return BJM_OK;

}

int CProcess::ProcUpdate()
{
	g_pcLog->INFO("CProcess Init");
    return BJM_OK;

}

int CProcess::ProcInsert()
{
	g_pcLog->INFO("CProcess Init");
    return BJM_OK;

}
