/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdRegist.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDREGISTNODE_HPP_
#define CCMDREGISTNODE_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdRegistNode : public CCmdBase
{
public:
	// Request
	std::string m_strPkgName;
	std::string m_strNodeType;
	std::string m_strProcName;
	std::string	m_strVersion;
	std::string	m_strUuid;
	std::string	m_strIp;  
	int			m_nProcNo;

	// Response
	int			m_nNodeNo;
	std::string	m_strNodeName;

public:
	CCmdRegistNode() {
		m_nProcNo = -1;
		m_nNodeNo = -1;
		return;
	}
	~CCmdRegistNode() {}
	
	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);

private:

};

#endif // CCMDREGISTNODE_HPP_
