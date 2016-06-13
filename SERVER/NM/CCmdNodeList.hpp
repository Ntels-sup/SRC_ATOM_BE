/* vim:ts=4:sw=4
 */
/**
 * \file	CCmdNodeList.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDNODELIST_HPP_
#define CCMDNODELIST_HPP_

#include <list>
#include <string>

#include "CCmdBase.hpp"


class CCmdNodeList : public CCmdBase
{
public:
	// Request
	std::string m_strPkgName;
	std::string m_strNodeType;

	// Response
	struct ST_RESPONSE {
		std::string	m_strPkgName;
		std::string	m_strNodeType;
		int			m_nNodeNo;
		std::string	m_strNodeName;
		std::string	m_strIp;
		std::string	m_strVersion;
	};
	std::list<ST_RESPONSE> m_lstNode;

public:
	CCmdNodeList() {}
	~CCmdNodeList() {}
	
	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);

private:

};

#endif // CCMDNODELIST_HPP_
