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
	std::string	m_strUuid;
	std::string	m_strStatus;
	std::string	m_strPkgName;
	std::string	m_strNodeType;
	int		m_nNodeNo;
	std::string	m_strNodeName;
	std::string	m_strIp;
	std::string	m_strVersion;

public:
	CCmdNodeStatus() {
		m_nNodeNo = -1;
		return;
	}
	~CCmdNodeStatus() {}
	
	bool	NotifyParse(const char* a_szJson);
};

#endif // CCMDNODESTATUS_HPP_
