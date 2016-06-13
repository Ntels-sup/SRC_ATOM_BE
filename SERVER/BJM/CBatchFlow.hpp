#ifndef __CBATCHFLOW_HPP__
#define __CBATCHFLOW_HPP__

//#include "AtomIF.hpp"
#include "BJM_Define.hpp"
#include <list>
#include <map>
#include <string>

class CBatchFlow
{
public:

    explicit CBatchFlow();
    ~CBatchFlow();

    bool			    Init(DB * a_pDB);
	int					LoadFlowInfo(ST_BatchFlow *_batchFlow);
	int					FlowSelect(ST_MULTI_JOB_FLOW *a_MultiFlow);
	int					GetMultiFlowCount(int a_nExit_cd);
	int					FlowCount();
	int					FlowUpdate();
	int					FlowDelete();
	int					FlowInsert();
//	char 				m_strNext_job_name     [BJM_FLOW_JOB_NAME_SIZE + 1];

	std::string 		GetJobName () { return m_stbatchFlowInfo.job_name; }
	int  				GetExitCd () { return m_stbatchFlowInfo.exit_cd ; }
	std::string 		GetNextJobName () { return m_stbatchFlowInfo.next_job_name; }

private:
	ST_BatchFlow		m_stbatchFlowInfo;
	DB * 				m_pDB;

};

#endif // __CBATCHFLOW_HPP__
