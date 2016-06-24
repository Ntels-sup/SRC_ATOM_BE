#ifndef __CSchedulerHpp__
#define __CSchedulerHpp__

#include "BJM_Define.hpp"
#include <iostream>
#include <vector>
#include "DB.hpp"
#include "CBatchJob.hpp"
#include "CBatchGroup.hpp"
#include "CBatchFlow.hpp"
#include "CBatchHist.hpp"
#include "CProcess.hpp"

using namespace std;

class CScheduler
{

private:
	DB * 			m_pDB;

	CBatchJob	    m_BatchJob;		// Batch Job info
	CBatchGroup 	m_BatchGroup;	// Batch Group info
	CBatchFlow	    m_BatchFlow;	// Batch Flow info
	CBatchHist		m_BatchHist;
	CProcess 		m_Process;		

	int 			GetGroupInfo();
	int 			GetJobInfo();
	int 			GetFlowInfo();

	int				CheckCurrentTime();

	ST_BatchGroup   m_stbatchgroups			[BJM_MAX_BATCH_GROUP];
   	ST_BatchJob     m_stbatchjobs			[BJM_MAX_BATCH_JOB];
    ST_BATCHFLOW    m_stbatchflows			[BJM_MAX_BATCH_FLOW];
   	ST_BATCHHIST    m_stbatchhists			[BJM_MAX_BATCH_HIST];
   	ST_PROCESS	    m_stprocess				[BJM_MAX_PROCESS];
	ST_COWORK_INFO 	m_stcoworkinfo			[0];
	ST_BatchRequest m_stBatchRequest		[BJM_MAX_REQUEST];
	ST_BatchResponse m_stBatchResponse		[BJM_MAX_RESPONSE];
	ST_BACKUP_REQUEST m_stBackupRequest		[BJM_MAX_REQUEST];
	ST_BACKUP_RESPONSE  m_stBackupResponse	[BJM_MAX_RESPONSE];
	ST_RESTORE_REQUEST m_stRestoreRequest	[BJM_MAX_REQUEST];
    ST_MULTI_JOB_FLOW m_stMultiJobFlow		[BJM_MAX_BATCH_FLOW];

    ST_BatchGroup   *m_batchGroupInfo_;

	int				m_nStartFlag;
    int             m_nBatchGroupCount;
   	int             m_nBatchJobCount;
    int             m_nBatchFlowCount;
    int             m_nProcessCount;
	int 			m_nMultiJobCount;
	int				m_nDBInitFlag;
	int				m_nInitInfoFlag;

    char            m_lastScheduleDate_[BJM_MAX_BATCH_JOB][APPLY_DATE_SIZE + 1] ;
    char            nextScheduleDate_[BJM_MAX_BATCH_JOB][APPLY_DATE_SIZE + 1] ;

	vector<ST_BatchGroup *> 	m_vBatchGroup;
	vector<ST_BatchJob *>  		m_vBatchJob;
	vector<ST_PROCESS *>		m_vProcess;
	vector<ST_BATCHFLOW *>		m_vBatchFlow;
	vector<ST_BatchRequest *>	m_vBatchReq;
	vector<ST_BATCHFLOW *>		m_vNextFlow;
	vector<ST_MULTI_JOB_FLOW *>	m_vMultiFlow;

    void 			SetNextScheduleMinute       (int a_nCount) ;
    void 			SetNextScheduleHour         (int a_nCount) ;
    void 			SetNextScheduleDay          (int a_nCount) ;
    void 			SetNextScheduleWeek         (int a_nCount) ;
    void 			SetNextScheduleMonth        (int a_nCount) ;
    void 			SetNextScheduleMonthLast    (int a_nCount) ;
	void 			RunSchedule					(char *a_strCurrent_date);
	void 			CheckNextDate(int a_nJobCount, int a_nGroupCount);
	void			FindNextJobInfo				(char * a_strJobName);

	int				ReceiveData					(ST_COWORK_INFO a_stcoworkinfo);
	int				SearchBatchJob				(ST_BatchRequest *a_stBatchrequest);
	int 			SearchNextJob				(ST_BatchResponse *a_stBatchresponse);
	int				FindGroupInfo				(char * a_strJobName, char * a_strGroupName);
	int		 		CheckMultiJob				(char * a_strGroupName, char * a_strJobName, int a_nExitCd);
	int 			ExcuteJobInfo				(int a_nGrpCnt, int a_wsm);

public:
    CScheduler() ;
    ~CScheduler() ;

    int             Initial() ;
    int             Run    () ;
    int             SetNextSchedule(ST_BatchGroup *a_stBatchGroupInfo, char *a_nextExecDate, int a_nCount) ;
	int 			SetBatchHist(ST_BatchJob *a_stBatchJobInfo, ST_PROCESS *a_stProcess, int a_count, int a_nWsmflag = 0);
	int 			CheckCondition(char * a_strJobName, int a_nExitCd);

    char 			*GetPkgName ()             { return m_batchGroupInfo_->pkg_name ; }
    char 			*GetGroupName ()           { return m_batchGroupInfo_->group_name ; }
    char 			*GetCreateDate ()          { return m_batchGroupInfo_->create_date ; }
    char 			*GetStartDate ()           { return m_batchGroupInfo_->start_date ; }
    char 			*GetSchedulecycletype ()   { return m_batchGroupInfo_->schedule_cycle_type ; }
    int   			Getschedule ()            { return m_batchGroupInfo_->nschedule_cycle ; }
    char 			*GetExpiredate ()          { return m_batchGroupInfo_->expire_date ; }
    char 			*GetDescription ()         { return m_batchGroupInfo_->description ; }

};
#endif

