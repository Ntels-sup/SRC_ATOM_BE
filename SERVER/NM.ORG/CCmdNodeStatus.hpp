/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdNodeStatus.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDNODESTATUS_HPP_
#define CCMDNODESTATUS_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdNodeStatus : public CCmdBase
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
	CCmdNodeStatus() {
		m_nNodeNo = -1;
		return;
	}
	~CCmdNodeStatus() {}
	
	std::string	NotifyGen(void);
};

#endif // CCMDNODESTATUS_HPP_
