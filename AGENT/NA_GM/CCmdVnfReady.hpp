/* vim:ts=4:sw=4
 */
/**
 * \file	CCMDVNFREADY.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDVNFREADY_HPP_
#define CCMDVNFREADY_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdVnfReady : public CCmdBase
{
public:
	// Request
	std::string m_strPkgName;
	std::string m_strNodeType;
	std::string	m_strIp;
	std::string m_strPrcDate;
	std::string	m_strDstYN;
	
	// Response
	bool		m_bResult;
	std::string	m_strReason;

public:
	CCmdVnfReady() {
		m_bResult = true;
		return;
	}
	~CCmdVnfReady() {}
	
	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);
	
private:

};

#endif // CCMDVNFREADY_HPP_
