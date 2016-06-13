/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdRegistNode.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDREGIST_HPP_
#define CCMDREGIST_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdRegistNode : public CCmdBase
{
public:
	// Request
	std::string m_strPkgName;
	std::string m_strNodeType;
	std::string m_strProcName;
	std::string m_strUuid;
	std::string	m_strNodeIp;
	int			m_nProcNo;
	std::string	m_strVersion;
	
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
	
	std::string	RequestGen(void);
	bool		ResponseParse(const char* a_szJson);

private:

};

#endif // CCMDREGIST_HPP_
