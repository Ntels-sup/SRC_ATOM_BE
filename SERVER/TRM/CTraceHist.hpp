#ifndef __CTRACEHIST_HPP__
#define __CTRACEHIST_HPP__
#include "TRM_Define.hpp"
#include <list>
#include <map>


class CTraceHist
{
public:

    CTraceHist();
    ~CTraceHist();

    bool    			Init(DB * a_pDB);
	int 				LoadHistInfo(ST_TRACE_HIST *a_tracehist);
	int					UpdateHist(DB * a_pDB, ST_TRACE_HIST *a_tracehist);
	int					InsertHist(ST_TRACE_HIST *a_tracehist);

private:
    DB					* m_pDB;
	ST_TRACE_HIST		m_stTraceHistInfo_;
};

#endif // __CBATCHHIST_HPP__
