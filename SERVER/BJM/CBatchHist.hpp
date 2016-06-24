#ifndef __CBATCHHIST_HPP__
#define __CBATCHHIST_HPP__
#include "BJM_Define.hpp"
#include <list>
#include <map>


class CBatchHist
{
public:

    CBatchHist();
    ~CBatchHist();

    bool    			Init(DB * a_pDB);
	int 				LoadHistInfo(ST_BATCHHIST *a_batchHist);
	char*				GetPrcdate(ST_BatchJob *a_batchHob);
	int					HistUpdate(DB * a_pDB, ST_BATCHHIST *a_batchHist);
	int					HistDelete();
	int					HistInsert(ST_BATCHHIST *a_batchHist);

private:
    DB					* m_pDB;
	char				m_strPrc_date[20 + 1];
};

#endif // __CBATCHHIST_HPP__
