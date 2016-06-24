/* vim:ts=4:sw=4
 */
/**
 * \file	CCMDVNFSTART.hpp
 * \brief	
  *
 * $Author: junls@ntels.com $
 * $Date: $
 * $Id: $
 */
 
#ifndef CCMDVNFSTOP_HPP_
#define CCMDVNFSTOP_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdVnfStop : public CCmdBase
{
public:
	// Request
	std::string	m_strUuid;
	bool		m_bIsScaleIn;
	bool		m_bIsAll;

	// Response
	bool		m_bResult;
	std::string	m_strReason;

public:
	CCmdVnfStop() {
		m_bResult = true;
		return;
	}
	~CCmdVnfStop() {}

	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);
	
private:

};

#endif // CCMDVNFSTOP_HPP_
