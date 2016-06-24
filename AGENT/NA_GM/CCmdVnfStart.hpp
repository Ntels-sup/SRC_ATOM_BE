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
 
#ifndef CCMDVNFSTART_HPP_
#define CCMDVNFSTART_HPP_

#include <string>

#include "CCmdBase.hpp"


class CCmdVnfStart : public CCmdBase
{
public:
	// Request
	std::string	m_strUuid;
	bool		m_bIsAll;

	// Response
	bool		m_bResult;
	std::string	m_strReason;

public:
	CCmdVnfStart() {
		m_bResult = true;
		return;
	}
	~CCmdVnfStart() {}
	
	std::string	ResponseGen(void);
	bool		RequestParse(const char* a_szJson);
	
private:

};

#endif // CCMDVNFSTART_HPP_
