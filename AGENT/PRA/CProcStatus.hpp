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

#ifndef CPROCSTATUS_HPP_
#define CPROCSTATUS_HPP_

#include <map>
#include <string>
#include <vector>


class CProcStatus
{
public:
	// 프로세스 현재 상태
	enum EN_STATUS {
		RUNNING = 1,
		STOPPED,
		SUSPEND,
		HANGUP,
		ABNORMAL,
		UNKNOWN,
		NOEXEC,				// 실행할 수 없는 상태, 실행 파일 없음 등..
		NONE
	};
	
public:
	CProcStatus();
	~CProcStatus() {}
	
	static const char*	StatusToString(EN_STATUS a_enStatus);
	EN_STATUS			FindWorstStatus(std::vector<EN_STATUS>& vecStatus);
	
private:
	std::map<EN_STATUS, int>	m_mapWorstOrder;

};

#endif // CPROCSTATUS_HPP_
