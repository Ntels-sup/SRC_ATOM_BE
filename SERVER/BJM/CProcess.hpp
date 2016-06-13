#ifndef __CPROCESS_HPP__
#define __CPROCESS_HPP__

#include "BJM_Define.hpp"
#include <list>
#include <map>

class CProcess
{
public:

    CProcess();
    ~CProcess();

    bool			Init(DB * a_pDB);
	int				LoadProcInfo(ST_BatchJob *a_batchjob, ST_PROCESS *a_process);
	int				ProcSelect(ST_BatchJob *a_batchjob, ST_PROCESS *a_process);
	int				ProcDelete();
	int				ProcUpdate();
	int				ProcInsert();
	int 			LoadCoworkNo(char* a_NodeNm, char* a_NmPrcNm, char* a_BjmPrcNm, char* a_WsmPrcNm, ST_COWORK_INFO *a_coworkinfo);

private:
	ST_PROCESS		processInfo_;
	DB * 			m_pDB;
};

#endif // __CPROCESS_HPP__
