#ifndef __CPROCESS_HPP__
#define __CPROCESS_HPP__

#include "TRM_Define.hpp"
#include <list>
#include <map>

#define BJ_BATCH_JOB_LAST "999999"

class CProcess
{
public:

    CProcess();
    ~CProcess();

    bool 			Init(DB * a_pDB);
	int				LoadProcInfo(ST_PROCESS *a_process, ST_TRACE_ROUTE *a_traceroute);
	int 		    LoadCoworkNo(char* a_NodeNm, char* a_NmPrcNm, char* a_TrmPrcNm, char* a_WsmPrcNm, ST_COWORK_INFO *a_coworkinfo);

private:
	ST_PROCESS		processInfo_;
	DB * 			m_pDB;
};

#endif // __CPROCESS_HPP__
