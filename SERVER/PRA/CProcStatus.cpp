/* vim:ts=4:sw=4
 */
/**
 * \file	CProcStatus.hpp
 * \brief	
 *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#include "CProcStatus.hpp"

using std::vector;
using std::map;


CProcStatus::CProcStatus()
{
	// 상태별 우선 순위를 정의 (최악 부터) 
	m_mapWorstOrder[UNKNOWN]	= 0;
	m_mapWorstOrder[ABNORMAL]	= 1;
	m_mapWorstOrder[HANGUP]		= 2;
	m_mapWorstOrder[STOPPED]	= 3;
	m_mapWorstOrder[SUSPEND]	= 4;
	m_mapWorstOrder[RUNNING]	= 5;

	return;	
}

const char* CProcStatus::StatusToString(EN_STATUS a_enStatus)
{
	switch (a_enStatus) {
		case RUNNING : 
			return "RUNNING";
		case STOPPED :
			return "STOPPED";
		case SUSPEND :
			return "SUSPEND";
		case HANGUP :
			return "HANGUP";
		case ABNORMAL :
			return "ABNORMAL";
		default :
			return "UNKNOWN";
	}

	return "UNKNOWN";
}

CProcStatus::EN_STATUS CProcStatus::FindWorstStatus(std::vector<EN_STATUS>& vecStatus)
{
	map<EN_STATUS, int>::iterator miter;
	int		nWorstLevel = -1;
	EN_STATUS enWorstStatus = STOPPED;
	
	for (auto viter = vecStatus.begin(); viter != vecStatus.end(); ++viter) {
		miter = m_mapWorstOrder.find(*viter);
		if  (miter == m_mapWorstOrder.end()) {
			continue;
		}
		if (nWorstLevel > miter->second) {
			// 가장 안좋은 상태를 유지, 숫자가 낮을 수록 worst status
			nWorstLevel = miter->second;
			enWorstStatus = *viter;
		}
	}

	return enWorstStatus;
}
