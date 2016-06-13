#ifndef __CBATCHJOB_HPP__
#define __CBATCHJOB_HPP__
#include "BJM_Define.hpp"
#include <list>
#include <map>


class CBatchJob
{
public:

    CBatchJob();
    ~CBatchJob();

    bool    			Init		(DB * a_pDB);
	int 				LoadJobInfo (ST_BatchGroup *batchgroup_, ST_BatchJob *a_batchjob);
	int					JobUpdate();
	int					JobDelete();
	int					JobInsert();

private:
    DB 						* m_pDB;
//	ST_BatchJob			batchJobInfo_;
};

#endif // __CBATCHJOB_HPP__
