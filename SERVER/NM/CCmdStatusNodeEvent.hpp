/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdStatusNodeEvent.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDSTATUSNODEEVENT_HPP_
#define CCMDSTATUSNODEEVENT_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdStatusNodeEvent : public CCmdBase
{
public:
	// Notify
	std::string	m_strMessage;
	std::string	m_strPkgName;
	int			m_nNodeNo;
	std::string	m_strNodeName;
	std::string	m_strNodeVersion;
	std::string	m_strNodeType;
	std::string	m_strNodeStatus;

public:
	CCmdStatusNodeEvent() {
		m_nNodeNo = -1;
		return;
	}
	~CCmdStatusNodeEvent() {}
	
	std::string	NotifyGen(void);
};

#endif // CCMDSTATUSNODEEVENT_HPP_
