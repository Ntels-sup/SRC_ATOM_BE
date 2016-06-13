//#include "CMain.hpp"
#include "BJM_Define.hpp"
#include "CScheduler.hpp"
#include "CNMSession.hpp"
#include "CATMDate.hpp"
#include "CATMTime.hpp"
#include "MariaDB.hpp"
#include <vector>
#include <map>
#include <cstdlib>

using namespace std;

//extern symbol
extern CFileLog* g_pcLog;
extern CConfigBJM g_pcCFG;
extern CNMSession g_pcNM;

CScheduler::CScheduler()
{
    m_pDB = NULL;

	m_nStartFlag = BJM_OK;
    memset(m_lastScheduleDate_,0x00,sizeof(m_lastScheduleDate_)) ;
    memset(nextScheduleDate_,0x00,sizeof(nextScheduleDate_)) ;

	m_nBatchGroupCount = 0;
	m_nBatchJobCount = 0;
	m_nBatchFlowCount = 0;
	m_nProcessCount = 0;
	m_nMultiJobCount = 0;
	m_nDBInitFlag = 0;			//start
	m_nInitInfoFlag = 0;

	if(Initial() != BJM_OK)
	{
		g_pcLog->ERROR("CScheduler Initialize error");
		m_nStartFlag = BJM_NOK;
	}
}

CScheduler::~CScheduler()
{
    if(m_pDB != NULL)
    {
        delete m_pDB;
        m_pDB = NULL;
    }
}

int CScheduler::Initial()
{
	int nRet = 0;

	g_pcLog->INFO("CScheduler Initialize");

	// DB Connection
    m_pDB = new (std::nothrow) MariaDB();
    if(m_pDB == NULL)
    {
        g_pcLog->INFO("new operator Fail [%d:%s]", errno, strerror(errno));
        return BJM_NOK;
    }

    // CONNECT
//    if(m_pDB->Connect("127.0.0.1", 3306, "atom", "atom", "ATOM")  != true)
    nRet = m_pDB->Connect(g_pcCFG.DB.m_strDbAddr.c_str(), 
						  g_pcCFG.DB.m_nDbPort, 
						  g_pcCFG.DB.m_strDbuserId.c_str(), 
						  g_pcCFG.DB.m_strDbPasswd.c_str(), 
						  g_pcCFG.DB.m_strDbName.c_str());
	if(nRet < 0)
    {
        g_pcLog->ERROR("Connect Fail [%d:%s]", nRet, m_pDB->GetErrorMsg(nRet));
        return BJM_NOK;
    }

    g_pcLog->INFO("DB Connection ...OK");

	// TAT_BATCH_JOB init
    if(m_BatchGroup.Init(m_pDB) != BJM_OK)
    {
        g_pcLog->ERROR("CBatchGroup - Init fail\n");
        return BJM_NOK;
    }

	// TAT_BATCH_CMD init
    if(m_BatchJob.Init(m_pDB) != BJM_OK)
    {
        g_pcLog->ERROR("CBatchJob - Init fail\n");
        return BJM_NOK;
    }

	// TAT_BATCH_FLOW init
    if(m_BatchFlow.Init(m_pDB) != BJM_OK)
    {
        g_pcLog->ERROR("CBatchFlow - Init fail\n");
        return BJM_NOK;
    }
	
	// TAT_BATCH_HIST init
    if(m_BatchHist.Init(m_pDB) != BJM_OK)
    {
        g_pcLog->ERROR("CBatchHist - Init fail\n");
        return BJM_NOK;
    }

    if(m_Process.Init(m_pDB) != BJM_OK)
    {
        g_pcLog->ERROR("CProcess - Init fail\n");
        return BJM_NOK;
    }

	// Get BJM Self Info
//	if(LoadCoworkNo("ATOM", "NM", "BJM", "WSM") == BJM_NOK)
	if(m_Process.LoadCoworkNo((char *)g_pcCFG.LOG.m_strNodeName.c_str(), 
				              (char *)g_pcCFG.LOG.m_strNmName.c_str(), 
							  (char *)g_pcCFG.LOG.m_strBjmName.c_str(), 
							  (char *)g_pcCFG.LOG.m_strWsmName.c_str(), 
							  m_stcoworkinfo) == BJM_NOK)
	{
		g_pcLog->ERROR("CScheduler LoadCoworkNo Error");	
		return BJM_NOK;
	}

	g_pcLog->INFO("Pkg_Name[%s], Node_Type[%s], Node_No[%d], Proc_No(NM)[%d], Proc_No(BJM)[%d], Proc_No(WSM)[%d]"
			, m_stcoworkinfo->pkg_name
			, m_stcoworkinfo->node_type
			, m_stcoworkinfo->node_no
			, m_stcoworkinfo->nm_proc_no
			, m_stcoworkinfo->bjm_proc_no
			, m_stcoworkinfo->wsm_proc_no);

	g_pcLog->INFO("Data Initialization - OK. ");

	return BJM_OK;
}

int CScheduler::Run()
{	
	g_pcLog->INFO("CScheduler Run");
	if(m_nStartFlag == BJM_NOK)
		return false;

	if(GetGroupInfo() == BJM_NOK)
	{
		g_pcLog->ERROR("CScheduler GetGroupInfo Error");	
		return false;
	}
	
	if(g_pcNM.Regist(m_stcoworkinfo) == BJM_NOK)
	{
		return false;
	}

	CheckCurrentTime();
	
	return true;

}

int CScheduler::ReceiveData(ST_COWORK_INFO a_stcoworkinfo)
{
	int nRecvFlag = 0;
    ST_COWORK_INFO          stcoworkinfo;
    stcoworkinfo = a_stcoworkinfo;

	nRecvFlag = g_pcNM.RecvMsg(m_pDB, m_stBatchRequest, m_stBatchResponse);
	g_pcLog->DEBUG("nRecvFlag[%d]", nRecvFlag);

	switch(nRecvFlag)
	{
		case BJM_RECV_RECONFIG:			// INIT Request
			if(m_nDBInitFlag == 1) // on operation
			{
				g_pcLog->INFO("BJM Schedule Job is on operation [%d]", nRecvFlag);
				memset(m_stbatchjobs, 0x00, sizeof(m_stbatchjobs)) ;
				memset(m_stprocess, 0x00, sizeof(m_stprocess)) ;
				memset(m_stbatchgroups, 0x00, sizeof(m_stbatchgroups)) ;
				memset(m_stbatchflows, 0x00, sizeof(m_stbatchflows)) ;
			}

			if(GetGroupInfo() == BJM_NOK)
			{
				g_pcLog->ERROR("Data Initialization Error [%d]", nRecvFlag);
				g_pcNM.SendInitResponseMsg(0, &stcoworkinfo, m_stBatchRequest);
				return BJM_NOK;
			}
			else
			{
				g_pcNM.SendInitResponseMsg(1, &stcoworkinfo, m_stBatchRequest);
			}

		 	break;
		case BJM_RECV_WSM_START:
			if( SearchBatchJob(m_stBatchRequest) == BJM_NOK)
			{
				g_pcLog->ERROR("WSM START Error [%d]", nRecvFlag);
				return BJM_NOK;
			}
		 	break;
		case BJM_RECV_START:
			if(g_pcNM.IsRequest() == 1 || g_pcNM.IsRequest() == 2) // response, notify
			{
				if(g_pcNM.GetResponseMsg(m_stBatchResponse, m_vBatchReq) == false)
				{       
					g_pcLog->ERROR("GetResponseMsg Error - false");
					return BJM_NOK;
				}
				else
				{
					m_stBatchResponse->exit_cd = 66;	// for debug 
					strcpy(m_stBatchResponse->status, "normal");	// for debug 
					
					if(m_stBatchResponse->exit_cd > -1)	// exit_cd < 0 종료 
					{
						if( CheckMultiJob(m_stBatchResponse->group_name, m_stBatchResponse->job_name, m_stBatchResponse->exit_cd) == BJM_OK)
						{
							SearchNextJob(m_stBatchResponse);
						}
					}

					if(strncmp( m_stBatchResponse->wsm_yn, "Y", strlen("Y")) == 0)
					{
						if( strncmp(m_stBatchResponse->status, "normal", strlen("normal")) == 0)
						{
							g_pcNM.SendWSMResponseMsg(1, &stcoworkinfo, m_stBatchRequest, m_stBatchResponse);	//true
							return BJM_NOK;
						}
						else
						{
							g_pcNM.SendWSMResponseMsg(0, &stcoworkinfo, m_stBatchRequest, m_stBatchResponse);
						}
					}
				}
			}
		 	break;
		case BJM_RECV_REGIST:
			g_pcLog->INFO("Receive Regist [%d]", nRecvFlag);
			break;
		case BJM_RECV_ERROR:
			g_pcLog->DEBUG("Data Receive Error [%d]", nRecvFlag);
			return BJM_RECV_ERROR;
		case BJM_RECV_PROC_START:
			g_pcLog->INFO("Process Start");
			break;
		case BJM_RECV_PROC_STOP:
			g_pcLog->INFO("Processs Stop");
			// 진행 중인 Job 초기화 후 다시 체크하게 함
			memset(m_stbatchjobs, 0x00, sizeof(m_stbatchjobs)) ;
			memset(m_stprocess, 0x00, sizeof(m_stprocess)) ;
			memset(m_stbatchgroups, 0x00, sizeof(m_stbatchgroups)) ;
			memset(m_stbatchflows, 0x00, sizeof(m_stbatchflows)) ;
			exit(EXIT_SUCCESS);
			break;
		case BJM_RECV_BACKUP:
			if(g_pcNM.IsRequest() == 0)		// request
			{
				if( g_pcNM.GetBackupRequest(m_stBackupRequest) == BJM_OK)
				{
					g_pcNM.SendBackupRequestMsg(&stcoworkinfo, m_stbatchjobs, m_stBackupRequest);
				}
			}
			else if(g_pcNM.IsRequest() == 1) // response
			{
				g_pcNM.GetBackupResponse(&stcoworkinfo,m_stBackupRequest);
			}

			break;
		case BJM_RECV_RESTORE:
			if(g_pcNM.IsRequest() == 0)
			{
				if(g_pcNM.GetRestoreRequest(m_stRestoreRequest) == BJM_OK)
				{
					g_pcNM.SendRestoreRequestMsg(&stcoworkinfo, m_stbatchjobs, m_stRestoreRequest);
				}
			}
			else if(g_pcNM.IsRequest() == 1)
			{
				g_pcNM.GetRestoreResponse(&stcoworkinfo, m_stRestoreRequest);
			}
			break;
		case BJM_RECV_SCALE_IN:
			g_pcLog->INFO("Recvive SCALE IN");

			if(g_pcNM.IsRequest() == 2)
			{
            	if(GetGroupInfo() == BJM_NOK)
            	{
                	g_pcLog->ERROR("Data Initialization Error [%d]", nRecvFlag);
    	            return BJM_NOK;
        	    }
			}
			break;
		case BJM_RECV_SCALE_OUT:
			g_pcLog->INFO("Recvive SCALE OUT");

			if(g_pcNM.IsRequest() == 2)
			{
            	if(GetGroupInfo() == BJM_NOK)
            	{
                	g_pcLog->ERROR("Data Initialization Error [%d]", nRecvFlag);
    	            return BJM_NOK;
        	    }
			}
			break;
//		dafault:
//			break;

	}
	return BJM_OK;
}

int CScheduler::SearchNextJob(ST_BatchResponse *a_stBatchresponse)
{
	ST_BatchFlow stbatchflow[BJM_MAX_BATCH_FLOW];
	ST_BatchJob  stbatchjob[BJM_MAX_BATCH_JOB];
	ST_PROCESS   stprocess[BJM_MAX_PROCESS];
	ST_BatchResponse stbatchresponse;

    memset(stbatchflow, 0x00, sizeof(stbatchflow)) ;
    memset(stbatchjob, 0x00, sizeof(stbatchjob)) ;
    memset(stprocess, 0x00, sizeof(stprocess)) ;

	int nGrpIndex = 0, nMultiCount = 0;

	stbatchresponse = *a_stBatchresponse;
	
	g_pcLog->DEBUG("SearchNextJob *a_stBatchresponse [%x], stbatchresponse [%x]", *a_stBatchresponse, stbatchresponse);
	
	FindNextJobInfo(stbatchresponse.job_name);

	for (nMultiCount = 0; nMultiCount < (int)m_vNextFlow.size(); nMultiCount++)
	{
		nGrpIndex = FindGroupInfo(m_vNextFlow[nMultiCount]->next_job_name, stbatchresponse.group_name);
		if (nGrpIndex < 0)
		{
		   	return BJM_NOK;
		}
		else
		{
            g_pcLog->INFO("m_vNextFlow[%d]->next_job_name %s, m_vBatchJob[%d].group_name %s, m_vBatchJob[%d].job_name %s",
                nMultiCount,
                m_vNextFlow[nMultiCount]->next_job_name,
                nGrpIndex,
                m_vBatchJob[nGrpIndex]->group_name,
                nGrpIndex,
                m_vBatchJob[nGrpIndex]->job_name );

			if(strncmp(stbatchresponse.wsm_yn, "Y", sizeof("Y")) == 0)
			{
				if(ExcuteJobInfo(nGrpIndex, 1) == BJM_NOK)
					return BJM_NOK;
			}
			else
			{
				if(ExcuteJobInfo(nGrpIndex, 0) == BJM_NOK)
					return BJM_NOK;
			}
		}
    	m_vNextFlow.erase(m_vNextFlow.begin() + nMultiCount) ;
	}

	return BJM_OK;
}

int CScheduler::ExcuteJobInfo(int a_nGrpCnt, int a_wsm)
{
	int nPrcCnt = 0;

	if(m_vBatchJob[a_nGrpCnt]->node_no == 0)		// Node Type All
	{
		for(nPrcCnt = 0; nPrcCnt < (int)m_vProcess.size(); nPrcCnt++)
		{	
			if(strcmp(m_vProcess[nPrcCnt]->pkg_name, m_vBatchJob[nPrcCnt]->pkg_name) == 0 && 
					m_vProcess[nPrcCnt]->proc_no == m_vBatchJob[nPrcCnt]->proc_no && 
					SetBatchHist(m_vBatchJob[nPrcCnt], m_vProcess[nPrcCnt], nPrcCnt, a_wsm) == BJM_NOK)
		    {
        	    g_pcLog->ERROR("ExcuteJobInfo(All) false [%x]", m_vBatchJob[nPrcCnt]);
	            return BJM_NOK;
			}
		}
	}
	else
	{
		if(strcmp(m_vProcess[a_nGrpCnt]->node_type, m_vBatchJob[a_nGrpCnt]->node_type ) == 0)
		{
		 	if(strcmp(m_vProcess[a_nGrpCnt]->pkg_name, m_vBatchJob[a_nGrpCnt]->pkg_name) == 0 &&
					m_vProcess[a_nGrpCnt]->proc_no == m_vBatchJob[a_nGrpCnt]->proc_no && 
					SetBatchHist(m_vBatchJob[a_nGrpCnt], m_vProcess[a_nGrpCnt], a_nGrpCnt, a_wsm) == BJM_NOK)
			{
           		g_pcLog->ERROR("ExcuteJobInfo false [%x]", m_vBatchJob[a_nGrpCnt]);
   				return BJM_NOK;
			}
		}
	}

	return BJM_OK;
}

int CScheduler::FindGroupInfo(char * a_strJobName, char * a_strGroupName)
{
	int j = 0;

    for(j = 0; j < (int)m_vBatchJob.size(); j++)
    {
	    if(strcmp(m_vBatchJob[j]->group_name, a_strGroupName) == 0 &&
           strcmp(m_vBatchJob[j]->job_name, a_strJobName) == 0)
        {
			return j;
		}
	}
	return -1;
}

void CScheduler::FindNextJobInfo(char * a_strJobName)
{
	int i = 0;

    for (i = 0; i < (int)m_vBatchFlow.size(); i++)
    {
        if(strcmp(m_vBatchFlow[i]->job_name, a_strJobName) == 0 && strcmp(m_vBatchFlow[i]->next_job_name, "") != 0 && 
          m_vBatchFlow[i]->exit_cd >= 0)
		{
			m_vNextFlow.push_back(m_vBatchFlow[i]);
		}
    }
}

int CScheduler::CheckMultiJob(char * a_strGroupName, char * a_strJobName, int a_nExitCd)
{
    int i = 0;
    int j = 0;

    for (j = 0; j < (int)m_vBatchFlow.size(); j++)
    {
        if(strcmp(m_vBatchFlow[j]->job_name, a_strJobName) == 0 && m_vBatchFlow[j]->exit_cd != a_nExitCd)
        {
			return BJM_NOK;
        }
    }

	for (i = 0; i < (int)m_vMultiFlow.size(); i++)
	{
		if(strcmp(m_vMultiFlow[i]->group_name, a_strGroupName) == 0)
		{
			if(strcmp(m_vMultiFlow[i]->prev_job_name, a_strJobName) == 0)
			{
				strcpy(m_vMultiFlow[i]->prev_job_name, "");
				m_vMultiFlow[i]->CurCount = 0;
			}

    	    if(m_vMultiFlow[i]->exit_cd == a_nExitCd)
        	{
				strcpy(m_vMultiFlow[i]->prev_job_name, a_strJobName);
           		g_pcLog->INFO("1 m_vMultiFlow[%d]->CurCount %d", i, m_vMultiFlow[i]->CurCount );

				m_vMultiFlow[i]->CurCount++;

				if(m_vMultiFlow[i]->CurCount == m_vMultiFlow[i]->Count)
				{
           			g_pcLog->INFO("2 m_vMultiFlow[%d]->CurCount %d", i, m_vMultiFlow[i]->CurCount );
					strcpy(m_vMultiFlow[i]->prev_job_name, "");
					m_vMultiFlow[i]->CurCount = 0;
					return BJM_OK;
				}

           		g_pcLog->INFO("3 m_vMultiFlow[%d]->CurCount %d", i, m_vMultiFlow[i]->CurCount );
			}
        }
    }
    return BJM_NOK;
}

int CScheduler::SearchBatchJob(ST_BatchRequest *a_stBatchrequest)
{
	ST_BatchRequest stbatchrequest;
	stbatchrequest = *a_stBatchrequest;
	unsigned int j = 0;

	for (j = 0; j < m_vBatchJob.size(); j++)
	{
		if(strncmp(m_vBatchJob[j]->group_name, stbatchrequest.group_name, sizeof(stbatchrequest.group_name)) == 0 &&
			strncmp(m_vBatchJob[j]->job_name, stbatchrequest.job_name, sizeof(stbatchrequest.job_name)) == 0)
		{
			g_pcLog->INFO("m_vBatchJob[%d].group_name %s, m_vBatchJob[%d].job_name %s", 
					j, m_vBatchJob[j]->group_name, 
					j, m_vBatchJob[j]->job_name );
			if(strncmp(stbatchrequest.wsm_yn, "Y", sizeof("Y")) == 0)
			{
				if(ExcuteJobInfo(j, 1) == BJM_NOK)
					return BJM_NOK;
			}
			else
			{
				if(ExcuteJobInfo(j, 0) == BJM_NOK)
					return BJM_NOK;
			}

		}
	}

	return BJM_OK;
}

int CScheduler::CheckCurrentTime()
{
	CATMTime nowT;
	int  i = 0;
    char strCurrent_date[BJM_GROUP_START_DATE_SIZE + 1] = "";

    ST_COWORK_INFO   stcoworkinfo;
    stcoworkinfo =  *m_stcoworkinfo;    

	nowT = time(NULL);

	for (i = 0; i < BJM_MAX_BATCH_JOB; i++)
		sprintf(nextScheduleDate_[i], "%s", nowT.AscTime("CCYYMMDDhhmmss"));
	
	while(1)	
	{
		nowT.Now();
		
		sprintf(strCurrent_date, "%s", nowT.AscTime("CCYYMMDDhhmmss"));

		RunSchedule(strCurrent_date);

		if(ReceiveData(stcoworkinfo) == BJM_RECV_ERROR)
		{
			break;
		}
		sleep(1);
	}

	return BJM_OK;
}

void CScheduler::RunSchedule(char *a_strCurrent_date)
{
	int  nGroupCount = 0;
	int  nJobCount = 0;

	for(nJobCount = 0; nJobCount < m_nBatchJobCount; nJobCount++)
	{
		{
			if(strncmp(m_vBatchJob[nJobCount]->nextExecDate, a_strCurrent_date, 14) == 0 && m_vProcess[nJobCount] != NULL)
			{
				for(nGroupCount = 0; nGroupCount < m_nBatchGroupCount; nGroupCount++)
				{
					if(strncmp(m_vBatchJob[nJobCount]->group_name, m_vBatchGroup[nGroupCount]->group_name, 
						sizeof(m_vBatchGroup[nGroupCount]->group_name)) == 0 )
					{
						SetNextSchedule(m_vBatchGroup[nGroupCount], m_vBatchJob[nJobCount]->nextExecDate, nJobCount);
						strncpy(m_vBatchJob[nJobCount]->nextExecDate, nextScheduleDate_[nJobCount], sizeof(nextScheduleDate_[nJobCount]));
					}
				}
				m_nDBInitFlag = 1;			//running

				if( (strncmp(m_vBatchJob[nJobCount]->rootjob_yn, "Y", sizeof("Y")) == 0) && 		
					(strncmp(m_vBatchJob[nJobCount]->run_yn, "Y", sizeof("Y")) == 0))
				{
					if(SetBatchHist(m_vBatchJob[nJobCount], m_vProcess[nJobCount], nJobCount) == BJM_NOK )
	    	    	{
       			    	g_pcLog->ERROR("SetBatchHist - SetBatchHist Error - false");
	    		    }
					else
					{
						g_pcLog->INFO("%s,\t%s, Current : %s, nextExecDate[%d] : %s, %d", 
			      				m_vBatchJob[nJobCount]->group_name,  
								m_vBatchJob[nJobCount]->job_name,
								a_strCurrent_date, 
								nJobCount, 
								m_vBatchJob[nJobCount]->nextExecDate);
					}
				}
				m_nDBInitFlag = 0;
			}
		}
	}
}

int CScheduler::SetBatchHist(ST_BatchJob *a_stBatchJobInfo, ST_PROCESS *a_stProcess, int a_count, int a_nWsmflag)
{
	ST_BatchJob		stbatchjob;
	ST_PROCESS		stprocess;
	ST_BATCHHIST	stbatchhist;

	ST_BatchRequest *stbatchrequest = new ST_BatchRequest;
	memset(stbatchrequest, 0x00, sizeof(ST_BatchRequest));

	stbatchjob = *a_stBatchJobInfo;
	stprocess = *a_stProcess;

	strncpy(stbatchhist.group_name, stbatchjob.group_name, sizeof(stbatchjob.group_name));
	strncpy(stbatchhist.job_name, stbatchjob.job_name, sizeof(stbatchjob.job_name));
	strncpy(stbatchhist.pkg_name, stbatchjob.pkg_name, sizeof(stbatchjob.pkg_name));
	strncpy(stbatchhist.proc_name, stprocess.proc_name, sizeof(stprocess.proc_name));
	strncpy(stbatchhist.node_name, stprocess.node_name, sizeof(stprocess.node_name));
	strncpy(stbatchhist.prc_date, m_lastScheduleDate_[a_count], sizeof(m_lastScheduleDate_[a_count]));
	stbatchhist.exit_cd = 0;

	m_BatchHist.HistInsert(&stbatchhist);

	strncpy(stbatchrequest->group_name, stbatchjob.group_name, sizeof(stbatchjob.group_name));
	strncpy(stbatchrequest->job_name, stbatchjob.job_name, sizeof(stbatchjob.job_name));
	strncpy(stbatchrequest->pkg_name, stbatchjob.pkg_name, sizeof(stbatchjob.pkg_name));
	stbatchrequest->proc_no = stbatchjob.proc_no;
	strncpy(stbatchrequest->prc_date, m_lastScheduleDate_[a_count], sizeof(m_lastScheduleDate_[a_count]));
	strncpy(stbatchrequest->exec_bin, stprocess.exec_bin, sizeof(stprocess.exec_bin));
	strncpy(stbatchrequest->exec_arg, stprocess.exec_arg, sizeof(stprocess.exec_arg));
	strncpy(stbatchrequest->exec_env, stprocess.exec_env, sizeof(stprocess.exec_env));

	if(a_nWsmflag == 1)
	{
		strncpy(stbatchrequest->wsm_yn, "Y", sizeof("Y"));
	}
	else
	{
		strncpy(stbatchrequest->wsm_yn, "N", sizeof("N"));
	}

	if(g_pcNM.SendRequestMsg(m_stcoworkinfo, &stbatchjob, stbatchrequest) == BJM_NOK)
	{
		g_pcLog->ERROR("SetBatchHist SendRequestMsg Error");
		return BJM_NOK;
	}

	stbatchrequest->sequence = g_pcNM.GetSequence();

	m_vBatchReq.insert(m_vBatchReq.end(), stbatchrequest);

	return BJM_OK;
}

// Load Batch Group/Job/Flow/Process info
int CScheduler::GetGroupInfo()
{
	int i = 0;	// group

	m_nBatchGroupCount = m_BatchGroup.LoadGroupInfo(m_stbatchgroups);
	if(m_nBatchGroupCount  < 0)
	{
		g_pcLog->INFO("LoadGroupInfo Error");
		return BJM_NOK;
	}
	else
	{
		for(i = 0; i < m_nBatchGroupCount; i++)
		{
			m_vBatchGroup.push_back(&m_stbatchgroups[i]);
			
		}
	}

	if(GetJobInfo() == BJM_NOK)
		return BJM_NOK;

	if( GetFlowInfo() == BJM_NOK)
		return BJM_NOK;

	return BJM_OK;
}

int CScheduler::GetJobInfo()
{
	int j = 0;
	char lastschedule[14];

	CATMTime nowT;
	nowT = time(NULL);
	nowT.Now();


	m_nBatchJobCount = m_BatchJob.LoadJobInfo(m_stbatchjobs);
	if(m_nBatchJobCount < 0)
	{
		g_pcLog->INFO("LoadJobInfo Error");
		return BJM_NOK;
	}
	else
	{
		if(m_nBatchJobCount != 0) 
		{
			for(j = 0; j < m_nBatchJobCount; j++)
			{
				sprintf(lastschedule, "%s", nowT.AscTime("CCYYMMDDhhmmss"));					

				strcpy(m_stbatchjobs[j].nextExecDate, lastschedule);

				m_vBatchJob.push_back(&m_stbatchjobs[j]);
				g_pcLog->DEBUG("group_name[%d] : %s, job_name[%d] : %s", 
								j, &m_stbatchjobs[j].group_name, j, &m_stbatchjobs[j].job_name);
					
				//process
				m_nProcessCount = m_Process.LoadProcInfo(&m_stbatchjobs[j], &m_stprocess[j]);
				if(m_nProcessCount < 0)
				{
					g_pcLog->ERROR("LoadFlowInfo Error");
					return BJM_NOK;
				}
				else
				{
					if(m_nProcessCount != 0)
					{
						m_vProcess.push_back(&m_stprocess[j]);
					}
				}
			}
		}
		g_pcLog->DEBUG("m_nBatchJobCount : %d", m_nBatchJobCount);
	}
	return BJM_OK;
}
int CScheduler::GetFlowInfo()
{
	int k = 0, j = 0;

	m_nBatchFlowCount = m_BatchFlow.LoadFlowInfo(m_stbatchflows);

	if(m_nBatchFlowCount < 0)
	{
		g_pcLog->ERROR("LoadFlowInfo Error");
		return BJM_NOK;
	}
	else
	{
		if(m_nBatchFlowCount != 0)
		{
			for(k = 0; k < m_nBatchFlowCount; k++)
			{
				g_pcLog->DEBUG("Exit CD[%d][%d] \tCurr Job Name[%d][%s], \tNext Job Name[%d][%s]", 
								k, m_stbatchflows[k].exit_cd, 
								k, m_stbatchflows[k].job_name, 
								k, m_stbatchflows[k].next_job_name);
				m_vBatchFlow.push_back(&m_stbatchflows[k]);
			}
		}
	}

	m_nMultiJobCount = m_BatchFlow.FlowSelect(m_stMultiJobFlow);

	if(m_nMultiJobCount < 0)
	{
		g_pcLog->ERROR("LoadFlowInfo Error");
		return BJM_NOK;
	}
	else
	{
		if(m_nMultiJobCount != 0)
		{
			for(j = 0; j < m_nMultiJobCount; j++)
			{
				strcpy(m_stMultiJobFlow[j].group_name, m_BatchGroup.GetGroupName(m_stMultiJobFlow[j].next_job_name));
				m_stMultiJobFlow[j].Count = m_BatchFlow.GetMultiFlowCount(m_stMultiJobFlow[j].exit_cd);
				g_pcLog->INFO("Count [%d], Group Name[%d][%s], Exit CD[%d][%d], Next Job Name[%d][%s]", 
								m_stMultiJobFlow[j].Count,
								j, m_stMultiJobFlow[j].group_name, 
								j, m_stMultiJobFlow[j].exit_cd, 
								j, m_stMultiJobFlow[j].next_job_name);


				m_vMultiFlow.push_back(&m_stMultiJobFlow[j]);
			}
		}
	}
	
	return BJM_OK;
}

void CScheduler::SetNextScheduleMinute(int a_nCount)
{
	char 	 strDate[9];
    int      hour, minute = 0, second;
    int      thour, tmin = 0;

    CATMDate *nextDate ;
    char    m_strTempDate[9] ;

    sprintf(m_strTempDate,"%8.8s",m_lastScheduleDate_[a_nCount]) ;
    nextDate = new CATMDate(m_strTempDate);

    sprintf(m_strTempDate,"%2.2s", m_lastScheduleDate_[a_nCount]+8) ;
    hour = atoi(m_strTempDate) ;
    sprintf(m_strTempDate,"%2.2s",m_lastScheduleDate_[a_nCount]+10) ;
    minute = atoi(m_strTempDate);
    sprintf(m_strTempDate,"%2.2s",m_lastScheduleDate_[a_nCount]+12) ;
    second = atoi(m_strTempDate) ;

    thour = m_batchGroupInfo_->nschedule_cycle / 60 ;
    tmin  = m_batchGroupInfo_->nschedule_cycle % 60 ;

    if ( tmin + minute >= 60 )
    {
        thour++ ;
        minute = minute + tmin - 60 ;
    }
    else
    {
        minute += tmin ;
    }

    if ( hour + thour >= 24 )
    {
        nextDate->AddDate(1) ;
        hour = hour + thour - 24 ;
    }
    else
    {
        hour += thour ;
    }

	sprintf(strDate, "%8.8s", nextDate->AscDate("CCYYMMDD"));
    sprintf(nextScheduleDate_[a_nCount], "%s%02d%02d%02d", strDate, hour, minute, second);

    delete nextDate ;
}

void CScheduler::SetNextScheduleHour(int a_nCount)
{
    CATMDate *nextDate ;
    int      hour,thour ;
    char     m_strTempDate[9] ;

    sprintf(m_strTempDate,"%8.8s",m_lastScheduleDate_[a_nCount]) ;
    nextDate = new CATMDate(m_strTempDate) ;

    sprintf(m_strTempDate,"%2.2s",m_lastScheduleDate_[a_nCount]+8) ;
    hour = atoi(m_strTempDate) ;

    thour = m_batchGroupInfo_->nschedule_cycle  ;
    if ( hour + thour >= 24 )
    {
        nextDate->AddDate(1) ;
        hour = hour + thour - 24 ;
    }
    else
    {
        hour += thour ;
    }

    sprintf(nextScheduleDate_[a_nCount],"%s%02d%2.2s", nextDate->AscDate("CCYYMMDD"), hour, m_lastScheduleDate_[a_nCount]+10) ;

    delete nextDate ;
}

void CScheduler::SetNextScheduleDay(int a_nCount)
{
    CATMDate *nextDate;
    char    m_strTempDate[9];

    sprintf(m_strTempDate,"%8.8s",m_lastScheduleDate_[a_nCount]);
    nextDate = new CATMDate(m_strTempDate);

    nextDate->AddDate(m_batchGroupInfo_->nschedule_cycle);

    sprintf(nextScheduleDate_[a_nCount],"%s%s", nextDate->AscDate("CCYYMMDD"), m_lastScheduleDate_[a_nCount]+8) ;

    delete nextDate;
}

void CScheduler::SetNextScheduleWeek(int a_nCount)
{
    CATMDate *nextDate;
    char    m_strTempDate[9];
    int     nWeek;

    sprintf(m_strTempDate,"%8.8s",m_lastScheduleDate_[a_nCount]);
    nextDate = new CATMDate(m_strTempDate);

    nWeek = m_batchGroupInfo_->nschedule_cycle + 7;

    nextDate->AddDate(nWeek);

    sprintf(nextScheduleDate_[a_nCount],"%s%s", nextDate->AscDate("CCYYMMDD"), (char*)m_lastScheduleDate_[a_nCount]+ 8);

    delete nextDate;
}

void CScheduler::SetNextScheduleMonth(int a_nCount)
{
    int     year,month ;

    year  = CATMConv::Str2Int(m_lastScheduleDate_[a_nCount]+0,4) ;
    month = CATMConv::Str2Int(m_lastScheduleDate_[a_nCount]+4,2) ;

    month += m_batchGroupInfo_->nschedule_cycle ;
    if ( month > 12 )
    {
        year++ ;
        month -= 12 ;
    }

    sprintf(nextScheduleDate_[a_nCount],"%04d%02d%s", year, month, m_lastScheduleDate_[a_nCount]+6) ;
}

void CScheduler::SetNextScheduleMonthLast(int a_nCount)
{
    int     year,month ;
    int     day[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    year  = CATMConv::Str2Int(m_lastScheduleDate_[a_nCount]+0,4) ;
    month = CATMConv::Str2Int(m_lastScheduleDate_[a_nCount]+4,2) ;

    month += m_batchGroupInfo_->nschedule_cycle ;
    if ( month > 12 )
    {
        year++ ;
        month -= 12 ;
    }

    if( (year%4) == 0 )
    {
        if( (year%100) == 0 )
        {
            if( (year%400) ==0 )
            {
                day[1] = 29;
            }
            else
            {
                day[1] = 28;
            }
        }
        else
        {
            day[1] = 29;
        }
    }

    sprintf(nextScheduleDate_[a_nCount],"%04d%02d%02d%s", year, month, day[month-1], m_lastScheduleDate_[a_nCount]+ 8) ;
}

int CScheduler::SetNextSchedule(ST_BatchGroup *a_stBatchGroupInfo, char *a_nextExecDate, int a_nCount)
{
    m_batchGroupInfo_ = a_stBatchGroupInfo;

    if ( atoi(m_batchGroupInfo_->schedule_cycle_type) == 1 || atoi(m_batchGroupInfo_->schedule_cycle_type) == 0 )
    {
         g_pcLog->INFO("Next schedule date of Pkg Name(%s)-Group Name[%s] was not reserved!", GetPkgName(), GetGroupName()) ;

        return BJM_OK ;
    }

	strncpy(m_lastScheduleDate_[a_nCount], a_nextExecDate, 14);

    switch(atoi(m_batchGroupInfo_->schedule_cycle_type))
    {
			case 1:	 // NONE
               g_pcLog->INFO("Package(%s)-BatchGroup(%s) can't find next schedule!", GetPkgName(), GetGroupName()) ;
				break;
            case 2 : // '02' Minute
                SetNextScheduleMinute(a_nCount) ;
                break ;
            case 3 : // '03' Day 
                SetNextScheduleDay(a_nCount) ;
                break ;
            case 4 : // '04' Week() 
                SetNextScheduleWeek(a_nCount) ;
                break ;
            case 5 : // '05' Month
                SetNextScheduleMonth(a_nCount) ;
                break ;
            case 6 : // '06' MonthLast 
                SetNextScheduleMonthLast(a_nCount) ;
                break ;
            case 7 : // '07' Hour 
                SetNextScheduleHour(a_nCount) ;
                break ;
            default  :
               g_pcLog->INFO("Package(%s)-BatchGroup(%s) can't find next schedule!", GetPkgName(), GetGroupName()) ;
               return BJM_NOK ;
    }

    return BJM_OK ;
}
