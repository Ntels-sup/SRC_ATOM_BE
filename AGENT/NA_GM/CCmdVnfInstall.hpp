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
 
#ifndef CCMDPKGINSTALL_HPP_
#define CCMDPKGINSTALL_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdVnfInstall : public CCmdBase
{
public:
	// Request
	std::string	m_strUuid;
	std::string	m_strVersion;
	std::string	m_strDownPath;
	std::string	m_strChecksum;
	std::string	m_strPrcDate;
	std::string	m_strDstYN;

	// Response
	bool		m_bResult;
	std::string	m_strReason;

public:
	CCmdVnfInstall() {
		m_bResult = true;
		return;
	}
	~CCmdVnfInstall() {}
	
	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);
	
private:

};

#endif // CCMDPKGINSTALL_HPP_
